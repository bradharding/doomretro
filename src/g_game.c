/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2024 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2024 by Brad Harding <mailto:brad@doomretro.com>.

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
#include "i_gamecontroller.h"
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
gamestate_t     gamestate = GS_NONE;
skill_t         gameskill;
int             pendinggameskill;
int             gameepisode;
int             gamemap;
char            speciallumpname[6] = "";

bool            paused;
bool            sendpause;                          // send a pause event next tic
static bool     sendsave;                           // send a save event next tic

bool            viewactive;

int             gametime = 0;
int             totalkills;                         // for intermission
int             totalitems;
int             totalsecrets;
int             totalpickups;
int             monstercount[NUMMOBJTYPES];
int             barrelcount;
int             player1starts;

wbstartstruct_t wminfo;                             // parms for world map/intermission

#define MAXPLMOVE               forwardmove[1]
#define GAMECONTROLLERANGLETURN 5120

fixed_t         forwardmove[] = { FORWARDMOVE0, FORWARDMOVE1 };
fixed_t         sidemove[] = { SIDEMOVE0, SIDEMOVE1 };
fixed_t         angleturn[] = { 640, 1280, 320 };   // + slow turn

#define NUMWEAPONKEYS   7

static const int *keyboardweapons[NUMWEAPONKEYS] =
{
    &keyboardweapon1,
    &keyboardweapon2,
    &keyboardweapon3,
    &keyboardweapon4,
    &keyboardweapon5,
    &keyboardweapon6,
    &keyboardweapon7
};

static const int *mouseweapons[NUMWEAPONKEYS] =
{
    &mouseweapon1,
    &mouseweapon2,
    &mouseweapon3,
    &mouseweapon4,
    &mouseweapon5,
    &mouseweapon6,
    &mouseweapon7
};

static const int *gamecontrollerweapons[NUMWEAPONKEYS] =
{
    &gamecontrollerweapon1,
    &gamecontrollerweapon2,
    &gamecontrollerweapon3,
    &gamecontrollerweapon4,
    &gamecontrollerweapon5,
    &gamecontrollerweapon6,
    &gamecontrollerweapon7
};

#define SLOWTURNTICS    6

bool            gamekeydown[NUMKEYS] = { 0 };
char            keyactionlist[NUMKEYS][255] = { "" };
static int      turnheld;                       // for accelerative turning

static bool     mousearray[MAXMOUSEBUTTONS + 1];
bool            *mousebuttons = &mousearray[1]; // allow [-1]
char            mouseactionlist[MAXMOUSEBUTTONS + 2][255] = { "" };

bool            skipaction = false;

static int      mousex;
static int      mousey;

bool            usefreelook = false;

static int      dclicktime;
static bool     dclickstate;
static int      dclicks;
static int      dclicktime2;
static bool     dclickstate2;
static int      dclicks2;

static int      savegameslot;
static char     savedescription[SAVESTRINGSIZE];
char            savename[MAX_PATH];

gameaction_t    loadaction = ga_nothing;

void G_RemoveChoppers(void)
{
    viewplayer->cheats &= ~CF_CHOPPERS;
    viewplayer->powers[pw_invulnerability] = (viewplayer->invulnbeforechoppers ? 1 : STARTFLASHING);
    viewplayer->weaponowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
    oldweaponsowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
}

void G_NextWeapon(void)
{
    const weapontype_t  pendingweapon = viewplayer->pendingweapon;
    const weapontype_t  readyweapon = viewplayer->readyweapon;
    weapontype_t        i = (pendingweapon == wp_nochange ? readyweapon : pendingweapon);

    if (viewplayer->health <= 0)
        return;

    do
    {
        i = weaponinfo[i].nextweapon;

        if (i == wp_fist && viewplayer->weaponowned[wp_chainsaw] && !viewplayer->powers[pw_strength])
            i = wp_chainsaw;
    } while (!viewplayer->weaponowned[i] || (viewplayer->ammo[weaponinfo[i].ammotype] < weaponinfo[i].ammopershot && !infiniteammo));

    if (i != readyweapon)
    {
        viewplayer->pendingweapon = i;

        if (i == wp_fist)
        {
            if (viewplayer->powers[pw_strength])
                S_StartSound(NULL, sfx_getpow);

            viewplayer->fistorchainsaw = wp_fist;
        }
        else if (i == wp_chainsaw)
            viewplayer->fistorchainsaw = wp_chainsaw;
        else if (i == wp_shotgun || i == wp_supershotgun)
            viewplayer->preferredshotgun = i;
    }

    if ((viewplayer->cheats & CF_CHOPPERS) && i != wp_chainsaw)
        G_RemoveChoppers();
}

