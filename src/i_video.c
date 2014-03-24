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

#include <math.h>

#include "d_main.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_tinttab.h"
#include "i_video.h"
#include "m_config.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include "s_sound.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// Window position:

char *windowposition = "";

SDL_Surface *screen;
SDL_Surface *screenbuffer = NULL;

// palette

SDL_Color palette[256];
static boolean   palette_to_set;

// Bit mask of mouse button state.

static unsigned int mouse_button_state = 0;

// Fullscreen width and height

int screenwidth = 0;
int screenheight = 0;

// Window width and height

int windowwidth = SCREENWIDTH;
int windowheight = SCREENWIDTH * 3 / 4;

// Run in full screen mode?

boolean fullscreen = true;

boolean widescreen = false;
boolean returntowidescreen = false;
boolean widescreenhud = true;

// Flag indicating whether the screen is currently visible:
// when the screen isn't visible, don't render the screen
boolean screenvisible;

boolean window_focused;

// Empty mouse cursor

static SDL_Cursor *emptycursor;

// Window resize state.

boolean      need_resize = false;
unsigned int resize_h;

int desktopwidth;
int desktopheight;

char *videodriver = "windib";
char envstring[255];

static int width;
static int height;
static int stepx;
static int stepy;
static int startx;
static int starty;
static int pitch;

byte *pixels;

boolean keys[UCHAR_MAX];

byte gammatable[GAMMALEVELS][256];

double gammalevel[GAMMALEVELS] =
{
    0.5, 0.625, 0.75, 0.875, 1.0, 1.25, 1.5, 1.75, 2.0
};

// Gamma correction level to use
int usegamma = USEGAMMA_DEFAULT;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = MOUSEACCELERATION_DEFAULT;
int   mouse_threshold = MOUSETHRESHOLD_DEFAULT;

static void ApplyWindowResize(int height);
static void SetWindowPositionVars(void);

boolean MouseShouldBeGrabbed()
{
    // if the window doesn't have focus, never grab it

    if (!window_focused)
        return false;

    // always grab the mouse when full screen (dont want to
    // see the mouse pointer)

    if (fullscreen)
        return true;

    // when menu is active or game is paused, release the mouse

    if (menuactive || paused)
        return false;

    // only grab mouse when playing levels (but not demos)

    return (gamestate == GS_LEVEL && !demoplayback && !advancedemo);

}

// Update the value of window_focused when we get a focus event
//
// We try to make ourselves be well-behaved: the grab on the mouse
// is removed if we lose focus (such as a popup window appearing),
// and we dont move the mouse around if we aren't focused either.

static void UpdateFocus(void)
{
    Uint8          state = SDL_GetAppState();
    static boolean alreadypaused = false;

    // Should the screen be grabbed?

    screenvisible = (state & SDL_APPACTIVE);

    // We should have input (keyboard) focus and be visible
    // (not minimized)

    window_focused = ((state & SDL_APPINPUTFOCUS) && screenvisible);

    if (!window_focused && !menuactive && gamestate == GS_LEVEL && !demoplayback && !advancedemo)
    {
        if (paused)
            alreadypaused = true;
        else
        {
            alreadypaused = false;
            sendpause = true;
        }
    }
}

// Show or hide the mouse cursor. We have to use different techniques
// depending on the OS.

static void SetShowCursor(boolean show)
{
    // On Windows, using SDL_ShowCursor() adds lag to the mouse input,
    // so work around this by setting an invisible cursor instead. On
    // other systems, it isn't possible to change the cursor, so this
    // hack has to be Windows-only. (Thanks to entryway for this)

    SDL_SetCursor(emptycursor);

    // When the cursor is hidden, grab the input.

    SDL_WM_GrabInput((SDL_GrabMode)!show);
}

