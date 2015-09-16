/*
========================================================================

                               DOOM RETRO
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright (C) 2013-2015 Brad Harding.

  DOOM RETRO is a fork of CHOCOLATE DOOM by Simon Howard.
  For a complete list of credits, see the accompanying AUTHORS file.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM RETRO is in no way affiliated with nor endorsed by
  id Software LLC.

========================================================================
*/

#include "c_console.h"
#include "d_deh.h"
#include "d_main.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_tinttab.h"
#include "i_video.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "SDL.h"
#include "s_sound.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"
#include "z_zone.h"

#if defined(WIN32)
#include "SDL_syswm.h"
#endif

char                    *vid_windowposition = vid_windowposition_default;
dboolean                manuallypositioning = false;

SDL_Window              *window = NULL;
SDL_Renderer            *renderer;
static SDL_Texture      *texture = NULL;
static SDL_Texture      *texture_upscaled = NULL;
static SDL_Surface      *surface = NULL;
static SDL_Surface      *buffer = NULL;
static SDL_Palette      *palette;
static SDL_Color        colors[256];

dboolean                bestscale = false;

int                     vid_display = vid_display_default;
static int              displayindex;
static int              numdisplays;
static SDL_Rect         *displays;
char                    *vid_scaledriver = vid_scaledriver_default;
char                    *vid_scalefilter = vid_scalefilter_default;
dboolean                vid_vsync = vid_vsync_default;

// Bit mask of mouse button state
static unsigned int     mouse_button_state = 0;

dboolean                m_novertical = m_novertical_default;

static int              buttons[MAX_MOUSE_BUTTONS + 1] = { 0, 1, 4, 2, 8, 16, 32, 64, 128 };

// Fullscreen width and height
int                     screenwidth;
int                     screenheight;
char                    *vid_screenresolution = vid_screenresolution_default;

// Window width and height
int                     windowwidth;
int                     windowheight;
char                    *vid_windowsize = vid_windowsize_default;

int                     windowx = 0;
int                     windowy = 0;

int                     displaywidth;
int                     displayheight;
int                     displaycenterx;
int                     displaycentery;

// Run in full screen mode?
dboolean                vid_fullscreen = vid_fullscreen_default;

dboolean                vid_widescreen = vid_widescreen_default;
dboolean                returntowidescreen = false;

dboolean                r_hud = r_hud_default;

dboolean                vid_capfps = vid_capfps_default;

dboolean                window_focused;

// Empty mouse cursor
static SDL_Cursor       *cursors[2];

#if !defined(WIN32)
char                    *vid_driver = vid_driver_default;
char                    envstring[255];
#endif

dboolean                keys[UCHAR_MAX];

byte                    gammatable[GAMMALEVELS][256];

float                   gammalevels[GAMMALEVELS] =
{
    // Darker
    0.50f, 0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f,

    // No gamma correction
    1.0f,

    // Lighter
    1.05f, 1.10f, 1.15f, 1.20f, 1.25f, 1.30f, 1.35f, 1.40f, 1.45f, 1.50f,
    1.55f, 1.60f, 1.65f, 1.70f, 1.75f, 1.80f, 1.85f, 1.90f, 1.95f, 2.0f
};

// Gamma correction level to use
int                     gammaindex;
float                   r_gamma = r_gamma_default;

SDL_Rect                src_rect = { 0, 0, 0, 0 };

void                    (*updatefunc)(void);

dboolean                vid_showfps = false;
int                     fps = 0;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of m_threshold, they are multiplied
// by m_acceleration to increase the speed.
float                   m_acceleration = m_acceleration_default;
int                     m_threshold = m_threshold_default;

int                     capslock;
dboolean                pm_alwaysrun = pm_alwaysrun_default;

extern int              key_alwaysrun;

extern int              windowborderwidth;
extern int              windowborderheight;

void ST_doRefresh(void);

dboolean MouseShouldBeGrabbed(void)
{
    // if the window doesn't have focus, never grab it
    if (!window_focused)
        return false;

    // always grab the mouse when full screen (don't want to
    // see the mouse pointer)
    if (vid_fullscreen)
        return true;

    // when menu is active or game is paused, release the mouse
    if (menuactive || consoleactive || paused)
        return false;

    // only grab mouse when playing levels
    return (gamestate == GS_LEVEL);
}

// Update the value of window_focused when we get a focus event
//
// We try to make ourselves be well-behaved: the grab on the mouse
// is removed if we lose focus (such as a popup window appearing),
// and we don't move the mouse around if we aren't focused either.
static void UpdateFocus(void)
{
    Uint32      state = SDL_GetWindowFlags(window);

    // We should have input (keyboard) focus and be visible (not minimized)
    window_focused = ((state & SDL_WINDOW_INPUT_FOCUS) && (state & SDL_WINDOW_SHOWN));

    if (!window_focused && !menuactive && gamestate == GS_LEVEL && !paused && !consoleactive)
    {
        sendpause = true;
        blurred = false;
    }
}