void G_PrevWeapon(void)
{
    const weapontype_t  pendingweapon = viewplayer->pendingweapon;
    const weapontype_t  readyweapon = viewplayer->readyweapon;
    weapontype_t        i = (pendingweapon == wp_nochange ? readyweapon : pendingweapon);

    if (viewplayer->health <= 0)
        return;

    do
    {
        i = weaponinfo[i].prevweapon;

        if (i == wp_fist && viewplayer->weaponowned[wp_chainsaw] && !viewplayer->powers[pw_strength])
            i = wp_bfg;
    } while (!viewplayer->weaponowned[i] || (viewplayer->ammo[weaponinfo[i].ammotype] < weaponinfo[i].ammopershot && !infiniteammo));

    if (i != readyweapon)
    {
        viewplayer->pendingweapon = i;

        if (i == wp_fist)
        {
            if (viewplayer->powers[pw_strength])
                S_StartSound(NULL, sfx_getpow);

            viewplayer->fistorchainsaw = wp_fist;
        }
        else if (i == wp_chainsaw)
            viewplayer->fistorchainsaw = wp_chainsaw;
        else if (i == wp_shotgun || i == wp_supershotgun)
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
    bool    strafe;
    int     run;
    int     forward = 0;
    int     side = 0;

    // [BH] This needs to be reset every tic, even if automap open and follow mode off
    memset(cmd, 0, sizeof(ticcmd_t));

    if (automapactive && !am_followmode && viewplayer->health > 0)
        return;

    strafe = (gamekeydown[keyboardstrafe] || mousebuttons[mousestrafe]
        || (gamecontrollerbuttons & gamecontrollerstrafe));
    run = ((gamekeydown[keyboardrun] | mousebuttons[mouserun]
        | (gamecontrollerbuttons & gamecontrollerrun)) ^ alwaysrun);
    usefreelook = (freelook || gamekeydown[keyboardfreelook] || mousebuttons[mousefreelook]
        || (gamecontrollerbuttons & gamecontrollerfreelook));

    // use two stage accelerative turning on the keyboard
    if (gamekeydown[keyboardright] || gamekeydown[keyboardleft]
        || (gamecontrollerbuttons & (gamecontrollerleft | gamecontrollerright)))
        turnheld++;
    else
        turnheld = 0;

    // let movement keys cancel each other out
    if (strafe)
    {
        if (gamekeydown[keyboardright] || mousebuttons[mouseright]
            || (gamecontrollerbuttons & gamecontrollerright))
            side += sidemove[run];

        if (gamekeydown[keyboardleft] || mousebuttons[mouseleft]
            || (gamecontrollerbuttons & gamecontrollerleft))
            side -= sidemove[run];
    }
    else
    {
        if (gamekeydown[keyboardright] || mousebuttons[mouseright]
            || (gamecontrollerbuttons & gamecontrollerright))
        {
            cmd->angleturn -= angleturn[(turnheld < SLOWTURNTICS ? 2 : run)];

            if (!menuactive)
                menuspindirection = SIGN(cmd->angleturn);
        }
        else if (gamecontrollerthumbRX > 0)
        {
            cmd->angleturn -= FixedMul(GAMECONTROLLERANGLETURN,
                (fixed_t)(gamecontrollerhorizontalsensitivity * gamecontrollerthumbRX));

            if (!menuactive)
                menuspindirection = SIGN(cmd->angleturn);
        }

        if (gamekeydown[keyboardleft] || mousebuttons[mouseleft]
            || (gamecontrollerbuttons & gamecontrollerleft))
        {
            cmd->angleturn += angleturn[(turnheld < SLOWTURNTICS ? 2 : run)];

            if (!menuactive)
                menuspindirection = SIGN(cmd->angleturn);
        }
        else if (gamecontrollerthumbRX < 0)
        {
            cmd->angleturn -= FixedMul(GAMECONTROLLERANGLETURN,
                (fixed_t)(gamecontrollerhorizontalsensitivity * gamecontrollerthumbRX));

            if (!menuactive)
                menuspindirection = SIGN(cmd->angleturn);
        }
    }

    if (gamecontrollerthumbRY)
    {
        if (usefreelook && joy_thumbsticks == 2)
        {
            if (!automapactive)
            {
                cmd->lookdir = (int)(96 * ((float)gamecontrollerthumbRY / SHRT_MAX)
                    * gamecontrollerverticalsensitivity);

                if (!joy_invertyaxis)
                    cmd->lookdir = -cmd->lookdir;
            }
        }
        else if (joy_thumbsticks == 1)
        {
            cmd->lookdir = 0;
            forward = (int)(forwardmove[run] * (float)gamecontrollerthumbRY / SHRT_MAX);
        }
    }

    if (gamekeydown[keyboardforward] || gamekeydown[keyboardforward2]
        || mousebuttons[mouseforward] || (gamecontrollerbuttons & gamecontrollerforward))
        forward += forwardmove[run];
    else if (gamecontrollerthumbLY < 0)
        forward -= (int)(forwardmove[run] * (float)gamecontrollerthumbLY / SHRT_MAX);

    if (gamekeydown[keyboardback] || gamekeydown[keyboardback2]
        || mousebuttons[mouseback] || (gamecontrollerbuttons & gamecontrollerback))
        forward -= forwardmove[run];
    else if (gamecontrollerthumbLY > 0)
        forward -= (int)(forwardmove[run] * (float)gamecontrollerthumbLY / SHRT_MAX);

    if (gamekeydown[keyboardstraferight] || gamekeydown[keyboardstraferight2]
        || mousebuttons[mousestraferight] || (gamecontrollerbuttons & gamecontrollerstraferight))
        side += sidemove[run];
    else if (gamecontrollerthumbLX > 0)
    {
        if (joy_thumbsticks == 2)
            side += (int)(sidemove[run] * (float)gamecontrollerthumbLX / SHRT_MAX);
        else
        {
            cmd->angleturn -= FixedMul(GAMECONTROLLERANGLETURN,
                (fixed_t)(gamecontrollerhorizontalsensitivity * gamecontrollerthumbLX));

            if (!menuactive)
                menuspindirection = SIGN(cmd->angleturn);
        }
    }

    if (gamekeydown[keyboardstrafeleft] || gamekeydown[keyboardstrafeleft2]
        || mousebuttons[mousestrafeleft] || (gamecontrollerbuttons & gamecontrollerstrafeleft))
        side -= sidemove[run];
    else if (gamecontrollerthumbLX < 0)
    {
        if (joy_thumbsticks == 2)
            side += (int)(sidemove[run] * (float)gamecontrollerthumbLX / SHRT_MAX);
        else
        {
            cmd->angleturn -= FixedMul(GAMECONTROLLERANGLETURN,
                (fixed_t)(gamecontrollerhorizontalsensitivity * gamecontrollerthumbLX));

            if (!menuactive)
                menuspindirection = SIGN(cmd->angleturn);
        }
    }

    if ((gamekeydown[keyboardjump] || mousebuttons[mousejump]
        || (gamecontrollerbuttons & gamecontrollerjump)) && !nojump)
        cmd->buttons |= BT_JUMP;

    // buttons
    if (skipaction)
        skipaction = false;
    else if (!freeze)
    {
        if ((mousebuttons[mousefire] || gamekeydown[keyboardfire]
            || (gamecontrollerbuttons & gamecontrollerfire)))
            cmd->buttons |= BT_ATTACK;

        if (gamekeydown[keyboarduse] || gamekeydown[keyboarduse2] || mousebuttons[mouseuse]
            || (gamecontrollerbuttons & (gamecontrolleruse | gamecontrolleruse2)))
        {
            cmd->buttons |= BT_USE;
            dclicks = 0;
        }
    }

    if (!idclev && !idmus)
        for (int i = 0; i < NUMWEAPONKEYS; i++)
        {
            const int   key = *keyboardweapons[i];

            if (gamekeydown[key] && !keydown)
            {
                keydown = key;
                cmd->buttons |= (BT_CHANGE | (i << BT_WEAPONSHIFT));
                break;
            }
            else if (mousebuttons[*mouseweapons[i]])
            {
                if (viewplayer->readyweapon != i
                    || (i == wp_fist && viewplayer->weaponowned[wp_chainsaw])
                    || (i == wp_shotgun && viewplayer->weaponowned[wp_supershotgun]))
                {
                    cmd->buttons |= (BT_CHANGE | (i << BT_WEAPONSHIFT));
                    break;
                }
            }
            else if (gamecontrollerbuttons & *gamecontrollerweapons[i])
            {
                if (viewplayer->readyweapon != i
                    || (i == wp_fist && viewplayer->weaponowned[wp_chainsaw])
                    || (i == wp_shotgun && viewplayer->weaponowned[wp_supershotgun]))
                {
                    cmd->buttons |= (BT_CHANGE | (i << BT_WEAPONSHIFT));
                    break;
                }
            }
        }

    if (m_doubleclick_use)
    {
        bool    bstrafe;

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
            side += mousex / 2;
        else
        {
            cmd->angleturn -= mousex * 0x08;

            if (!menuactive)
                menuspindirection = SIGN(cmd->angleturn);
        }

        mousex = 0;
    }

    if (mousey)
    {
        if (usefreelook && !automapactive)
            cmd->lookdir = (m_invertyaxis ? -mousey : mousey);
        else if (!m_novertical)
            forward += mousey / 2;

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
        cmd->buttons = (BT_SPECIAL | BTS_PAUSE);
    }

    if (sendsave)
    {
        sendsave = false;
        cmd->buttons = (BT_SPECIAL | BTS_SAVEGAME);
    }
}

static void G_SetInitialWeapon(void)
{
    viewplayer->weaponowned[wp_fist] = true;
    viewplayer->weaponowned[wp_pistol] = true;
    viewplayer->ammo[am_clip] = initial_bullets;
    viewplayer->readyweapon = (!initial_bullets && weaponinfo[wp_pistol].ammotype != am_noammo ?
        wp_fist : wp_pistol);
    viewplayer->pendingweapon = viewplayer->readyweapon;

    for (ammotype_t i = 0; i < NUMAMMO; i++)
        viewplayer->maxammo[i] = maxammo[i];
}

//
// G_ResetPlayer
// [BH] Reset player's health, armor, weapons and ammo
//
static void G_ResetPlayer(void)
{
    viewplayer->health = 100;
    viewplayer->armor = 0;
    viewplayer->armortype = armortype_none;
    viewplayer->preferredshotgun = wp_shotgun;
    viewplayer->fistorchainsaw = wp_fist;
    viewplayer->backpack = false;
    memset(viewplayer->weaponowned, false, sizeof(viewplayer->weaponowned));
    memset(viewplayer->ammo, 0, sizeof(viewplayer->ammo));

    viewplayer->weaponowned[wp_fist] = true;
    viewplayer->weaponowned[wp_pistol] = true;
    viewplayer->ammo[am_clip] = 50;
    viewplayer->readyweapon = wp_pistol;
    viewplayer->pendingweapon = wp_pistol;

    for (ammotype_t i = 0; i < NUMAMMO; i++)
        viewplayer->maxammo[i] = maxammo[i];
}

//
// G_DoLoadLevel
//
void G_DoLoadLevel(void)
{
    int     ep;
    bool    resetplayer;

    drawdisk = true;
    drawdisktics = DRAWDISKTICS;

    if (timer)
        P_SetTimer(timer);

    if (wipegamestate == GS_LEVEL)
        wipegamestate = GS_NONE;    // force a wipe

    gamestate = GS_LEVEL;

    if (viewplayer->playerstate == PST_DEAD)
        viewplayer->playerstate = PST_REBORN;

    viewplayer->damageinflicted = 0;
    viewplayer->damagereceived = 0;
    viewplayer->cheated = 0;
    memset(viewplayer->shotssuccessful, 0, sizeof(viewplayer->shotssuccessful));
    memset(viewplayer->shotsfired, 0, sizeof(viewplayer->shotsfired));
    viewplayer->distancetraveled = 0;
    viewplayer->gamessaved = 0;
    viewplayer->gamesloaded = 0;
    viewplayer->itemspickedup_ammo_bullets = 0;
    viewplayer->itemspickedup_ammo_cells = 0;
    viewplayer->itemspickedup_ammo_rockets = 0;
    viewplayer->itemspickedup_ammo_shells = 0;
    viewplayer->itemspickedup_armor = 0;
    viewplayer->itemspickedup_health = 0;
    viewplayer->itemspickedup_keys = 0;
    viewplayer->itemspickedup_powerups = 0;
    memset(viewplayer->monsterskilled, 0, sizeof(viewplayer->monsterskilled));
    viewplayer->prevmessage[0] = '\0';
    viewplayer->prevmessagetics = 0;
    viewplayer->infightcount = 0;
    viewplayer->respawncount = 0;
    viewplayer->resurrectioncount = 0;
    viewplayer->telefragcount = 0;
    viewplayer->automapopened = 0;
    viewplayer->monstersgibbed = 0;

    prevmessage[0] = '\0';
    freeze = false;

    ep = (gamemode == commercial ? (gamemission == pack_nerve ? 2 : 1) : gameepisode);

    // [BH] Reset player's health, armor, weapons and ammo on pistol start
    if ((resetplayer = (pistolstart || P_GetMapPistolStart(ep, gamemap))))
        G_ResetPlayer();

    if (viewplayer->cheats & CF_CHOPPERS)
    {
        viewplayer->cheats &= ~CF_CHOPPERS;
        viewplayer->powers[pw_invulnerability] = 0;

        if (!(viewplayer->weaponowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers))
            viewplayer->readyweapon = wp_fist;

        oldweaponsowned[wp_chainsaw] = viewplayer->chainsawbeforechoppers;
    }

    if (pendinggameskill)
    {
        gameskill = pendinggameskill - 1;

        if (gameskill == sk_nightmare)
            viewplayer->cheats &= ~(CF_NOCLIP | CF_GODMODE | CF_BUDDHA);

        pendinggameskill = 0;
    }

    P_RemoveBloodSplats();

    // initialize the msecnode_t freelist. phares 03/25/98
    // any nodes in the freelist are gone by now, cleared
    // by Z_FreeTags() when the previous level ended or player
    // died.
    P_FreeSecNodeList();

    P_MapName(ep, gamemap);

    P_SetupLevel(ep, gamemap);

    // [BH] Reset player's health, armor, weapons and ammo on pistol start
    if (resetplayer && gamemap != 1)
    {
        if (M_StringCompare(playername, playername_default))
            C_Warning(0, "You now have 100%% health, no armor, and only a pistol with 50 bullets.");
        else
            C_Warning(0, "%s now have 100%% health, no armor, and only a pistol with 50 bullets.",
                playername);
    }

    skycolumnoffset = 0;

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

    I_UpdateBlitFunc(false);

    M_SetWindowCaption();

    if (automapactive || mapwindow)
        AM_Start(automapactive);

    ammohighlight = 0;
    armorhighlight = 0;
    healthhighlight = 0;

    ammodiff[am_clip] = 0;
    ammodiff[am_shell] = 0;
    ammodiff[am_misl] = 0;
    ammodiff[am_cell] = 0;
    armordiff = 0;
    healthdiff = 0;

    if (r_screensize == r_screensize_max && animatedstats)
        P_AnimateAllStatsFromStart();
}

void G_ToggleAlwaysRun(evtype_t type)
{
    char        temp[255];
    const int   oldcaretpos = caretpos;
    const int   oldselectstart = selectstart;
    const int   oldselectend = selectend;

#if defined(_WIN32)
    alwaysrun = (keyboardalwaysrun == KEY_CAPSLOCK && type == ev_keydown ?
        (GetKeyState(VK_CAPITAL) & 0x0001) : !alwaysrun);
#else
    alwaysrun = !alwaysrun;
#endif

    M_StringCopy(temp, consoleinput, sizeof(temp));
    C_StringCVAROutput(stringize(alwaysrun), (alwaysrun ? "on" : "off"));
    M_StringCopy(consoleinput, temp, sizeof(consoleinput));

    caretpos = oldcaretpos;
    selectstart = oldselectstart;
    selectend = oldselectend;

    if (!consoleactive)
    {
        if (alwaysrun)
        {
            HU_SetPlayerMessage(s_ALWAYSRUNON, false, false);
            C_Output(s_ALWAYSRUNON);
        }
        else
        {
            HU_SetPlayerMessage(s_ALWAYSRUNOFF, false, false);
            C_Output(s_ALWAYSRUNOFF);
        }

        message_dontfuckwithme = true;
    }

    M_SaveCVARs();
}

//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//
bool G_Responder(const event_t *ev)
{
    int key;

    // any other key pops up menu if on title screen
    if (gameaction == ga_nothing && gamestate == GS_TITLESCREEN)
    {
        if (!menuactive
            && !consoleactive
            && !fadecount
            && ((ev->type == ev_keydown
                && !keydown
                && (ev->data1 < KEY_F1 || ev->data1 > KEY_F12)
                && ev->data1 != KEY_BACKSPACE
                && ev->data1 != KEY_ALT
                && !((ev->data1 == KEY_ENTER || ev->data1 == KEY_TAB) && altdown)
                && ev->data1 != keyboardscreenshot)
            || (ev->type == ev_mouse && mousewait < I_GetTime() && ev->data1 && !(ev->data1 & MOUSE_RIGHTBUTTON))
            || (ev->type == ev_controller
                && gamecontrollerwait < I_GetTime()
                && gamecontrollerbuttons)))
        {
            if (ev->type == ev_keydown && ev->data1 == keyboardalwaysrun)
            {
                keydown = keyboardalwaysrun;
                G_ToggleAlwaysRun(ev_keydown);
            }
            else if (ev->type == ev_mouse && ev->data1 == mousealwaysrun)
                G_ToggleAlwaysRun(ev_mouse);
            else
            {
                keydown = ev->data1;
                gamecontrollerbuttons = 0;
                mousewait = I_GetTime() + 5;
                gamecontrollerwait = mousewait + 3;

                if (splashscreen)
                {
                    logotic = MIN(logotic, 93);
                    pagetic = MIN(pagetic, 10);
                }
                else
                {
                    pagetic = PAGETICS;
                    M_OpenMainMenu();
                    S_StartSound(NULL, sfx_swtchn);
                }
            }

            return true;
        }
        else if (!menuactive && !consoleactive && !splashscreen && ev->type == ev_keyup && ev->data1 == keyboardscreenshot)
        {
            S_StartSound(NULL, sfx_scrsht);
            memset(screens[0], nearestwhite, SCREENAREA);
            D_FadeScreen(true);

            return true;
        }

        return false;
    }

    if (gamestate == GS_LEVEL)
    {
        if (ST_Responder(ev))
            return true;    // status window ate it

        if (AM_Responder(ev))
            return true;    // automap ate it
    }

    if (gamestate == GS_FINALE && F_Responder(ev))
        return true;        // finale ate the event

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

                D_FadeScreen(false);
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

            return true;    // eat events

        case ev_keyup:
            if (ev->data1 < NUMKEYS)
                gamekeydown[ev->data1] = false;

            return false;   // always let key up events filter down

        case ev_mouse:
        {
            const int   mousebutton = ev->data1;

            for (int i = 0, j = 1; i < MAXMOUSEBUTTONS; i++, j <<= 1)
                mousebuttons[i] = !!(mousebutton & j);

            if (mousebuttons[mousealwaysrun])
                G_ToggleAlwaysRun(ev_mouse);

            if (mouseactionlist[mousebutton][0] && !freeze)
                C_ExecuteInputString(mouseactionlist[mousebutton]);

            if (!automapactive && !menuactive && !paused && !freeze)
            {
                if (mousenextweapon < MAXMOUSEBUTTONS && mousebuttons[mousenextweapon])
                    G_NextWeapon();
                else if (mouseprevweapon < MAXMOUSEBUTTONS && mousebuttons[mouseprevweapon])
                    G_PrevWeapon();
            }

            if (!automapactive || am_followmode)
            {
                mousex = (int)(ev->data2 * m_sensitivity / 10.0f);
                mousey = (int)(-ev->data3 * m_sensitivity / 10.0f);
            }

            return true;    // eat events
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

            return true;    // eat events

        case ev_controller:
            if (!automapactive && !menuactive && !paused)
            {
                static uint64_t wait;
                uint64_t        time = I_GetTime();

                if ((gamecontrollerbuttons & gamecontrollernextweapon) && wait < time && !freeze)
                {
                    wait = time + 7;

                    if (!gamecontrollerpress || gamecontrollerwait < time)
                    {
                        G_NextWeapon();
                        gamecontrollerpress = false;
                    }
                }
                else if ((gamecontrollerbuttons & gamecontrollerprevweapon) && wait < time && !freeze)
                {
                    wait = time + 7;

                    if (!gamecontrollerpress || gamecontrollerwait < time)
                    {
                        G_PrevWeapon();
                        gamecontrollerpress = false;
                    }
                }
                else if ((gamecontrollerbuttons & gamecontrolleralwaysrun) && wait < time)
                {
                    wait = time + 7;

                    if (!gamecontrollerpress || gamecontrollerwait < time)
                    {
                        G_ToggleAlwaysRun(ev_controller);
                        gamecontrollerpress = false;
                    }
                }
            }

            return true;    // eat events

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
                M_StringCopy(savename, P_SaveGameFile(quicksaveslot), sizeof(savename));
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
                    S_StopSounds();
                    S_StartSound(NULL, sfx_swtchn);
                    viewplayer->fixedcolormap = 0;
                    I_SetPalette(PLAYPAL);
                    I_UpdateBlitFunc(false);
                    I_StopGameControllerRumble();

                    if (windowfocused)
                        S_LowerMusicVolume();
                }
                else
                {
                    S_ResumeMusic();
                    S_StartSound(NULL, sfx_swtchx);
                    I_SetPalette(&PLAYPAL[st_palette * 768]);

                    if (windowfocused)
                        S_RestoreMusicVolume();

                    if (reopenautomap)
                    {
                        reopenautomap = false;
                        AM_Start(true);
                        viewactive = false;
                    }
                }

                break;

            case BTS_SAVEGAME:
                gameaction = ga_savegame;
                break;
        }

        viewplayer->cmd.buttons = 0;
    }

    // Have we just finished displaying an intermission screen?
    if (oldgamestate == GS_INTERMISSION && gamestate != GS_INTERMISSION)
        WI_End();
    else if (oldgamestate == GS_LEVEL && gamestate == GS_INTERMISSION)
        I_Sleep(500);

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
// Called when the player completes a level.
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

    totaltime += maptime;
}