int translatekey[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, KEY_BACKSPACE, KEY_TAB, 0, 0, 0, KEY_ENTER, 0, 0,
    0, 0, 0, KEY_PAUSE, 0, 0, 0, 0, 0, 0, 0, KEY_ESCAPE, 0, 0, 0, 0, ' ', '!',
    '\"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', '-', KEY_MINUS, '.',
    ',', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<',
    KEY_EQUALS, '>', '?', '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', 0, 0, 0, 0, KEY_DEL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEYP_0, KEYP_1, KEYP_2,
    KEYP_3, KEYP_4, KEYP_5, KEYP_6, KEYP_7, KEYP_8, KEYP_9, KEYP_PERIOD,
    KEYP_DIVIDE, KEYP_MULTIPLY, KEYP_MINUS, KEYP_PLUS, KEYP_ENTER, KEYP_EQUALS,
    KEY_UPARROW, KEY_DOWNARROW, KEY_RIGHTARROW, KEY_LEFTARROW, KEY_INS,
    KEY_HOME, KEY_END, KEY_PGUP, KEY_PGDN, KEY_F1, KEY_F2, KEY_F3, KEY_F4,
    KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, 0, 0, 0,
    0, 0, 0, KEY_NUMLOCK, KEY_CAPSLOCK, KEY_SCRLCK, KEY_RSHIFT, KEY_RSHIFT,
    KEY_RCTRL, KEY_RCTRL, KEY_RALT, KEY_RALT, KEY_RALT, KEY_RALT, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

int TranslateKey2(int key)
{
    switch (key)
    {
        case KEY_LEFTARROW:    return SDLK_LEFT;
        case KEY_RIGHTARROW:   return SDLK_RIGHT;
        case KEY_DOWNARROW:    return SDLK_DOWN;
        case KEY_UPARROW:      return SDLK_UP;
        case KEY_ESCAPE:       return SDLK_ESCAPE;
        case KEY_ENTER:        return SDLK_RETURN;
        case KEY_TAB:          return SDLK_TAB;
        case KEY_F1:           return SDLK_F1;
        case KEY_F2:           return SDLK_F2;
        case KEY_F3:           return SDLK_F3;
        case KEY_F4:           return SDLK_F4;
        case KEY_F5:           return SDLK_F5;
        case KEY_F6:           return SDLK_F6;
        case KEY_F7:           return SDLK_F7;
        case KEY_F8:           return SDLK_F8;
        case KEY_F9:           return SDLK_F9;
        case KEY_F10:          return SDLK_F10;
        case KEY_F11:          return SDLK_F11;
        case KEY_F12:          return SDLK_F12;
        case KEY_BACKSPACE:    return SDLK_BACKSPACE;
        case KEY_DEL:          return SDLK_DELETE;
        case KEY_PAUSE:        return SDLK_PAUSE;
        case KEY_EQUALS:       return SDLK_EQUALS;
        case KEY_MINUS:        return SDLK_MINUS;
        case KEY_RSHIFT:       return SDLK_RSHIFT;
        case KEY_RCTRL:        return SDLK_RCTRL;
        case KEY_RALT:         return SDLK_RALT;
        case KEY_CAPSLOCK:     return SDLK_CAPSLOCK;
        case KEY_SCRLCK:       return SDLK_SCROLLOCK;
        case KEYP_0:           return SDLK_KP0;
        case KEYP_1:           return SDLK_KP1;
        case KEYP_3:           return SDLK_KP3;
        case KEYP_5:           return SDLK_KP5;
        case KEYP_7:           return SDLK_KP7;
        case KEYP_9:           return SDLK_KP9;
        case KEYP_PERIOD:      return SDLK_KP_PERIOD;
        case KEYP_MULTIPLY:    return SDLK_KP_MULTIPLY;
        case KEYP_DIVIDE:      return SDLK_KP_DIVIDE;
        case KEY_INS:          return SDLK_INSERT;
        case KEY_NUMLOCK:      return SDLK_NUMLOCK;
        default:               return key;
    }
}

boolean keystate(int key)
{
    Uint8 *keystate = SDL_GetKeyState(NULL);

    return keystate[TranslateKey2(key)];
}

void I_SaveWindowPosition(void)
{
    if (!fullscreen)
    {
        static SDL_SysWMinfo pInfo;
        RECT r;

        SDL_VERSION(&pInfo.version);
        SDL_GetWMInfo(&pInfo);

        GetWindowRect(pInfo.window, &r);
        sprintf(windowposition, "%i,%i", r.left + 8, r.top + 30);
    }
}

void I_ShutdownGraphics(void)
{
    SetShowCursor(true);

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static void UpdateMouseButtonState(unsigned int button, boolean on)
{
    event_t     ev;
    int         buttons[MAX_MOUSE_BUTTONS + 1] = { 0, 1, 4, 2, 8, 16, 32, 64, 128 };

    // Turn bit representing this button on or off.
    if (on)
        mouse_button_state |= buttons[MAX(SDL_BUTTON_LEFT, MIN(button, MAX_MOUSE_BUTTONS))];
    else
        mouse_button_state &= ~buttons[MAX(SDL_BUTTON_LEFT, MIN(button, MAX_MOUSE_BUTTONS))];

    // Post an event with the new button state.
    ev.type = ev_mouse;
    ev.data1 = mouse_button_state;
    ev.data2 = ev.data3 = 0;
    D_PostEvent(&ev);
}

static int AccelerateMouse(int val)
{
    if (val < 0)
        return -AccelerateMouse(-val);

    if (val > mouse_threshold)
        return (int)((val - mouse_threshold) * mouse_acceleration + mouse_threshold);
    else
        return val;
}

boolean altdown = false;
boolean waspaused = false;

void I_GetEvent(void)
{
    SDL_Event   sdlevent;
    event_t     ev;
    SDLKey      key;

    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:
                ev.type = ev_keydown;
                key = sdlevent.key.keysym.sym;
                ev.data1 = translatekey[key];
                ev.data2 = key;

                altdown = (sdlevent.key.keysym.mod & KMOD_ALT);

                if (altdown && ev.data1 == KEY_TAB)
                    ev.data1 = ev.data2 = ev.data3 = 0;

                if (!isdigit(ev.data2))
                    idclev = idmus = false;

                if (idbehold && keys[MAX(0, MIN(ev.data2, 255))])
                {
                    HU_clearMessages();
                    idbehold = false;
                }

                if (ev.data1)
                    D_PostEvent(&ev);
                break;

            case SDL_KEYUP:
                ev.type = ev_keyup;
                ev.data1 = translatekey[sdlevent.key.keysym.sym];
                ev.data2 = 0;

                altdown = (sdlevent.key.keysym.mod & KMOD_ALT);
                keydown = 0;

                if (ev.data1)
                    D_PostEvent(&ev);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (window_focused)
                {
                    idclev = false;
                    idmus = false;
                    if (idbehold)
                    {
                        HU_clearMessages();
                        idbehold = false;
                    }
                    UpdateMouseButtonState(sdlevent.button.button, true);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (window_focused)
                {
                    keydown = 0;
                    UpdateMouseButtonState(sdlevent.button.button, false);
                }
                break;

            case SDL_JOYBUTTONUP:
                keydown = 0;
                break;

            case SDL_QUIT:
                if (!quitting)
                {
                    keydown = 0;
                    if (paused)
                    {
                        paused = false;
                        waspaused = true;
                    }
                    S_StartSound(NULL, sfx_swtchn);
                    M_QuitDOOM();
                }
                break;

            case SDL_ACTIVEEVENT:
                // need to update our focus state
                UpdateFocus();
                break;

            case SDL_VIDEOEXPOSE:
                palette_to_set = true;
                break;

            case SDL_SYSWMEVENT:
                if (sdlevent.syswm.msg->msg == WM_MOVE)
                {
                    I_SaveWindowPosition();
                    SetWindowPositionVars();
                    M_SaveDefaults();
                }
                break;

            case SDL_VIDEORESIZE:
                if (!fullscreen)
                {
                    need_resize = true;
                    resize_h = sdlevent.resize.h;
                    break;
                }

            default:
                break;
        }
    }
}

// Warp the mouse back to the middle of the screen
static void CenterMouse(void)
{
    // Warp the the screen center
    SDL_WarpMouse(screen->w / 2, screen->h / 2);

    // Clear any relative movement caused by warping
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(NULL, NULL);
}

//
// Read the change in mouse state to generate mouse motion events
//
// This is to combine all mouse movement for a tic into one mouse
// motion event.
static void I_ReadMouse(void)
{
    int         x, y;
    event_t     ev;

    SDL_GetRelativeMouseState(&x, &y);

    if (x)
    {
        ev.type = ev_mouse;
        ev.data1 = mouse_button_state;
        ev.data2 = AccelerateMouse(x);
        ev.data3 = 0;

        D_PostEvent(&ev);
    }

    if (MouseShouldBeGrabbed())
        CenterMouse();
}

//
// I_StartTic
//
void I_StartTic(void)
{
    I_GetEvent();
    I_ReadMouse();
    gamepadfunc();
}

boolean currently_grabbed = false;

static void UpdateGrab(void)
{
    boolean     grab = MouseShouldBeGrabbed();

    if (grab && !currently_grabbed)
        SetShowCursor(false);
    else if (!grab && currently_grabbed)
    {
        SetShowCursor(true);
        SDL_WarpMouse(screen->w - 16, screen->h - 16);
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;
}

static __forceinline void blit(void)
{
    fixed_t     i = 0;
    fixed_t     y = starty;

    do
    {
        byte    *dest = pixels + i;
        byte    *src = *screens + (y >> FRACBITS) * SCREENWIDTH;
        fixed_t x = startx;

        do
            *dest++ = *(src + (x >> FRACBITS));
        while ((x += stepx) < (SCREENWIDTH << FRACBITS));
        i += pitch;
    }
    while ((y += stepy) < (SCREENHEIGHT << FRACBITS));
}

SDL_Rect dest_rect;

//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{
    if (need_resize)
    {
        ApplyWindowResize(resize_h);
        need_resize = false;
        palette_to_set = true;
    }

    UpdateGrab();

    // Don't update the screen if the window isn't visible.
    // Not doing this breaks under Windows when we alt-tab away
    // while fullscreen.
    if (!screenvisible)
        return;

    if (palette_to_set)
    {
        SDL_SetColors(screenbuffer, palette, 0, 256);
        palette_to_set = false;
    }

    // draw to screen
    blit();

    SDL_FillRect(screen, NULL, 0);

    SDL_BlitSurface(screenbuffer, NULL, screen, &dest_rect);

    SDL_Flip(screen);
}

//
// I_ReadScreen
//
void I_ReadScreen(byte *scr)
{
    memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void I_SetPalette(byte *doompalette)
{
    int i;

    for (i = 0; i < 256; ++i)
    {
        palette[i].r = gammatable[usegamma][*doompalette++];
        palette[i].g = gammatable[usegamma][*doompalette++];
        palette[i].b = gammatable[usegamma][*doompalette++];
    }

    palette_to_set = true;
}

static void CreateCursors(void)
{
    static Uint8 emptycursordata = 0;

    emptycursor = SDL_CreateCursor(&emptycursordata, &emptycursordata, 1, 1, 0, 0);
}

static void SetWindowPositionVars(void)
{
    char        buf[64];
    int         x, y;

    if (sscanf(windowposition, "%i,%i", &x, &y) == 2)
    {
        if (x < 0)
            x = 0;
        else if (x > desktopwidth)
            x = desktopwidth - 16;
        if (y < 0)
            y = 0;
        else if (y > desktopheight)
            y = desktopheight - 16;
        sprintf(buf, "SDL_VIDEO_WINDOW_POS=%i,%i", x, y);
        putenv(buf);
    }
    else
        putenv("SDL_VIDEO_CENTERED=1");
}

static void SetVideoMode(void)
{
    SDL_VideoInfo *videoinfo = (SDL_VideoInfo *)SDL_GetVideoInfo();

    desktopwidth = videoinfo->current_w;
    desktopheight = videoinfo->current_h;

    if (fullscreen)
    {
        width = screenwidth;
        height = screenheight;
        if (!width || !height)
        {
            width = desktopwidth;
            height = desktopheight;
        }

        screen = SDL_SetVideoMode(width, height, 0,
                                  SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF | SDL_FULLSCREEN);

        height = screen->h;
        width = height * 4 / 3;
        width += (width & 1);

        if (width > screen->w)
        {
            width = screen->w;
            height = width * 3 / 4;
            height += (height & 1);
        }
    }
    else
    {
        height = MAX(ORIGINALWIDTH * 3 / 4, windowheight);
        width = height * 4 / 3;
        width += (width & 1);

        if (width > windowwidth)
        {
            width = windowwidth;
            height = width * 3 / 4;
            height += (height & 1);
        }

        SetWindowPositionVars();
        screen = SDL_SetVideoMode(windowwidth, windowheight, 0,
                                  SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF | SDL_RESIZABLE);

        widescreen = false;
    }

    screenbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
    pitch = screenbuffer->pitch;
    pixels = (byte *)screenbuffer->pixels;

    stepx = (SCREENWIDTH << FRACBITS) / width;
    stepy = (SCREENHEIGHT << FRACBITS) / height;

    startx = stepx - 1;
    starty = stepy - 1;

    dest_rect.x = (screen->w - screenbuffer->w) / 2;
    dest_rect.y = (screen->h - screenbuffer->h) / 2;
}

void ToggleWideScreen(boolean toggle)
{
    if ((double)screen->w / screen->h < 1.6)
    {
        widescreen = returntowidescreen = false;
        return;
    }

    if (toggle)
    {
        if (!dest_rect.x && !dest_rect.y)
            return;

        widescreen = true;

        if (returntowidescreen && screenblocks == 11)
        {
            screenblocks = 10;
            R_SetViewSize(screenblocks);
        }

        width = screen->w;
        height = screen->h + (int)((double)screen->h * 32 / (ORIGINALHEIGHT - 32) + 1.5);
    }
    else
    {
        widescreen = false;

        height = screen->h;
        width = height * 4 / 3;
        width += (width & 1);
    }
    returntowidescreen = false;

    screenbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
    pitch = screenbuffer->pitch;
    pixels = (byte *)screenbuffer->pixels;

    stepx = (SCREENWIDTH << FRACBITS) / width;
    stepy = (SCREENHEIGHT << FRACBITS) / height;

    startx = stepx - 1;
    starty = stepy - 1;

    if (widescreen)
        dest_rect.x = dest_rect.y = 0;
    else
    {
        dest_rect.x = (screen->w - screenbuffer->w) / 2;
        dest_rect.y = (screen->h - screenbuffer->h) / 2;
    }

    palette_to_set = true;
}

void init_win32(LPCTSTR lpIconName);

void ToggleFullScreen(void)
{
    fullscreen = !fullscreen;
    if (fullscreen)
    {
        width = screenwidth;
        height = screenheight;
        if (!width || !height)
        {
            width = desktopwidth;
            height = desktopheight;
        }

        screen = SDL_SetVideoMode(width, height, 0,
                                  SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF | SDL_FULLSCREEN);

        if (screenblocks == 11)
        {
            if (gamestate != GS_LEVEL)
                returntowidescreen = true;
            else
            {
                dest_rect.x = 1;
                ToggleWideScreen(true);
                if (widescreen && screenblocks == 11)
                    screenblocks = 10;
                R_SetViewSize(screenblocks);
                M_SaveDefaults();
                return;
            }
        }

        height = screen->h;
        width = height * 4 / 3;
        width += (width & 1);

        if (width > screen->w)
        {
            width = screen->w;
            height = width * 3 / 4;
            height += (height & 1);
        }
    }
    else
    {
        event_t ev;

        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        putenv(envstring);
        SDL_InitSubSystem(SDL_INIT_VIDEO);

        init_win32(gamemission == doom ? "DOOM" : "DOOM2");

        height = MAX(ORIGINALWIDTH * 3 / 4, windowheight);
        width = height * 4 / 3;
        width += (width & 1);

        if (width > windowwidth)
        {
            width = windowwidth;
            height = width * 3 / 4;
            height += (height & 1);
        }

        if (widescreen)
        {
            widescreen = false;
            screenblocks = 11;
            R_SetViewSize(screenblocks);
        }

        SetWindowPositionVars();
        screen = SDL_SetVideoMode(width, height, 0,
                                  SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF | SDL_RESIZABLE);

        CreateCursors();
        SDL_SetCursor(emptycursor);

        SDL_WM_SetCaption(gamestate == GS_LEVEL ? mapnumandtitle : gamedescription, NULL);

        currently_grabbed = true;
        UpdateFocus();
        UpdateGrab();

        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

        ev.type = ev_keyup;
        ev.data1 = KEY_RALT;
        ev.data2 = ev.data3 = 0;
        D_PostEvent(&ev);
    }

    screenbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
    pitch = screenbuffer->pitch;
    pixels = (byte *)screenbuffer->pixels;

    stepx = (SCREENWIDTH << FRACBITS) / width;
    stepy = (SCREENHEIGHT << FRACBITS) / height;

    startx = stepx - 1;
    starty = stepy - 1;

    dest_rect.x = (screen->w - screenbuffer->w) / 2;
    dest_rect.y = (screen->h - screenbuffer->h) / 2;

    M_SaveDefaults();
}

void ApplyWindowResize(int height)
{
    windowheight = MAX(SCREENWIDTH * 3 / 4, height);
    windowwidth = windowheight * 4 / 3;
    windowwidth += (windowwidth & 1);

    screen = SDL_SetVideoMode(windowwidth, windowheight, 0,
                              SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF | SDL_RESIZABLE);

    screenbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, windowwidth, windowheight, 8, 0, 0, 0, 0);
    pitch = screenbuffer->pitch;
    pixels = (byte *)screenbuffer->pixels;

    stepx = (SCREENWIDTH << FRACBITS) / windowwidth;
    stepy = (SCREENHEIGHT << FRACBITS) / windowheight;

    startx = stepx - 1;
    starty = stepy - 1;

    dest_rect.x = (screen->w - screenbuffer->w) / 2;
    dest_rect.y = (screen->h - screenbuffer->h) / 2;

    M_SaveDefaults();
}

void I_InitGammaTables(void)
{
    int i;
    int j;

    for (i = 0; i < GAMMALEVELS; i++)
        if (gammalevel[i] == 1.0)
            for (j = 0; j < 256; j++)
                gammatable[i][j] = j;
        else
            for (j = 0; j < 256; j++)
                gammatable[i][j] = (byte)(pow((j + 1) / 256.0, 1.0 / gammalevel[i]) * 255.0);
}

void I_InitGraphics(void)
{
    int       i = 0;
    SDL_Event dummy;
    byte      *doompal = (byte *)W_CacheLumpName("PLAYPAL", PU_CACHE);

    while (i < UCHAR_MAX)
        keys[i++] = true;
    keys['v'] = keys['V'] = false;
    keys['s'] = keys['S'] = false;
    keys['i'] = keys['I'] = false;
    keys['r'] = keys['R'] = false;
    keys['a'] = keys['A'] = false;
    keys['l'] = keys['L'] = false;

    I_InitTintTables(doompal);

    I_InitGammaTables();

    sprintf(envstring, "SDL_VIDEODRIVER=%s", videodriver);
    putenv(envstring);

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
        I_Error("Failed to initialize video: %s", SDL_GetError());

    CreateCursors();
    SDL_SetCursor(emptycursor);

    init_win32(gamemission == doom ? "DOOM" : "DOOM2");

    SetVideoMode();

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    SDL_WM_SetCaption(gamedescription, NULL);

    SDL_FillRect(screenbuffer, NULL, 0);

    I_SetPalette(doompal);
    SDL_SetColors(screenbuffer, palette, 0, 256);

    UpdateFocus();
    UpdateGrab();

    screens[0] = (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);

    memset(screens[0], 0, SCREENWIDTH * SCREENHEIGHT);

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    while (SDL_PollEvent(&dummy));

    if (fullscreen)
        CenterMouse();
}