// Show or hide the mouse cursor. We have to use different techniques
// depending on the OS.
static void SetShowCursor(dboolean show)
{
    // On Windows, using SDL_ShowCursor() adds lag to the mouse input,
    // so work around this by setting an invisible cursor instead. On
    // other systems, it isn't possible to change the cursor, so this
    // hack has to be Windows-only. (Thanks to entryway for this)
#if defined(WIN32)
    SDL_SetCursor(cursors[show]);
#else
    SDL_ShowCursor(show);
#endif

    // When the cursor is hidden, grab the input.
    SDL_SetRelativeMouseMode(!show);
}

int translatekey[] =
{
    0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, ' ', KEY_MINUS, KEY_EQUALS, '[', ']', '\\',
    '\\', ';', '\'', '`', ',', '.', '/', KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
    KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, 0, KEY_SCRLCK, KEY_PAUSE, KEY_INS,
    KEY_HOME, KEY_PGUP, KEY_DEL, KEY_END, KEY_PGDN, KEY_RIGHTARROW, KEY_LEFTARROW, KEY_DOWNARROW,
    KEY_UPARROW, KEY_NUMLOCK, KEYP_DIVIDE, KEYP_MULTIPLY, KEYP_MINUS, KEYP_PLUS, KEYP_ENTER,
    KEYP_1, KEYP_2, KEYP_3, KEYP_4, KEYP_5, KEYP_6, KEYP_7, KEYP_8, KEYP_9, KEYP_0, KEYP_PERIOD, 0,
    0, 0, KEYP_EQUALS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    KEY_RCTRL, KEY_RSHIFT, KEY_RALT, 0, KEY_RCTRL, KEY_RSHIFT, KEY_RALT, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int TranslateKey2(int key)
{
    switch (key)
    {
        case KEY_LEFTARROW:    return SDL_SCANCODE_LEFT;
        case KEY_RIGHTARROW:   return SDL_SCANCODE_RIGHT;
        case KEY_DOWNARROW:    return SDL_SCANCODE_DOWN;
        case KEY_UPARROW:      return SDL_SCANCODE_UP;
        case KEY_ESCAPE:       return SDL_SCANCODE_ESCAPE;
        case KEY_ENTER:        return SDL_SCANCODE_RETURN;
        case KEY_TAB:          return SDL_SCANCODE_TAB;
        case KEY_F1:           return SDL_SCANCODE_F1;
        case KEY_F2:           return SDL_SCANCODE_F2;
        case KEY_F3:           return SDL_SCANCODE_F3;
        case KEY_F4:           return SDL_SCANCODE_F4;
        case KEY_F5:           return SDL_SCANCODE_F5;
        case KEY_F6:           return SDL_SCANCODE_F6;
        case KEY_F7:           return SDL_SCANCODE_F7;
        case KEY_F8:           return SDL_SCANCODE_F8;
        case KEY_F9:           return SDL_SCANCODE_F9;
        case KEY_F10:          return SDL_SCANCODE_F10;
        case KEY_F11:          return SDL_SCANCODE_F11;
        case KEY_F12:          return SDL_SCANCODE_F12;
        case KEY_BACKSPACE:    return SDL_SCANCODE_BACKSPACE;
        case KEY_DEL:          return SDL_SCANCODE_DELETE;
        case KEY_PAUSE:        return SDL_SCANCODE_PAUSE;
        case KEY_EQUALS:       return SDL_SCANCODE_EQUALS;
        case KEY_MINUS:        return SDL_SCANCODE_MINUS;
        case KEY_RSHIFT:       return SDL_SCANCODE_RSHIFT;
        case KEY_RCTRL:        return SDL_SCANCODE_RCTRL;
        case KEY_RALT:         return SDL_SCANCODE_RALT;
        case KEY_CAPSLOCK:     return SDL_SCANCODE_CAPSLOCK;
        case KEY_SCRLCK:       return SDL_SCANCODE_SCROLLLOCK;
        case KEYP_0:           return SDL_SCANCODE_KP_0;
        case KEYP_1:           return SDL_SCANCODE_KP_1;
        case KEYP_3:           return SDL_SCANCODE_KP_3;
        case KEYP_5:           return SDL_SCANCODE_KP_5;
        case KEYP_7:           return SDL_SCANCODE_KP_7;
        case KEYP_9:           return SDL_SCANCODE_KP_9;
        case KEYP_PERIOD:      return SDL_SCANCODE_KP_PERIOD;
        case KEYP_MULTIPLY:    return SDL_SCANCODE_KP_MULTIPLY;
        case KEYP_DIVIDE:      return SDL_SCANCODE_KP_DIVIDE;
        case KEY_INS:          return SDL_SCANCODE_INSERT;
        case KEY_NUMLOCK:      return SDL_SCANCODE_NUMLOCKCLEAR;
        default:               return key;
    }
}

dboolean keystate(int key)
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    return keystate[TranslateKey2(key)];
}