//
// G_PlayerReborn
// Called after the player dies
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
    shake = 0;
}

//
// G_DoReborn
//
static void G_DoReborn(void)
{
    if (solonet)
        P_ResurrectPlayer(initial_health);
    else if (quicksaveslot >= 0 && autoload)
        gameaction = ga_autoloadgame;
    else
    {
        gameaction = ga_loadlevel;
        C_Input("restartmap");

        if (M_StringCompare(mapnum, "E1M4B") || M_StringCompare(mapnum, "E1M8B"))
            M_StringCopy(speciallumpname, mapnum, sizeof(speciallumpname));
    }
}

void G_ScreenShot(void)
{
    if (V_ScreenShot())
    {
        static char buffer[512];

        M_snprintf(buffer, sizeof(buffer), s_GSCREENSHOT, lbmname1);
        HU_SetPlayerMessage(buffer, false, false);
        message_dontfuckwithme = true;

        C_Output(BOLD("%s") " was saved.", lbmpath1);

        if (*lbmpath2)
            C_Output(BOLD("%s") " was also saved.", lbmpath2);
    }
    else
    {
        C_ShowConsole(false);
        C_Warning(0, "A screenshot couldn't be taken.");
    }
}

bool    newpars = false;

// DOOM par times
int pars[10][10] =
{
    { 0 },
    { 0,  30,  75, 120,  90, 165, 180, 180, 165, 165 },
    { 0,  90,  90,  90, 120,  90, 360, 240, 135, 170 },
    { 0,  90,  45,  90, 150,  90,  90, 165, 105, 135 },

    // [BH] Episode 4, 5 and 6 par times
    { 0, 165, 255, 135, 150, 180, 390, 135, 360, 180 },
    { 0,  90, 150, 360, 420, 780, 420, 780, 300, 660 },
    { 0, 480, 300, 360, 240, 510, 840, 960, 390, 450 }
};