static void FreeSurfaces(void)
{
    SDL_FreePalette(palette);
    SDL_FreeSurface(surface);
    SDL_FreeSurface(buffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(texture_upscaled);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void I_ShutdownGraphics(void)
{
    SetShowCursor(true);
    FreeSurfaces();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void I_ShutdownKeyboard(void)
{
#if defined(WIN32)
    if (key_alwaysrun == KEY_CAPSLOCK)
        if ((GetKeyState(VK_CAPITAL) & 0x0001) && !capslock)
        {
            keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY, (uintptr_t)0);
            keybd_event(VK_CAPITAL, 0x45, (KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP), (uintptr_t)0);
        }
#endif
}

static int AccelerateMouse(int val)
{
    if (val < 0)
        return -AccelerateMouse(-val);

    if (val > m_threshold)
        return (int)((val - m_threshold) * m_acceleration + m_threshold);
    else
        return val;
}

// Warp the mouse back to the middle of the screen
static void CenterMouse(void)
{
    // Warp to the screen center
    SDL_WarpMouseInWindow(window, displaycenterx, displaycentery);

    // Clear any relative movement caused by warping
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(NULL, NULL);
}

dboolean altdown = false;
dboolean waspaused = false;

dboolean noinput = true;

static void I_GetEvent(void)
{
    event_t     event;
    SDL_Event   SDLEvent;
    SDL_Event   *Event = &SDLEvent;

    while (SDL_PollEvent(Event))
    {
        switch (Event->type)
        {
            case SDL_KEYDOWN:
                if (noinput)
                    return;

                event.type = ev_keydown;

                event.data1 = translatekey[Event->key.keysym.scancode];
                event.data2 = Event->key.keysym.sym;
                if (event.data2 < SDLK_SPACE || event.data2 > SDLK_z)
                    event.data2 = 0;

                altdown = (Event->key.keysym.mod & KMOD_ALT);

                if (event.data1)
                {
                    if (altdown && event.data1 == KEY_TAB)
                        event.data1 = event.data2 = 0;

                    if (!isdigit(event.data2))
                        idclev = idmus = false;

                    if (idbehold && keys[event.data2])
                    {
                        message_clearable = true;
                        idbehold = false;
                        HU_ClearMessages();
                    }

                    D_PostEvent(&event);
                }
                break;

            case SDL_KEYUP:
                event.type = ev_keyup;

                event.data1 = translatekey[Event->key.keysym.scancode];

                altdown = (Event->key.keysym.mod & KMOD_ALT);
                keydown = 0;

                if (event.data1)
                    D_PostEvent(&event);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (m_sensitivity || menuactive)
                {
                    idclev = false;
                    idmus = false;
                    if (idbehold)
                    {
                        message_clearable = true;
                        HU_ClearMessages();
                        idbehold = false;
                    }
                    event.type = ev_mouse;
                    mouse_button_state |= buttons[Event->button.button];
                    event.data1 = mouse_button_state;
                    event.data2 = 0;
                    event.data3 = 0;
                    D_PostEvent(&event);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (m_sensitivity || menuactive)
                {
                    keydown = 0;
                    event.type = ev_mouse;
                    mouse_button_state &= ~buttons[Event->button.button];
                    event.data1 = mouse_button_state;
                    event.data2 = 0;
                    event.data3 = 0;
                    D_PostEvent(&event);
                }
                break;

            case SDL_MOUSEWHEEL:
                if (m_sensitivity || menuactive || consoleactive)
                {
                    keydown = 0;
                    event.type = ev_mousewheel;
                    event.data1 = Event->wheel.y;
                    event.data2 = 0;
                    event.data3 = 0;
                    D_PostEvent(&event);
                }
                break;

            case SDL_JOYBUTTONUP:
                keydown = 0;
                break;

            case SDL_QUIT:
                if (!quitting && !splashscreen)
                {
                    keydown = 0;
                    C_HideConsoleFast();
                    if (paused)
                    {
                        paused = false;
                        waspaused = true;
                    }
                    S_StartSound(NULL, sfx_swtchn);
                    M_QuitDOOM(0);
                }
                break;

            case SDL_WINDOWEVENT:
                switch (Event->window.event)
                {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        // need to update our focus state
                        UpdateFocus();
                        break;

                    case SDL_WINDOWEVENT_EXPOSED:
                        SDL_SetPaletteColors(palette, colors, 0, 256);
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        if (!vid_fullscreen)
                        {
                            char        buffer[16] = "";

                            windowwidth = Event->window.data1;
                            windowheight = Event->window.data2;
                            M_snprintf(buffer, sizeof(buffer), "%ix%i", windowwidth, windowheight);
                            vid_windowsize = strdup(buffer);
                            M_SaveCVARs();

                            displaywidth = windowwidth;
                            displayheight = windowheight;
                            displaycenterx = displaywidth / 2;
                            displaycentery = displayheight / 2;
                        }
                        break;

                    case SDL_WINDOWEVENT_MOVED:
                        if (!vid_fullscreen && !manuallypositioning)
                        {
                            char        buffer[16] = "";

                            windowx = Event->window.data1;
                            windowy = Event->window.data2;
                            M_snprintf(buffer, sizeof(buffer), "(%i,%i)", windowx, windowy);
                            vid_windowposition = strdup(buffer);

                            vid_display = SDL_GetWindowDisplayIndex(window) + 1;
                            M_SaveCVARs();
                        }
                        manuallypositioning = false;
                        break;
                }
                break;

            default:
                break;
        }
    }
}

static void I_ReadMouse(void)
{
    int         x, y;
    event_t     ev;

    SDL_GetRelativeMouseState(&x, &y);

    ev.type = ev_mouse;
    ev.data1 = mouse_button_state;
    ev.data2 = AccelerateMouse(x);
    ev.data3 = (m_novertical ? 0 : -AccelerateMouse(y));

    D_PostEvent(&ev);

    if (MouseShouldBeGrabbed())
        CenterMouse();
}

//
// I_StartTic
//
void I_StartTic(void)
{
    I_GetEvent();
    if (m_sensitivity)
        I_ReadMouse();
    gamepadfunc();
}

dboolean currently_grabbed = false;

static void UpdateGrab(void)
{
    dboolean    grab = MouseShouldBeGrabbed();

    if (grab && !currently_grabbed)
    {
        SetShowCursor(false);
        CenterMouse();
    }
    else if (!grab && currently_grabbed)
    {
        SetShowCursor(true);

        SDL_WarpMouseInWindow(window, windowwidth - 10 * windowwidth / SCREENWIDTH,
            windowheight - 16);
        SDL_PumpEvents();
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;
}

void I_FinishUpdate(void)
{
    static int      pitch = SCREENWIDTH * sizeof(Uint32);

    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

void I_FinishUpdate_Best(void)
{
    static int      pitch = SCREENWIDTH * sizeof(Uint32);

    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void I_FinishUpdate_ShowFPS(void)
{
    static int      pitch = SCREENWIDTH * sizeof(Uint32);
    static int      frames = -1;
    static Uint32   starttime = 0;
    static Uint32   currenttime;

    UpdateGrab();

    ++frames;
    currenttime = SDL_GetTicks();
    if (currenttime - starttime >= 1000)
    {
        fps = frames;
        frames = 0;
        starttime = currenttime;
    }
    C_UpdateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

void I_FinishUpdate_Best_ShowFPS(void)
{
    static int      pitch = SCREENWIDTH * sizeof(Uint32);
    static int      frames = -1;
    static Uint32   starttime = 0;
    static Uint32   currenttime;

    UpdateGrab();

    ++frames;
    currenttime = SDL_GetTicks();
    if (currenttime - starttime >= 1000)
    {
        fps = frames;
        frames = 0;
        starttime = currenttime;
    }
    C_UpdateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void I_FinishUpdate_Shake(void)
{
    static int      pitch = SCREENWIDTH * sizeof(Uint32);

    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, M_RandomInt(-100, 100) / 100.0, NULL,
        SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

void I_FinishUpdate_Best_Shake(void)
{
    static int      pitch = SCREENWIDTH * sizeof(Uint32);

    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, M_RandomInt(-100, 100) / 100.0, NULL,
        SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void I_FinishUpdate_ShowFPS_Shake(void)
{
    static int      pitch = SCREENWIDTH * sizeof(Uint32);
    static int      frames = -1;
    static Uint32   starttime = 0;
    static Uint32   currenttime;

    UpdateGrab();

    ++frames;
    currenttime = SDL_GetTicks();
    if (currenttime - starttime >= 1000)
    {
        fps = frames;
        frames = 0;
        starttime = currenttime;
    }
    C_UpdateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, M_RandomInt(-100, 100) / 100.0, NULL,
        SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

void I_FinishUpdate_Best_ShowFPS_Shake(void)
{
    static int      pitch = SCREENWIDTH * sizeof(Uint32);
    static int      frames = -1;
    static Uint32   starttime = 0;
    static Uint32   currenttime;

    UpdateGrab();

    ++frames;
    currenttime = SDL_GetTicks();
    if (currenttime - starttime >= 1000)
    {
        fps = frames;
        frames = 0;
        starttime = currenttime;
    }
    C_UpdateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, M_RandomInt(-100, 100) / 100.0, NULL,
        SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
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
void I_SetPalette(byte *playpal)
{
    int i;

    for (i = 0; i < 256; ++i)
    {
        colors[i].r = gammatable[gammaindex][*playpal++];
        colors[i].g = gammatable[gammaindex][*playpal++];
        colors[i].b = gammatable[gammaindex][*playpal++];
    }

    SDL_SetPaletteColors(palette, colors, 0, 256);
}

static void CreateCursors(void)
{
    static Uint8 empty_cursor_data = 0;

    // Save the default cursor so it can be recalled later
    cursors[1] = SDL_GetCursor();

    // Create an empty cursor
    cursors[0] = SDL_CreateCursor(&empty_cursor_data, &empty_cursor_data, 1, 1, 0, 0);
}

void GetWindowPosition(void)
{
    int x = 0, y = 0;

    if (!strcasecmp(vid_windowposition, vid_windowposition_centered))
    {
        windowx = 0;
        windowy = 0;
    }
    else if (!sscanf(vid_windowposition, "(%10i,%10i)", &x, &y))
    {
        windowx = 0;
        windowy = 0;
        vid_windowposition = vid_windowposition_centered;

        M_SaveCVARs();
    }
    else
    {
        windowx = BETWEEN(displays[displayindex].x, x,
            displays[displayindex].x + displays[displayindex].w - 50);
        windowy = BETWEEN(displays[displayindex].y, y,
            displays[displayindex].y + displays[displayindex].h - 50);
    }
}

void GetWindowSize(void)
{
    int     width = -1;
    int     height = -1;
    char    *left = strtok(strdup(vid_windowsize), "x");
    char    *right = strtok(NULL, "x");

    if (!right)
        right = "";

    sscanf(left, "%10i", &width);
    sscanf(right, "%10i", &height);

    if (width < ORIGINALWIDTH + windowborderwidth
        || height < ORIGINALWIDTH * 3 / 4 + windowborderheight)
    {
        char    buffer[16] = "";

        windowwidth = ORIGINALWIDTH + windowborderwidth;
        windowheight = ORIGINALWIDTH * 3 / 4 + windowborderheight;
        M_snprintf(buffer, sizeof(buffer), "%ix%i", windowwidth, windowheight);
        vid_windowsize = strdup(buffer);

        M_SaveCVARs();
    }
    else
    {
        windowwidth = width;
        windowheight = height;
    }
}

dboolean ValidScreenMode(int width, int height)
{
    SDL_DisplayMode     mode;
    const int           modecount = SDL_GetNumDisplayModes(displayindex);
    int                 i;

    for (i = 0; i < modecount; i++)
    {
        SDL_GetDisplayMode(displayindex, i, &mode);
        if (width == mode.w && height == mode.h)
            return true;
    }
    return false;
}

void GetScreenResolution(void)
{
    if (!strcasecmp(vid_screenresolution, "desktop"))
    {
        screenwidth = 0;
        screenheight = 0;
    }
    else
    {
        int     width = -1;
        int     height = -1;
        char    *left = strtok(strdup(vid_screenresolution), "x");
        char    *right = strtok(NULL, "x");

        if (!right)
            right = "";

        sscanf(left, "%10i", &width);
        sscanf(right, "%10i", &height);

        if (width >= 0 && height >= 0 && ValidScreenMode(width, height))
        {
            screenwidth = width;
            screenheight = height;
        }
        else
        {
            screenwidth = 0;
            screenheight = 0;
            vid_screenresolution = "desktop";

            M_SaveCVARs();
        }
    }
}

static resolution_t resolutions[] =
{
    {  960,  640, "DVGA",   "3:2"   }, {  960,  720, "",       "4:3"   },
    { 1024,  640, "",       "16:10" }, { 1024,  768, "XGA",    "4:3"   },
    { 1136,  640, "",       "16:9"  }, { 1152,  720, "",       "3:2"   },
    { 1152,  768, "WXGA",   "3:2"   }, { 1152,  864, "XGA+",   "4:3"   },
    { 1280, 1024, "SXGA",   "5:4"   }, { 1280,  720, "WXGA",   "16:9"  },
    { 1280,  768, "WXGA",   "5:3"   }, { 1280,  800, "WXGA",   "16:10" },
    { 1280,  864, "",       "3:2"   }, { 1280,  960, "SXGA-",  "4:3"   },
    { 1280, 1024, "SXGA",   "5:4"   }, { 1360,  768, "FWXGA",  "16:9"  },
    { 1366,  768, "FWXGA",  "16:9"  }, { 1400, 1050, "SXGA+",  "4:3"   },
    { 1440,  900, "WXGA+",  "16:10" }, { 1440,  960, "FWXGA+", "3:2"   },
    { 1600, 1024, "WSXGA",  "3:2"   }, { 1600,  900, "HD+",    "16:9"  },
    { 1600, 1200, "UXGA",   "4:3"   }, { 1680, 1050, "WSXGA+", "16:10" },
    { 1792, 1344, "",       "4:3"   }, { 1856, 1392, "",       "4:3"   },
    { 1920, 1080, "FHD",    "16:9"  }, { 1920, 1200, "WUXGA",  "16:10" },
    { 1920, 1280, "",       "3:2"   }, { 1920, 1440, "",       "4:3"   },
    { 2048, 1152, "QWXGA",  "16:9"  }, { 2048, 1536, "QXGA",   "4:3"   },
    { 2160, 1440, "",       "3:2"   }, { 2560, 1080, "",       "21:9"  },
    { 2560, 1440, "QHD",    "16:9", }, { 2560, 1600, "WQXGA",  "16:10" },
    { 2560, 1920, "",       "4:3"   }, { 2560, 2048, "QSXGA",  "5:4"   },
    { 2880, 1620, "",       "16:9"  }, { 2880, 1800, "",       "16:10" },
    { 3200, 1800, "WQXGA+", "16:9"  }, { 3200, 2048, "WQSXGA", "25:16" },
    { 3200, 2400, "QUXGA",  "4:3"   }, { 3440, 1440, "",       "21:9"  },
    { 3840, 2160, "UHD",    "16:9"  }, { 3840, 2400, "WQUXGA", "16:10" },
    { 4096, 2160, "DCI",    "19:10" }, { 4096, 2560, "4K",     "16:10" },
    { 4096, 3072, "HXGA",   "4:3"   }, { 5120, 2160, "4K",     "21:9"  },
    { 5120, 2880, "UHD+",   "16:9"  }, { 5120, 3200, "WHXGA",  "16:10" },
    { 5120, 4096, "HSXGA",  "5:4"   }, { 5760, 3240, "",       "16:9"  },
    { 6400, 4096, "WHSXGA", "25:16" }, { 6400, 4800, "",       "4:3"   },
    { 7680, 4320, "FUHD",   "16:9"  }, { 7680, 4800, "WHUXGA", "16:10" },
    { 8192, 5120, "FUHD",   "16:10" }, {    0,    0, "",       ""      }
};

static char *getacronym(int width, int height)
{
    int i = 0;

    while (resolutions[i].width)
    {
        if (width == resolutions[i].width && height == resolutions[i].height)
            return resolutions[i].acronym;
        ++i;
    }
    return "";
}

static char *getaspectratio(int width, int height)
{
    int         i = 0;
    int         hcf;
    static char ratio[10];

    while (resolutions[i].width)
    {
        if (width == resolutions[i].width && height == resolutions[i].height)
            return resolutions[i].aspectratio;
        ++i;
    }

    hcf = gcd(width, height);
    width /= hcf;
    height /= hcf;

    M_snprintf(ratio, sizeof(ratio), "%i:%i", width, height);
    return ratio;
}

static void PositionOnCurrentDisplay(void)
{
    manuallypositioning = true;
    if (!windowx && !windowy)
        SDL_SetWindowPosition(window,
            displays[displayindex].x + (displays[displayindex].w - windowwidth) / 2,
            displays[displayindex].y + (displays[displayindex].h - windowheight) / 2);
    else
        SDL_SetWindowPosition(window, windowx, windowy);
}

static void SetVideoMode(dboolean output)
{
    int i;
    int flags = SDL_RENDERER_TARGETTEXTURE;

    for (i = 0; i < numdisplays; ++i)
        SDL_GetDisplayBounds(i, &displays[i]);
    displayindex = vid_display - 1;
    if (displayindex < 0 || displayindex >= numdisplays)
    {
        if (output)
            C_Warning("Unable to find display %i.", vid_display);
        displayindex = vid_display_default - 1;
    }
    if (output)
    {
        const char      *displayname = SDL_GetDisplayName(displayindex);

        if (strlen(displayname) > 1)
            C_Output("Using display %i of %i called \"%s\".", displayindex + 1, numdisplays,
                displayname);
        else
            C_Output("Using display %i of %i.", displayindex + 1, numdisplays);
    }

    if (vid_vsync)
        flags |= SDL_RENDERER_PRESENTVSYNC;

    if (!strcasecmp(vid_scalefilter, vid_scalefilter_nearest_linear))
        bestscale = true;
    else
    {
        if (strcasecmp(vid_scalefilter, vid_scalefilter_linear))
        {
            vid_scalefilter = vid_scalefilter_nearest;
            M_SaveCVARs();
        }
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter, SDL_HINT_OVERRIDE);
    }

    if (vid_scaledriver[0])
        SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, vid_scaledriver, SDL_HINT_OVERRIDE);

    GetWindowPosition();
    GetWindowSize();
    GetScreenResolution();

    if (vid_fullscreen)
    {
        if (!screenwidth && !screenheight)
        {
            char    *acronym = getacronym(displays[displayindex].w, displays[displayindex].h);
            char    *ratio = getaspectratio(displays[displayindex].w, displays[displayindex].h);

            window = SDL_CreateWindow(PACKAGE_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), 0, 0,
                (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_RESIZABLE));
            if (output)
                C_Output("Staying at the desktop resolution of %ix%i%s%s%s with a %s aspect "
                    "ratio.", displays[displayindex].w, displays[displayindex].h, (acronym[0] ?
                    " (" : " "), acronym, (acronym[0] ? ")" : ""), ratio);
        }
        else
        {
            char    *acronym = getacronym(screenwidth, screenheight);
            char    *ratio = getaspectratio(screenwidth, screenheight);

            window = SDL_CreateWindow(PACKAGE_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), screenwidth, screenheight,
                (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE));
            if (output)
                C_Output("Switched to a resolution of %ix%i%s%s%s with a %s aspect ratio.",
                    screenwidth, screenheight, (acronym[0] ? " (" : " "), acronym,
                    (acronym[0] ? ")" : ""), ratio);
        }
    }
    else
    {
        if (windowheight > displays[displayindex].h)
        {
            windowheight = displays[displayindex].h - windowborderheight;
            windowwidth = windowheight * 4 / 3;
            M_SaveCVARs();
        }

        if (!windowx && !windowy)
        {
            window = SDL_CreateWindow(PACKAGE_NAME, SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex),
                SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex), windowwidth, windowheight,
                (SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));
            if (output)
                C_Output("Created a resizable window with dimensions %ix%i and centered.",
                    windowwidth, windowheight);
        }
        else
        {
            window = SDL_CreateWindow(PACKAGE_NAME, windowx, windowy, windowwidth, windowheight,
                (SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));
            if (output)
                C_Output("Created a resizable window with dimensions %ix%i at (%i,%i).",
                    windowwidth, windowheight, windowx, windowy);
        }
    }

    SDL_GetWindowSize(window, &displaywidth, &displayheight);
    displaycenterx = displaywidth / 2;
    displaycentery = displayheight / 2;

    renderer = SDL_CreateRenderer(window, -1, flags);

    SDL_RenderSetLogicalSize(renderer, SCREENWIDTH, SCREENWIDTH * 3 / 4);

    if (output)
    {
        SDL_RendererInfo        rendererinfo;
        char                    *renderername = "unknown renderer";
        wad_file_t              *playpalwad = lumpinfo[W_CheckNumForName("PLAYPAL")]->wad_file;

        SDL_GetRendererInfo(renderer, &rendererinfo);
        if (!strcasecmp(rendererinfo.name, vid_scaledriver_direct3d))
            renderername = "Direct3D 9";
        else if (!strcasecmp(rendererinfo.name, vid_scaledriver_opengl))
            renderername = "OpenGL";
        else if (!strcasecmp(rendererinfo.name, vid_scaledriver_software))
            renderername = "software";

        if (bestscale)
            C_Output("Scaling the screen using nearest-neighbor interpolation and linear filtering"
                " in %s.", renderername);
        else if (!strcasecmp(vid_scalefilter, vid_scalefilter_linear))
            C_Output("Scaling the screen using linear filtering in %s.", renderername);
        else
            C_Output("Scaling the screen using nearest-neighbor interpolation in %s.",
                renderername);

        if (vid_capfps)
            C_Output("The framerate is capped at %i FPS.", TICRATE);
        else
        {
            if (rendererinfo.flags & SDL_RENDERER_PRESENTVSYNC)
            {
                SDL_DisplayMode displaymode;

                SDL_GetWindowDisplayMode(window, &displaymode);
                C_Output("The framerate is capped at the display's refresh rate of %iHz.",
                    displaymode.refresh_rate);
            }
            else
            {
                if (vid_vsync)
                {
                    if (!strcasecmp(rendererinfo.name, "software"))
                        C_Warning("Vertical synchronization can't be enabled in software.");
                    else
                        C_Warning("Vertical synchronization can't be enabled.");
                }
                C_Output("The framerate is uncapped.");
            }
        }

        C_Output("Using the 256-color palette from the PLAYPAL lump in %s file %s.",
            (playpalwad->type == IWAD ? "IWAD" : "PWAD"), uppercase(playpalwad->path));

        if (gammaindex == 10)
            C_Output("Gamma correction is off.");
        else
        {
            static char     buffer[128];

            M_snprintf(buffer, sizeof(buffer), "The gamma correction level is %.2f.",
                gammalevels[gammaindex]);
            if (buffer[strlen(buffer) - 1] == '0' && buffer[strlen(buffer) - 2] == '0')
                buffer[strlen(buffer) - 1] = '\0';
            C_Output(buffer);
        }
    }
    surface = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 8, 0, 0, 0, 0);
    buffer = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 32, 0, 0, 0, 0);
    SDL_FillRect(buffer, NULL, 0);
    if (bestscale)
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        SCREENWIDTH, SCREENHEIGHT);
    if (bestscale)
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear);
    texture_upscaled = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET, 3 * SCREENWIDTH, 3 * SCREENHEIGHT);
    palette = SDL_AllocPalette(256);
    SDL_SetSurfacePalette(surface, palette);

    src_rect.w = SCREENWIDTH;
    src_rect.h = SCREENHEIGHT - SBARHEIGHT * vid_widescreen;
}

void ToggleWidescreen(dboolean toggle)
{
    if (toggle)
    {
        vid_widescreen = true;

        if (returntowidescreen && r_screensize == r_screensize_max)
        {
            r_screensize = r_screensize_max - 1;
            R_SetViewSize(r_screensize);
        }

        SDL_RenderSetLogicalSize(renderer, SCREENWIDTH, SCREENHEIGHT);
        src_rect.h = SCREENHEIGHT - SBARHEIGHT;
    }
    else
    {
        vid_widescreen = false;

        ST_doRefresh();

        SDL_RenderSetLogicalSize(renderer, SCREENWIDTH, SCREENWIDTH * 3 / 4);
        src_rect.h = SCREENHEIGHT;
    }

    returntowidescreen = false;

    SDL_SetPaletteColors(palette, colors, 0, 256);
}

#if defined(WIN32)
void I_InitWindows32(void);
#endif

void I_RestartGraphics(void)
{
    FreeSurfaces();
    SetVideoMode(false);
    if (vid_widescreen)
        ToggleWidescreen(true);

#if defined(WIN32)
    I_InitWindows32();
#endif

    M_SetWindowCaption();
}

void ToggleFullscreen(void)
{
    vid_fullscreen = !vid_fullscreen;
    M_SaveCVARs();
    if (vid_fullscreen)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        C_Input("%s on", stringize(vid_fullscreen));
    }
    else
    {
        SDL_SetWindowFullscreen(window, SDL_FALSE);
        C_Input("%s off", stringize(vid_fullscreen));

        SDL_SetWindowSize(window, windowwidth, windowheight);

        displaywidth = windowwidth;
        displayheight = windowheight;
        displaycenterx = displaywidth / 2;
        displaycentery = displayheight / 2;

        PositionOnCurrentDisplay();

        if (menuactive || consoleactive || paused || gamestate != GS_LEVEL)
            SDL_WarpMouseInWindow(window, windowwidth - 10 * windowwidth / SCREENWIDTH,
                windowheight - 16);
    }
}