// DOOM II par times
int cpars[100] =
{
     30,  90, 120, 120,  90, 150, 120, 120, 270,  90,   // 01-10
    210, 150, 150, 150, 210, 150, 420, 150, 210, 150,   // 11-20
    240, 150, 180, 150, 150, 300, 330, 420, 300, 180,   // 21-30
    120,  30,   0                                       // 31-32
};

// [BH] No Rest For The Living par times
static const int npars[9] =
{
     75, 105, 120, 105, 210, 105, 165, 105, 135
};

//
// G_DoCompleted
//
bool    secretexit;

void G_ExitLevel(void)
{
    secretexit = false;
    gameaction = ga_completed;
}

void G_SecretExitLevel(void)
{
    secretexit = true;
    gameaction = ga_completed;
}

int G_GetParTime(void)
{
    const int   par = P_GetMapPar(gameepisode, gamemap);

    if (par)
        return par;
    else if (!newpars && !canmodify && (!nerve || gamemap > 9))
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
    else if (gameepisode <= 6 && gamemap <= 9)
        return pars[gameepisode][gamemap];
    else
        return 0;
}

static void G_DoCompleted(void)
{
    const int   nextmap = P_GetMapNext(gameepisode, gamemap);
    const int   secretnextmap = P_GetMapSecretNext(gameepisode, gamemap);

    P_LookForFriends();

    gameaction = ga_nothing;

    I_UpdateBlitFunc(false);

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
                if ((gamemode == registered && gameepisode < 3)
                    || (gamemode == retail && gameepisode < (sigil ? (sigil2 ? 6 : 5) : 4)))
                {
                    episode++;
                    EpiDef.laston++;
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
            wminfo.next = secretnextmap - 1;
        else if (nextmap > 0)
            wminfo.next = nextmap - 1;
        else if (secretexit)
            wminfo.next = 8;            // go to secret level
        else if (gamemap == 9)
        {
            // returning from secret level
            switch (gameepisode)
            {
                case 1:
                case 6:
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
    wminfo.maxsecret = totalsecrets;
    wminfo.partime = G_GetParTime() * TICRATE;
    wminfo.skills = (totalkills ? viewplayer->killcount : 1);
    wminfo.sitems = (totalitems ? viewplayer->itemcount : 1);
    wminfo.ssecret = viewplayer->secretcount;
    wminfo.stime = maptime;

    gamestate = GS_INTERMISSION;
    viewactive = false;
    automapactive = false;

    stat_mapsfinished = SafeAdd(stat_mapsfinished, 1);
    M_SaveCVARs();

    if (!numconsolestrings || (!M_StringCompare(console[numconsolestrings - 1].string, "exitmap")))
        C_Input("exitmap");

    WI_Start(&wminfo);
}

//
// G_WorldDone
//
void G_WorldDone(void)
{
    const char  *intertext = P_GetInterText(gameepisode, gamemap);
    const char  *intersecrettext = P_GetInterSecretText(gameepisode, gamemap);

    gameaction = ga_worlddone;

    if (secretexit)
        viewplayer->didsecret = true;

    if (*intertext
        || (*intersecrettext && secretexit)
        || P_GetMapEndCast(gameepisode, gamemap)
        || P_GetMapEndGame(gameepisode, gamemap))
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

    if (quicksaveslot >= 0 && autosave)
        gameaction = ga_autosavegame;
}

void G_LoadGame(const char *name)
{
    M_StringCopy(savename, name, sizeof(savename));
    gameaction = ga_loadgame;
}

void G_DoLoadGame(void)
{
    int savedmaptime;

    I_SetPalette(PLAYPAL);

    loadaction = gameaction;
    gameaction = ga_nothing;

    if (numconsolestrings == 1 || !M_StringStartsWith(console[numconsolestrings - 1].string, "load "))
        C_Input("load %s", savename);

    if (!(save_stream = fopen(savename, "rb")))
    {
        menuactive = false;
        C_ShowConsole(false);
        C_Warning(0, BOLD("%s") " couldn't be loaded.", savename);
        loadaction = ga_nothing;
        return;
    }

    if (!P_ReadSaveGameHeader(savedescription))
    {
        fclose(save_stream);
        loadaction = ga_nothing;
        return;
    }

    savedmaptime = maptime;

    // load a base level
    G_InitNew(gameskill, gameepisode, gamemap);

    maptime = savedmaptime;

    // unarchive all the modifications
    P_UnarchivePlayer();
    P_UnarchiveWorld();
    P_UnarchiveThinkers();
    P_UnarchiveSpecials();
    P_UnarchiveMap();

    P_RestoreTargets();

    P_MapEnd();

    if (musinfo.currentitem != -1)
        S_ChangeMusInfoMusic(musinfo.currentitem, true);

    if (!P_ReadSaveGameEOF())
        I_Error("%s is invalid.", savename);

    P_ReadSaveGameFooter();

    fclose(save_stream);

    if (setsizeneeded)
        R_ExecuteSetViewSize();

    // draw the pattern into the back screen
    if (viewwidth != SCREENWIDTH)
        R_FillBackScreen();

    st_facecount = 0;

    viewplayer->gamesloaded++;
    stat_gamesloaded = SafeAdd(stat_gamesloaded, 1);
    M_SaveCVARs();

    if (consoleactive)
    {
        C_Output(BOLD("%s") " loaded.", savename);
        C_HideConsoleFast();
    }

    ammohighlight = 0;
    armorhighlight = 0;
    healthhighlight = 0;

    ammodiff[am_clip] = 0;
    ammodiff[am_shell] = 0;
    ammodiff[am_misl] = 0;
    ammodiff[am_cell] = 0;
    armordiff = 0;
    healthdiff = 0;

    if (r_screensize == r_screensize_max && animatedstats)
        P_AnimateAllStatsFromStart();
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
void G_SaveGame(const int slot, const char *description, const char *name)
{
    M_StringCopy(savename, name, sizeof(savename));
    savegameslot = slot;
    M_StringCopy(savedescription, description, sizeof(savedescription));
    sendsave = true;
    drawdisk = true;
    drawdisktics = DRAWDISKTICS;
}

static void G_DoSaveGame(void)
{
    char    *temp_savegame_file = P_TempSaveGameFile();
    char    *savegame_file = (consoleactive || !*savedescription ? savename : P_SaveGameFile(savegameslot));

    // Open the savegame file for writing. We write to a temporary file
    // and then rename it at the end if it was successfully written.
    // This prevents an existing savegame from being overwritten by
    // a corrupted one, or if a savegame buffer overrun occurs.
    if (!(save_stream = fopen(temp_savegame_file, "wb")))
    {
        menuactive = false;
        C_ShowConsole(false);
        C_Warning(0, BOLD("%s") " couldn't be saved.", savegame_file);
    }
    else
    {
        char    *backup_savegame_file = M_StringJoin(savegame_file, ".bak", NULL);

        if (gameaction == ga_autosavegame)
        {
            M_UpdateSaveGameName(quicksaveslot);
            M_StringCopy(savedescription, savegamestrings[quicksaveslot], sizeof(savedescription));
        }

        P_WriteSaveGameHeader(savedescription);

        P_ArchivePlayer();
        P_ArchiveWorld();
        P_ArchiveThinkers();
        P_ArchiveSpecials();
        P_ArchiveMap();

        P_WriteSaveGameEOF();

        P_WriteSaveGameFooter();

        // Finish up, close the savegame file.
        fclose(save_stream);

        // Now rename the temporary savegame file to the actual savegame
        // file, backing up the old savegame if there was one there.
        remove(backup_savegame_file);
        rename(savegame_file, backup_savegame_file);
        rename(temp_savegame_file, savegame_file);

        free(backup_savegame_file);

        if (savegameslot >= 0)
            savegames = true;

        if (!numconsolestrings || !M_StringStartsWith(console[numconsolestrings - 1].string, "save "))
            C_Input("save %s", savegame_file);

        if (!*savedescription)
            M_StringCopy(savedescription, maptitle, sizeof(savedescription));

        if (consoleactive)
            C_Output(BOLD("%s") " was saved.", savename);
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
        if (viewwidth != SCREENWIDTH)
            R_FillBackScreen();
    }

    gameaction = ga_nothing;
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
    infight = false;
    totaltime = 0;

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

    st_facecount = ST_STRAIGHTFACECOUNT;
    G_InitNew(d_skill, d_episode, d_map);
    gameaction = ga_nothing;
    infight = false;
}

// killough 04/10/98: New function to fix bug which caused DOOM
// lockups when idclev was used in conjunction with -fast.
void G_SetFastParms(bool fast_pending)
{
    static bool fast;                       // remembers fast state

    if (fast != fast_pending)               // only change if necessary
    {
        for (int i = 0; i < nummobjtypes; i++)
            if (mobjinfo[i].altspeed != NO_ALTSPEED)
                SWAP(mobjinfo[i].speed, mobjinfo[i].altspeed);

        if ((fast = fast_pending))
        {
            for (int i = 0; i < numstates; i++)
                if ((states[i].flags & STATEF_SKILL5FAST) && states[i].tics != 1)
                    states[i].tics >>= 1;   // don't change 1->0 since it causes cycles
        }
        else
        {
            for (int i = 0; i < numstates; i++)
                if (states[i].flags & STATEF_SKILL5FAST)
                    states[i].tics <<= 1;
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

    if (!customepisode)
    {
        if (gamemode == retail)
        {
            if (sigil2)
            {
                if (ep > 6)
                    ep = 6;
            }
            else if (sigil)
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
    }

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

    if (numconsolestrings == 1
        || (!M_StringCompare(console[numconsolestrings - 2].string, "newgame")
            && !M_StringStartsWith(console[numconsolestrings - 2].string, "map ")
            && !M_StringStartsWith(console[numconsolestrings - 1].string, "load ")
            && !autostart))
        C_Input("newgame");

    mobjinfo[MT_SPIDER].spawnhealth = (sigil2 && ep == 6 ? 9000 : 3000);

    G_DoLoadLevel();
}