void I_InitGammaTables(void)
{
    int i;
    int j;

    for (i = 0; i < GAMMALEVELS; i++)
        if (gammalevels[i] == 1.0)
            for (j = 0; j < 256; j++)
                gammatable[i][j] = j;
        else
            for (j = 0; j < 256; j++)
                gammatable[i][j] = (byte)(pow((j + 1) / 256.0, 1.0 / gammalevels[i]) * 255.0);
}

void I_InitKeyboard(void)
{
#if defined(WIN32)
    if (key_alwaysrun == KEY_CAPSLOCK)
    {
        capslock = (GetKeyState(VK_CAPITAL) & 0x0001);

        if ((pm_alwaysrun && !capslock) || (!pm_alwaysrun && capslock))
        {
            keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY, (uintptr_t)0);
            keybd_event(VK_CAPITAL, 0x45, (KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP), (uintptr_t)0);
        }
    }
#endif
}

void I_InitGraphics(void)
{
    int         i = 0;
    SDL_Event   dummy;
    byte        *doompal = W_CacheLumpName("PLAYPAL", PU_CACHE);
    SDL_version linked, compiled;

    SDL_GetVersion(&linked);
    SDL_VERSION(&compiled);

    if (linked.major != compiled.major || linked.minor != compiled.minor)
        I_Error("The wrong version of SDL2.DLL was found. "PACKAGE_NAME" requires v%d.%d.%d, "
            "not v%d.%d.%d.", compiled.major, compiled.minor, compiled.patch, linked.major,
            linked.minor, linked.patch);

    if (linked.patch != compiled.patch)
        C_Warning("The wrong version of SDL2.DLL was found. "PACKAGE_NAME" requires v%d.%d.%d, "
            "not v%d.%d.%d.", compiled.major, compiled.minor, compiled.patch, linked.major,
            linked.minor, linked.patch);

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

#if !defined(WIN32)
    if (vid_driver && strlen(vid_driver) > 0)
    {
        M_snprintf(envstring, sizeof(envstring), "SDL_VIDEODRIVER=%s", vid_driver);
        putenv(envstring);
    }
#endif

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
        I_Error("I_InitGraphics: %s", SDL_GetError());

    CreateCursors();
    SDL_SetCursor(cursors[0]);

    numdisplays = SDL_GetNumVideoDisplays();
    displays = Z_Malloc(numdisplays, PU_STATIC, NULL);

#if defined (_DEBUG)
    vid_fullscreen = false;
#endif

    SetVideoMode(true);

#if defined(WIN32)
    I_InitWindows32();
#endif

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    SDL_SetWindowTitle(window, PACKAGE_NAME);

    I_SetPalette(doompal);

    UpdateFocus();

    screens[0] = surface->pixels;

    updatefunc = (bestscale ? I_FinishUpdate_Best : I_FinishUpdate);
    updatefunc();

    while (SDL_PollEvent(&dummy));
}
