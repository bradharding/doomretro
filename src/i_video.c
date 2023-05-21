/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

========================================================================
*/

#if defined(_WIN32)
#include <Windows.h>
#include <mmsystem.h>
#elif defined(X11)
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#endif

#include <math.h>

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_main.h"
#include "doomstat.h"
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
#include "m_random.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"

#if defined(_WIN32)
void I_InitWindows32(void);
#endif

#define SHAKEANGLE  ((double)M_BigRandomInt(-1000, 1000) * r_shake_damage / 100000.0)

int                 SCREENWIDTH;
int                 SCREENHEIGHT = VANILLAHEIGHT * SCREENSCALE;
int                 SCREENAREA;
int                 WIDESCREENDELTA;
int                 MAXWIDESCREENDELTA;
int                 WIDEFOVDELTA;

static int          WIDESCREENWIDTH;

bool                nowidescreen = false;

int                 MAPWIDTH;
unsigned int        MAPHEIGHT = VANILLAHEIGHT * SCREENSCALE;
unsigned int        MAPAREA;
int                 MAPBOTTOM;

static bool         manuallypositioning;

SDL_Window          *window = NULL;
static int          windowid;
SDL_Renderer        *renderer;
static SDL_Texture  *texture;
static SDL_Texture  *texture_upscaled;
static SDL_Surface  *surface;
static SDL_Surface  *buffer;
static byte         *pixels;
static int          pitch;
static SDL_Palette  *palette;
static SDL_Color    colors[256];
byte                *PLAYPAL;

byte                *mapscreen;
SDL_Window          *mapwindow = NULL;
SDL_Renderer        *maprenderer;
static SDL_Texture  *maptexture;
static SDL_Texture  *maptexture_upscaled;
static SDL_Surface  *mapsurface;
static SDL_Surface  *mapbuffer;
static byte         *mappixels;
static int          mappitch;
static SDL_Palette  *mappalette;

static bool         nearestlinear;
static int          upscaledwidth;
static int          upscaledheight;

static bool         software;

static int          displayindex;
static int          numdisplays;
static SDL_Rect     displays[vid_display_max];

static int          mousepointerx, mousepointery;

// Bit mask of mouse button state
static unsigned int mousebuttonstate;

static const int buttons[MAX_MOUSE_BUTTONS + 1] =
{
    0x0000, 0x0001, 0x0004, 0x0002, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080
};

// Fullscreen width and height
static int          screenwidth;
static int          screenheight;

// Window width and height
int                 windowwidth;
int                 windowheight;

int                 windowx;
int                 windowy;

static int          displaywidth;
static int          displayheight;
static int          displaycenterx;
static int          displaycentery;

bool                usinggamecontroller = false;
bool                usingmouse = false;
bool                windowfocused = true;

int                 keydown = 0;

static bool         keys[NUMKEYS];

static byte         gammatable[GAMMALEVELS][256];
static double       saturationtable[256][256][256];

const float gammalevels[GAMMALEVELS] =
{
    // Darker
    0.50f, 0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f,

    // No gamma correction
    1.0f,

    // Lighter
    1.10f, 1.20f, 1.30f, 1.40f, 1.50f, 1.60f, 1.70f, 1.80f, 1.90f, 2.0f
};

int                 gammaindex;

static SDL_Rect     src_rect;
static SDL_Rect     map_rect;

int                 framespersecond = 0;
int                 refreshrate;

#if defined(_WIN32)
HANDLE              CapFPSEvent;
#endif

static bool         capslock;

evtype_t            lasteventtype = ev_none;

int                 windowborderwidth = 0;
int                 windowborderheight = 0;

bool MouseShouldBeGrabbed(void)
{
    // if the window doesn't have focus, never grab it
    if (!windowfocused)
        return false;

    // if not fullscreen, only grab the mouse when not playing a game
    if (!vid_fullscreen)
        return (gamestate == GS_LEVEL && !menuactive && !consoleactive);

    // grab the mouse when on the splash screen
    if (splashscreen)
        return true;

    // when menu is active, release the mouse
    if (((menuactive && !helpscreen) || consoleactive || gamestate != GS_LEVEL)
        && m_pointer && usingmouse && !usinggamecontroller)
        return false;

    return true;
}

static void SetShowCursor(bool show)
{
    SDL_PumpEvents();
    SDL_SetRelativeMouseMode(!show);
    SDL_GetRelativeMouseState(NULL, NULL);
}

static const int translatekey[] =
{
    0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', KEY_ENTER,
    KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, ' ', KEY_MINUS, KEY_EQUALS, '[', ']', '\\', 0, ';', '\'', '`', ',',
    '.', '/', KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_F11, KEY_F12, KEY_PRINTSCREEN, KEY_SCROLLLOCK, KEY_PAUSE, KEY_INSERT, KEY_HOME, KEY_PAGEUP,
    KEY_DELETE, KEY_END, KEY_PAGEDOWN, KEY_RIGHTARROW, KEY_LEFTARROW, KEY_DOWNARROW, KEY_UPARROW,
    KEY_NUMLOCK, KEYP_DIVIDE, KEYP_MULTIPLY, KEYP_MINUS, KEYP_PLUS, KEYP_ENTER, KEYP_1, KEYP_2, KEYP_3,
    KEYP_4, KEYP_5, KEYP_6, KEYP_7, KEYP_8, KEYP_9, KEYP_0, KEYP_PERIOD, 0, 0, 0, KEYP_EQUALS, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_CTRL, KEY_SHIFT, KEY_ALT, 0, KEY_CTRL, KEY_SHIFT, KEY_ALT
};

bool keystate(int key)
{
    const uint8_t   *state = SDL_GetKeyboardState(NULL);

    return state[SDL_GetScancodeFromKey(key)];
}

void I_CapFPS(int cap)
{
#if defined(_WIN32)
    static unsigned int CapFPSTimer;

    if (CapFPSTimer)
    {
        timeKillEvent(CapFPSTimer);
        CapFPSTimer = 0;
    }

    if (!cap)
    {
        if (CapFPSEvent)
        {
            CloseHandle(CapFPSEvent);
            CapFPSEvent = NULL;
        }
    }
    else
    {
        if (!CapFPSEvent)
            CapFPSEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

        if (CapFPSEvent)
        {
            CapFPSTimer = timeSetEvent((unsigned int)(1000.0 / cap + 0.5), 0,
                (LPTIMECALLBACK)CapFPSEvent, 0, (TIME_PERIODIC | TIME_CALLBACK_EVENT_SET));

            if (!CapFPSTimer)
            {
                CloseHandle(CapFPSEvent);
                CapFPSEvent = NULL;
            }
        }
    }
#endif
}

#if defined(_WIN32)
static void ToggleCapsLockState(void)
{
    keybd_event(VK_CAPITAL, 0x45, 0, (uintptr_t)0);
    keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_KEYUP, (uintptr_t)0);
}
#elif defined(X11)
static void SetCapsLockState(bool enabled)
{
    Display *dpy = XOpenDisplay(0);

    XkbLockModifiers(dpy, XkbUseCoreKbd, 2, enabled * 2);
    XFlush(dpy);
    XCloseDisplay(dpy);
}
#endif

bool GetCapsLockState(void)
{
#if defined(_WIN32)
    return (GetKeyState(VK_CAPITAL) & 0xFFFF);
#else
    return (SDL_GetModState() & KMOD_CAPS);
#endif
}

void I_ShutdownKeyboard(void)
{
#if defined(_WIN32)
    if (keyboardalwaysrun == KEY_CAPSLOCK && !capslock && GetCapsLockState())
        ToggleCapsLockState();
#elif defined(X11)
    if (keyboardalwaysrun == KEY_CAPSLOCK)
        SetCapsLockState(false);
#endif
}

static int AccelerateMouse(int value)
{
    return (value < -10 ? value * 2 + 10 : (value < 10 ? value : value * 2 - 10));
}

static short inline clamp(short value, short deadzone)
{
    return (ABS(value) < deadzone ? 0 : (joy_analog ? MAX(-SDL_JOYSTICK_AXIS_MAX, value) : SIGN(value) * SDL_JOYSTICK_AXIS_MAX));
}

bool    altdown = false;
bool    waspaused = false;

static const SDL_Scancode keypad[] =
{
    SDL_SCANCODE_KP_1,  SDL_SCANCODE_DOWN, SDL_SCANCODE_KP_3, SDL_SCANCODE_LEFT, SDL_SCANCODE_KP_5,
    SDL_SCANCODE_RIGHT, SDL_SCANCODE_KP_7, SDL_SCANCODE_UP,   SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0
};

static void I_GetEvent(void)
{
    SDL_Event   SDLEvent = { 0 };
    SDL_Event   *Event = &SDLEvent;

    SDL_PumpEvents();

    while (SDL_PollEvent(Event))
    {
        event_t     ev = { 0 };

#if !defined(_WIN32)
        static bool enterdown;
#endif

        switch (Event->type)
        {
            case SDL_KEYDOWN:
            {
                const SDL_Scancode  scancode = Event->key.keysym.scancode;

                ev.type = ev_keydown;

                if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_0 && !SDL_IsTextInputActive())
                    ev.data1 = translatekey[keypad[scancode - SDL_SCANCODE_KP_1]];
                else if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_RALT)
                    ev.data1 = translatekey[scancode];

                ev.data2 = Event->key.keysym.sym;

                if (ev.data2 < SDLK_SPACE || ev.data2 > SDLK_z)
                    ev.data2 = 0;

                altdown = (Event->key.keysym.mod & KMOD_ALT);

                if (ev.data1)
                {
                    if (altdown && ev.data1 == KEY_F4)
                        M_QuitResponse('y');

                    if (!isdigit(ev.data2))
                    {
                        idclev = false;
                        idmus = false;
                    }

                    if (idbehold && keys[ev.data2])
                    {
                        idbehold = false;
                        HU_ClearMessages();
                        C_Cheat(cheat_powerup[6].sequence);
                        C_Output(s_STSTR_BEHOLD);
                    }

#if !defined(_WIN32)
                    // Handle ALT+ENTER on non-Windows systems
                    if (altdown && ev.data1 == KEY_ENTER && !enterdown)
                    {
                        enterdown = true;
                        I_ToggleFullscreen();

                        return;
                    }
#endif

                    D_PostEvent(&ev);

                    if (gamestate != GS_LEVEL)
                        I_SaveMousePointerPosition();

                    usingmouse = false;
                }

                break;
            }

            case SDL_KEYUP:
            {
                const SDL_Scancode  scancode = Event->key.keysym.scancode;

                ev.type = ev_keyup;

                if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_0 && !SDL_IsTextInputActive())
                    ev.data1 = translatekey[keypad[scancode - SDL_SCANCODE_KP_1]];
                else if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_RALT)
                    ev.data1 = translatekey[scancode];

                altdown = (Event->key.keysym.mod & KMOD_ALT);
                keydown = 0;

#if !defined(_WIN32)
                // Handle ALT+ENTER on non-Windows systems
                if (ev.data1 == KEY_ENTER)
                    enterdown = false;
#endif

                if (ev.data1)
                    D_PostEvent(&ev);

                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                idclev = false;
                idmus = false;

                if (idbehold)
                {
                    HU_ClearMessages();
                    idbehold = false;
                    C_Cheat(cheat_powerup[6].sequence);
                    C_Output(s_STSTR_BEHOLD);
                }

                mousebuttonstate |= buttons[Event->button.button];
                break;

            case SDL_MOUSEBUTTONUP:
                keydown = 0;
                mousebuttonstate &= ~buttons[Event->button.button];
                break;

            case SDL_MOUSEWHEEL:
                keydown = 0;
                ev.type = ev_mousewheel;
                ev.data1 = Event->wheel.y;

                if (!consoleactive)
                    usingmouse = false;

                D_PostEvent(&ev);
                break;

            case SDL_CONTROLLERAXISMOTION:
                switch (Event->caxis.axis)
                {
                    case SDL_CONTROLLER_AXIS_LEFTX:
                        if (joy_swapthumbsticks)
                            gamecontrollerthumbRX = clamp(Event->caxis.value, gamecontrollerrightdeadzone);
                        else
                            gamecontrollerthumbLX = clamp(Event->caxis.value, gamecontrollerleftdeadzone);

                        break;

                    case SDL_CONTROLLER_AXIS_LEFTY:
                        if (joy_swapthumbsticks)
                            gamecontrollerthumbRY = clamp(Event->caxis.value, gamecontrollerrightdeadzone);
                        else
                            gamecontrollerthumbLY = clamp(Event->caxis.value, gamecontrollerleftdeadzone);

                        break;

                    case SDL_CONTROLLER_AXIS_RIGHTX:
                        if (joy_swapthumbsticks)
                            gamecontrollerthumbLX = clamp(Event->caxis.value, gamecontrollerleftdeadzone);
                        else
                            gamecontrollerthumbRX = clamp(Event->caxis.value, gamecontrollerrightdeadzone);

                        break;

                    case SDL_CONTROLLER_AXIS_RIGHTY:
                        if (joy_swapthumbsticks)
                            gamecontrollerthumbLY = clamp(Event->caxis.value, gamecontrollerleftdeadzone);
                        else
                            gamecontrollerthumbRY = clamp(Event->caxis.value, gamecontrollerrightdeadzone);

                        break;

                    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                        if (Event->caxis.value >= GAMECONTROLLER_TRIGGER_THRESHOLD)
                            gamecontrollerbuttons |= GAMECONTROLLER_LEFT_TRIGGER;
                        else
                            gamecontrollerbuttons &= ~GAMECONTROLLER_LEFT_TRIGGER;

                        break;

                    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                        if (Event->caxis.value >= GAMECONTROLLER_TRIGGER_THRESHOLD)
                            gamecontrollerbuttons |= GAMECONTROLLER_RIGHT_TRIGGER;
                        else
                            gamecontrollerbuttons &= ~GAMECONTROLLER_RIGHT_TRIGGER;

                        break;
                }

                if (gamestate != GS_LEVEL)
                    I_SaveMousePointerPosition();

                usingmouse = false;
                ev.type = ev_controller;
                D_PostEvent(&ev);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                gamecontrollerbuttons |= (1 << Event->cbutton.button);
                ev.type = ev_controller;
                D_PostEvent(&ev);

                if (gamestate != GS_LEVEL)
                    I_SaveMousePointerPosition();

                usingmouse = false;
                break;

            case SDL_CONTROLLERBUTTONUP:
                gamecontrollerbuttons &= ~(1 << Event->cbutton.button);
                keydown = 0;
                ev.type = ev_controller;
                D_PostEvent(&ev);

                if (gamestate != GS_LEVEL)
                    I_SaveMousePointerPosition();

                usingmouse = false;
                break;

            case SDL_TEXTINPUT:
            {
                void    *text = SDL_iconv_utf8_ucs4(Event->text.text);

                ev.data1 = (text ? ((char *)text)[0] : Event->text.text[strlen(Event->text.text) - 1]);
                ev.type = ev_textinput;
                D_PostEvent(&ev);
                break;
            }

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

                    M_OpenMainMenu();
                    S_StartSound(NULL, sfx_swtchn);
                    M_QuitDOOM(0);
                }

                break;

            case SDL_WINDOWEVENT:
                if (Event->window.windowID == windowid)
                {
                    switch (Event->window.event)
                    {
                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                            if (!windowfocused)
                            {
                                windowfocused = true;
                                S_ResumeMusic();
                                I_InitKeyboard();
                            }

                            break;

                        case SDL_WINDOWEVENT_FOCUS_LOST:
                        case SDL_WINDOWEVENT_MINIMIZED:
                            windowfocused = false;

                            if (!s_musicinbackground)
                                S_PauseMusic();

                            if (gamestate == GS_LEVEL && !menuactive && !consoleactive && !paused)
                                sendpause = true;

                            I_ShutdownKeyboard();
                            break;

                        case SDL_WINDOWEVENT_EXPOSED:
                            SDL_SetPaletteColors(palette, colors, 0, 256);
                            break;

                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            if (!vid_fullscreen)
                            {
                                char    *temp1 = commify((windowwidth = Event->window.data1));
                                char    *temp2 = commify((windowheight = Event->window.data2));
                                char    size[16];

                                M_snprintf(size, sizeof(size), "%sx%s", temp1, temp2);
                                vid_windowsize = M_StringDuplicate(size);
                                M_SaveCVARs();

                                displaywidth = windowwidth;
                                displayheight = windowheight;
                                displaycenterx = displaywidth / 2;
                                displaycentery = displayheight / 2;

                                free(temp1);
                                free(temp2);

                                I_RestartGraphics(false);
                            }

                            break;

                        case SDL_WINDOWEVENT_MOVED:
                            if (!vid_fullscreen && !manuallypositioning)
                            {
                                char    pos[16];

                                windowx = Event->window.data1;
                                windowy = Event->window.data2;
                                M_snprintf(pos, sizeof(pos), "(%i,%i)", windowx, windowy);
                                vid_windowpos = M_StringDuplicate(pos);
                                vid_display = SDL_GetWindowDisplayIndex(window) + 1;
                                M_SaveCVARs();
                            }

                            manuallypositioning = false;
                            break;
                    }
                }

                break;
        }
    }
}

void I_SaveMousePointerPosition(void)
{
    SDL_GetMouseState(&mousepointerx, &mousepointery);
}

void I_RestoreMousePointerPosition(void)
{
    SDL_WarpMouseInWindow(window, mousepointerx, mousepointery);
}

static void SmoothMouse(int *x, int *y)
{
    const fixed_t   tic = ((I_GetTimeMS() * TICRATE) % 1000) * FRACUNIT / 1000;
    const fixed_t   adjustment = FixedDiv(tic, FRACUNIT + tic);
    static int      xx, yy;

    *x += xx;
    xx = FixedMul(*x, adjustment);
    *x -= xx;

    *y += yy;
    yy = FixedMul(*y, adjustment);
    *y -= yy;
}

static void I_ReadMouse(void)
{
    int         x, y;
    static int  prevmousebuttonstate;

    SDL_GetRelativeMouseState(&x, &y);

    if (x || y || (mousebuttonstate != prevmousebuttonstate || (mousebuttonstate && menuactive)))
    {
        event_t ev = { 0 };

        ev.type = ev_mouse;
        ev.data1 = mousebuttonstate;

        if (((menuactive && !helpscreen) || consoleactive || gamestate != GS_LEVEL)
            && !splashscreen && m_pointer)
        {
            if (x || y)
                usingmouse = true;

            SDL_GetMouseState(&x, &y);

            ev.data2 = x * WIDESCREENWIDTH / displaywidth / SCREENSCALE;
            ev.data3 = y * SCREENHEIGHT / displayheight / SCREENSCALE;
        }
        else
        {
            SmoothMouse(&x, &y);

            if (m_acceleration)
            {
                ev.data2 = AccelerateMouse(x);
                ev.data3 = AccelerateMouse(y);
            }
            else
            {
                ev.data2 = x;
                ev.data3 = y;
            }
        }

        prevmousebuttonstate = mousebuttonstate;
        D_PostEvent(&ev);
    }
}

//
// I_StartTic
//
void I_StartTic(void)
{
    I_GetEvent();
    I_ReadMouse();
    I_UpdateGameControllerRumble();
}

static void UpdateGrab(void)
{
    bool        grab = MouseShouldBeGrabbed();
    static bool currently_grabbed;

    if (grab == currently_grabbed)
        return;

    if (grab && !currently_grabbed)
        SetShowCursor(false);
    else if (!grab && currently_grabbed)
        SetShowCursor(true);

    currently_grabbed = grab;
}

static void GetUpscaledTextureSize(int width, int height)
{
    if (width * ACTUALHEIGHT < height * SCREENWIDTH)
        height = width * ACTUALHEIGHT / SCREENWIDTH;
    else
        width = height * SCREENWIDTH / ACTUALHEIGHT;

    upscaledwidth = (width + SCREENWIDTH - 1) / SCREENWIDTH;
    upscaledheight = (height + SCREENHEIGHT - 1) / SCREENHEIGHT;
}

void (*blitfunc)(void);
void (*mapblitfunc)(void);

static void (*clearframefunc)(void);
static void nullfunc(void) {}

static uint64_t performancefrequency;
uint64_t        starttime;
int             frames = -1;

static void CalculateFPS(void)
{
    const uint64_t  currenttime = SDL_GetPerformanceCounter();

    frames++;

    if (starttime < currenttime - performancefrequency)
    {
        framespersecond = frames;
        frames = 0;
        starttime = currenttime;
    }
}

#if defined(_WIN32)
void I_WindowResizeBlit(void)
{
    if (vid_showfps)
        CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    SDL_RenderClear(renderer);

    if (nearestlinear)
    {
        SDL_SetRenderTarget(renderer, texture_upscaled);
        SDL_RenderCopy(renderer, texture, &src_rect, NULL);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    }
    else
        SDL_RenderCopy(renderer, texture, &src_rect, NULL);

    SDL_RenderPresent(renderer);
}
#endif

static void I_Blit(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    clearframefunc();
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    clearframefunc();
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_ShowFPS(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    clearframefunc();
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_ShowFPS(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    clearframefunc();
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_Shake(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    clearframefunc();
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_Shake(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    clearframefunc();
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_ShowFPS_Shake(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    clearframefunc();
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_ShowFPS_Shake(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    clearframefunc();
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_Automap(void)
{
    SDL_LowerBlit(mapsurface, &map_rect, mapbuffer, &map_rect);
    SDL_UpdateTexture(maptexture, &map_rect, mappixels, mappitch);
    SDL_RenderClear(maprenderer);
    SDL_RenderCopy(maprenderer, maptexture, &map_rect, NULL);
    SDL_RenderPresent(maprenderer);
}

static void I_Blit_Automap_NearestLinear(void)
{
    SDL_LowerBlit(mapsurface, &map_rect, mapbuffer, &map_rect);
    SDL_UpdateTexture(maptexture, &map_rect, mappixels, mappitch);
    SDL_RenderClear(maprenderer);
    SDL_SetRenderTarget(maprenderer, maptexture_upscaled);
    SDL_RenderCopy(maprenderer, maptexture, &map_rect, NULL);
    SDL_SetRenderTarget(maprenderer, NULL);
    SDL_RenderCopy(maprenderer, maptexture_upscaled, NULL, NULL);
    SDL_RenderPresent(maprenderer);
}

void I_UpdateBlitFunc(bool shaking)
{
    const bool  nearest = (nearestlinear && (displayheight % VANILLAHEIGHT));

    if (shaking && !software)
        blitfunc = (nearest ?
            (vid_showfps ? &I_Blit_NearestLinear_ShowFPS_Shake : &I_Blit_NearestLinear_Shake) :
            (vid_showfps ? &I_Blit_ShowFPS_Shake : &I_Blit_Shake));
    else
        blitfunc = (nearest ?
            (vid_showfps ? &I_Blit_NearestLinear_ShowFPS : &I_Blit_NearestLinear) :
            (vid_showfps ? &I_Blit_ShowFPS : &I_Blit));

    mapblitfunc = (mapwindow ? (nearest ? &I_Blit_Automap_NearestLinear : &I_Blit_Automap) : &nullfunc);
}

//
// I_SetPalette
//
void I_SetPalette(byte *playpal)
{
    byte    *gamma = gammatable[gammaindex];

    if (r_saturation == r_saturation_default)
        for (int i = 0; i < 256; i++)
        {
            colors[i].r = gamma[*playpal++];
            colors[i].g = gamma[*playpal++];
            colors[i].b = gamma[*playpal++];
        }
    else
    {
        const double    saturation = r_saturation / 100.0;

        for (int i = 0; i < 256; i++)
        {
            const byte      r = gamma[*playpal++];
            const byte      g = gamma[*playpal++];
            const byte      b = gamma[*playpal++];
            const double    p = saturationtable[r][g][b];

            colors[i].r = (byte)BETWEENF(0, p + (r - p) * saturation, 255);
            colors[i].g = (byte)BETWEENF(0, p + (g - p) * saturation, 255);
            colors[i].b = (byte)BETWEENF(0, p + (b - p) * saturation, 255);
        }
    }

    SDL_SetPaletteColors(palette, colors, 0, 256);

    if (vid_pillarboxes)
        SDL_SetRenderDrawColor(renderer, colors[0].r, colors[0].g, colors[0].b, SDL_ALPHA_OPAQUE);
}

void I_SetExternalAutomapPalette(void)
{
    if (mapwindow)
    {
        SDL_SetPaletteColors(mappalette, colors, 0, 256);
        mapblitfunc();
    }
}

void I_SetPaletteWithBrightness(byte *playpal, double brightness)
{
    byte    *gamma = gammatable[gammaindex];

    if (r_saturation == r_saturation_default)
        for (int i = 0; i < 256; i++)
        {
            colors[i].r = (byte)(gamma[*playpal++] * brightness);
            colors[i].g = (byte)(gamma[*playpal++] * brightness);
            colors[i].b = (byte)(gamma[*playpal++] * brightness);
        }
    else
    {
        const double    saturation = r_saturation / 100.0;

        for (int i = 0; i < 256; i++)
        {
            const double    r = gamma[*playpal++] * brightness;
            const double    g = gamma[*playpal++] * brightness;
            const double    b = gamma[*playpal++] * brightness;
            const double    p = sqrt(r * r * 0.299 + g * g * 0.587 + b * b * 0.114);

            colors[i].r = (byte)BETWEENF(0, p + (r - p) * saturation, 255);
            colors[i].g = (byte)BETWEENF(0, p + (g - p) * saturation, 255);
            colors[i].b = (byte)BETWEENF(0, p + (b - p) * saturation, 255);
        }
    }

    SDL_SetPaletteColors(palette, colors, 0, 256);
}

static void GetDisplays(void)
{
    numdisplays = MIN(SDL_GetNumVideoDisplays(), vid_display_max);

    for (int i = 0; i < numdisplays; i++)
        SDL_GetDisplayBounds(i, &displays[i]);

    if ((double)displays[displayindex].w / displays[displayindex].h <= NONWIDEASPECTRATIO)
    {
        nowidescreen = true;
        vid_widescreen = false;
    }
}

bool I_CreateExternalAutomap(void)
{
    uint32_t    pixelformat;
    uint32_t    rmask;
    uint32_t    gmask;
    uint32_t    bmask;
    uint32_t    amask;
    int         bpp;
    const char  *displayname;

    mapscreen = *screens;
    mapblitfunc = &nullfunc;

    if (!am_external)
        return false;

    GetDisplays();

    if (am_display == vid_display)
    {
        if (!togglingvanilla)
            C_Warning(1, "The external automap can't be shown on this display.");

        return false;
    }
    else if (am_display > numdisplays)
    {
        if (!togglingvanilla)
            C_Warning(1, "The external automap can't be shown. Display %i wasn't found.", am_display);

        return false;
    }

    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0", SDL_HINT_OVERRIDE);

    mapwindow = SDL_CreateWindow("Automap", SDL_WINDOWPOS_UNDEFINED_DISPLAY(am_display - 1),
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(am_display - 1), 0, 0, (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SKIP_TASKBAR));

    MAPHEIGHT = VANILLAHEIGHT * SCREENSCALE;
    MAPWIDTH = MIN(((displays[am_display - 1].w * MAPHEIGHT / displays[am_display - 1].h + 1) & ~3), MAXWIDTH);
    MAPAREA = MAPWIDTH * MAPHEIGHT;

    maprenderer = SDL_CreateRenderer(mapwindow, -1, SDL_RENDERER_TARGETTEXTURE);
    SDL_RenderSetLogicalSize(maprenderer, MAPWIDTH, MAPHEIGHT);
    mapsurface = SDL_CreateRGBSurface(0, MAPWIDTH, MAPHEIGHT, 8, 0, 0, 0, 0);
    pixelformat = SDL_GetWindowPixelFormat(mapwindow);
    SDL_PixelFormatEnumToMasks(pixelformat, &bpp, &rmask, &gmask, &bmask, &amask);
    mapbuffer = SDL_CreateRGBSurface(0, MAPWIDTH, MAPHEIGHT, bpp, rmask, gmask, bmask, amask);

    mappitch = mapbuffer->pitch;
    mappixels = mapbuffer->pixels;

    SDL_FillRect(mapbuffer, NULL, 0);

    if (nearestlinear)
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE);

    maptexture = SDL_CreateTexture(maprenderer, pixelformat, SDL_TEXTUREACCESS_STREAMING, MAPWIDTH, MAPHEIGHT);

    if (nearestlinear)
    {
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear, SDL_HINT_OVERRIDE);
        maptexture_upscaled = SDL_CreateTexture(maprenderer, pixelformat, SDL_TEXTUREACCESS_TARGET,
            upscaledwidth * MAPWIDTH, upscaledheight * MAPHEIGHT);
        mapblitfunc = &I_Blit_Automap_NearestLinear;
    }
    else
        mapblitfunc = &I_Blit_Automap;

    mappalette = SDL_AllocPalette(256);
    SDL_SetSurfacePalette(mapsurface, mappalette);
    SDL_SetPaletteColors(mappalette, colors, 0, 256);
    mapscreen = mapsurface->pixels;
    memset(mapscreen, nearestblack, MAPAREA);

    map_rect.w = MAPWIDTH;
    map_rect.h = MAPHEIGHT;

    if ((displayname = SDL_GetDisplayName(am_display - 1)))
        C_Output("Using \"%s\" (display %i of %i) to show the external automap.", displayname, am_display, numdisplays);
    else
        C_Output("Using display %i of %i to show the external automap.", am_display, numdisplays);

    return true;
}

void I_DestroyExternalAutomap(void)
{
    SDL_DestroyWindow(mapwindow);
    mapwindow = NULL;
    mapblitfunc = &nullfunc;
}

void GetWindowPosition(void)
{
    int x = 0;
    int y = 0;

    if (M_StringCompare(vid_windowpos, vid_windowpos_centered) || M_StringCompare(vid_windowpos, vid_windowpos_centred))
    {
        windowx = 0;
        windowy = 0;
    }
    else if (sscanf(vid_windowpos, "(%10i,%10i)", &x, &y) != 2)
    {
        windowx = 0;
        windowy = 0;
        vid_windowpos = vid_windowpos_centered;
        M_SaveCVARs();
    }
    else
    {
        windowx = BETWEEN(displays[displayindex].x, x, displays[displayindex].x + displays[displayindex].w - 50);
        windowy = BETWEEN(displays[displayindex].y, y, displays[displayindex].y + displays[displayindex].h - 50);
    }
}

void GetWindowSize(void)
{
    char    width[11] = "";
    char    height[11] = "";

    if (sscanf(vid_windowsize, "%10[^x]x%10[^x]", width, height) == 2)
    {
        char    *temp1 = uncommify(width);
        char    *temp2 = uncommify(height);

        windowwidth = strtol(temp1, NULL, 10);
        windowheight = strtol(temp2, NULL, 10);
        free(temp1);
        free(temp2);
    }
    else
    {
        windowheight = SCREENHEIGHT + windowborderheight;
        windowwidth = SCREENHEIGHT * 16 / 10 + windowborderwidth;
        vid_windowsize = vid_windowsize_default;
        M_SaveCVARs();
    }
}

static bool ValidScreenMode(int width, int height)
{
    const int   modes = SDL_GetNumDisplayModes(displayindex);

    for (int i = 0; i < modes; i++)
    {
        SDL_DisplayMode mode;

        SDL_GetDisplayMode(displayindex, i, &mode);

        if (width == mode.w && height == mode.h)
            return true;
    }

    return false;
}

void GetScreenResolution(void)
{
    if (M_StringCompare(vid_screenresolution, vid_screenresolution_desktop))
    {
        screenwidth = 0;
        screenheight = 0;
    }
    else
    {
        int width;
        int height;

        if (sscanf(vid_screenresolution, "%10ix%10i", &width, &height) != 2 || !ValidScreenMode(width, height))
        {
            screenwidth = 0;
            screenheight = 0;
            vid_screenresolution = vid_screenresolution_desktop;
            M_SaveCVARs();
        }
        else
        {
            screenwidth = width;
            screenheight = height;
        }
    }
}

static char *getaspectratio(int width, int height)
{
    int         hcf = gcd(width, height);
    static char ratio[10];

    width /= hcf;
    height /= hcf;

    if (width == 8)
    {
        width = 16;
        height *= 2;
    }

    M_snprintf(ratio, sizeof(ratio), "%i:%i", width, height);
    return ratio;
}

static void PositionOnCurrentDisplay(void)
{
    manuallypositioning = true;

    if (windowx || windowy)
        SDL_SetWindowPosition(window, windowx, windowy);
    else
        SDL_SetWindowPosition(window, displays[displayindex].x + (displays[displayindex].w - windowwidth) / 2,
            displays[displayindex].y + (displays[displayindex].h - windowheight) / 2);
}

void I_SetMotionBlur(int percent)
{
    if (percent)
    {
        SDL_SetSurfaceAlphaMod(surface, SDL_ALPHA_OPAQUE - 128 * percent / 100);
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    }
    else
    {
        SDL_SetSurfaceAlphaMod(surface, SDL_ALPHA_OPAQUE);
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    }
}

static void SetVideoMode(bool createwindow, bool output)
{
    int                 rendererflags = SDL_RENDERER_TARGETTEXTURE;
    int                 windowflags = (SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    int                 width, height;
    uint32_t            pixelformat;
    uint32_t            rmask;
    uint32_t            gmask;
    uint32_t            bmask;
    uint32_t            amask;
    int                 bpp = 0;
    SDL_RendererInfo    rendererinfo;
    const char          *displayname = SDL_GetDisplayName((displayindex = vid_display - 1));
    bool                instead = false;

    if (displayindex >= numdisplays)
    {
        if (output)
            C_Warning(1, "Display %i couldn't be found.", vid_display);

        displayname = SDL_GetDisplayName((displayindex = vid_display_default - 1));
        instead = true;
    }

    if (output)
    {
        if (displayname)
            C_Output("Using \"%s\" (display %i of %i)%s.",
                displayname, displayindex + 1, numdisplays, (instead ? " instead" : ""));
        else
            C_Output("Using display %i of %i%s.",
                displayindex + 1, numdisplays, (instead ? " instead" : ""));
    }

    if (nowidescreen && output)
    {
        consolecmds[C_GetIndex(stringize(vid_widescreen))].flags |= CF_READONLY;
        C_Warning(1, "The aspect ratio of display %i is too low to show " ITALICS(DOOMRETRO_NAME) " in widescreen.",
            displayindex + 1);
    }

    if (vid_vsync)
        rendererflags |= SDL_RENDERER_PRESENTVSYNC;

    if (M_StringCompare(vid_scalefilter, vid_scalefilter_nearest_linear))
        nearestlinear = true;
    else
    {
        nearestlinear = false;

        if (!M_StringCompare(vid_scalefilter, vid_scalefilter_linear)
            && !M_StringCompare(vid_scalefilter, vid_scalefilter_nearest))
        {
            vid_scalefilter = vid_scalefilter_default;
            M_SaveCVARs();
        }

        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter, SDL_HINT_OVERRIDE);
    }

    SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, vid_scaleapi, SDL_HINT_OVERRIDE);

    software = M_StringCompare(vid_scaleapi, vid_scaleapi_software);

    GetWindowPosition();
    GetWindowSize();
    GetScreenResolution();

    if (vid_fullscreen)
    {
        if (!screenwidth && !screenheight)
        {
            width = displays[displayindex].w;
            height = displays[displayindex].h;

            if (!width || !height)
                I_Error("Graphics couldn't be %s.", (english == english_american ? "initialized" : "initialised"));

            if (createwindow)
                window = SDL_CreateWindow(DOOMRETRO_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                    SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), width, height,
                    (windowflags | (vid_borderlesswindow ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN)));

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("Staying at the native desktop resolution of %sx%s with an aspect ratio of %s.",
                    temp1, temp2, getaspectratio(width, height));

                free(temp1);
                free(temp2);
            }
        }
        else
        {
            width = screenwidth;
            height = screenheight;

            if (createwindow)
                window = SDL_CreateWindow(DOOMRETRO_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                    SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), width, height,
                    (windowflags | (vid_borderlesswindow ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN)));

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("Switched to a resolution of %sx%s with an aspect ratio of %s.",
                    temp1, temp2, getaspectratio(width, height));

                free(temp1);
                free(temp2);
            }
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

        width = windowwidth;
        height = windowheight;

        if (!windowx && !windowy)
        {
            if (createwindow)
                window = SDL_CreateWindow(DOOMRETRO_NAME, SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex),
                    SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex), width, height, windowflags);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("Created a %sx%s resizable window %s on the screen.",
                    temp1, temp2, (english == english_american ? vid_windowpos_centered : vid_windowpos_centred));

                free(temp1);
                free(temp2);
            }
        }
        else
        {
            if (createwindow)
                window = SDL_CreateWindow(DOOMRETRO_NAME, windowx, windowy, width, height, windowflags);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("Created a %sx%s resizable window at (%i, %i).", temp1, temp2, windowx, windowy);

                free(temp1);
                free(temp2);
            }
        }
    }

    GetUpscaledTextureSize(width, height);

    windowid = SDL_GetWindowID(window);

    SDL_GetWindowSize(window, &displaywidth, &displayheight);
    displaycenterx = displaywidth / 2;
    displaycentery = displayheight / 2;

    if (createwindow && !(renderer = SDL_CreateRenderer(window, -1, rendererflags)) && !software)
    {
        if ((renderer = SDL_CreateRenderer(window, -1, (SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE))))
        {
            C_Warning(1, "The " BOLD("vid_scaleapi") " CVAR was changed from " BOLD("%s") " to " BOLD("\"software\"") ".", vid_scaleapi);
            vid_scaleapi = vid_scaleapi_software;
            M_SaveCVARs();
        }
    }

    SDL_RenderSetLogicalSize(renderer, !vid_widescreen * SCREENWIDTH, ACTUALHEIGHT);

    if (output)
    {
        char    *temp1 = commify(SCREENWIDTH);
        char    *temp2 = commify(SCREENHEIGHT);
        char    *temp3 = commify(width);
        char    *temp4 = commify(height);

        C_Output("A software renderer is used to render every frame.");

        if (nearestlinear)
        {
            char    *temp5 = commify((int64_t)upscaledwidth * SCREENWIDTH);
            char    *temp6 = commify((int64_t)upscaledheight * SCREENHEIGHT);

            C_Output("Every frame is scaled up from %sx%s to %sx%s using nearest-%s interpolation "
                "and then down to %sx%s using linear filtering.",
                temp1, temp2, temp5, temp6, (english == english_american ? "neighbor" : "neighbour"), temp3, temp4);

            free(temp5);
            free(temp6);
        }
        else if (M_StringCompare(vid_scalefilter, vid_scalefilter_linear) && !software)
            C_Output("Every frame is scaled up from %sx%s to %sx%s using linear filtering.",
                temp1, temp2, temp3, temp4);
        else
            C_Output("Every frame is scaled up from %sx%s to %sx%s using nearest-%s interpolation.",
                temp1, temp2, temp3, temp4, (english == english_american ? "neighbor" : "neighbour"));

        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
    }

    if (!SDL_GetRendererInfo(renderer, &rendererinfo))
    {
        if (M_StringCompare(rendererinfo.name, vid_scaleapi_opengl))
        {
            int major;
            int minor;

            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
            SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

            if (major * 10 + minor < 21)
            {
                C_Warning(1, ITALICS(DOOMRETRO_NAME) " requires at least " ITALICS("OpenGL v2.1."));

#if defined(_WIN32)
                vid_scaleapi = vid_scaleapi_direct3d;
                M_SaveCVARs();

                SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, vid_scaleapi, SDL_HINT_OVERRIDE);

                if (output)
                    C_Output("This scaling is now done using hardware acceleration with " ITALICS("Direct3D."));
#endif
            }
            else
            {
                if (output)
                    C_Output("This scaling is done using hardware acceleration with " ITALICS("OpenGL v%i.%i."), major, minor);

                if (!M_StringCompare(vid_scaleapi, vid_scaleapi_opengl))
                {
                    vid_scaleapi = vid_scaleapi_opengl;
                    M_SaveCVARs();
                }
            }
        }
#if defined(_WIN32)
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_direct3d))
        {
            if (output)
                C_Output("This scaling is done using hardware acceleration with " ITALICS("Direct3D."));

            if (!M_StringCompare(vid_scaleapi, vid_scaleapi_direct3d))
            {
                vid_scaleapi = vid_scaleapi_direct3d;
                M_SaveCVARs();
            }
        }
#else
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_opengles))
        {
            if (output)
                C_Output("This scaling is done using hardware acceleration with " ITALICS("OpenGL ES."));
        }
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_opengles2))
        {
            if (output)
                C_Output("This scaling is done using hardware acceleration with " ITALICS("OpenGL ES 2."));
        }
#endif
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_software))
        {
            software = true;
            nearestlinear = false;

            SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE);

            if (output)
                C_Output("This scaling is also done in software.");

            if (!M_StringCompare(vid_scaleapi, vid_scaleapi_software))
            {
                vid_scaleapi = vid_scaleapi_software;
                M_SaveCVARs();
            }

            if (output && (M_StringCompare(vid_scalefilter, vid_scalefilter_linear)
                || M_StringCompare(vid_scalefilter, vid_scalefilter_nearest_linear)))
                C_Warning(1, "Linear filtering can't be used in software.");
        }

        refreshrate = 0;

        if (rendererinfo.flags & SDL_RENDERER_PRESENTVSYNC)
        {
            SDL_DisplayMode displaymode;

            if (!SDL_GetWindowDisplayMode(window, &displaymode))
            {
                refreshrate = displaymode.refresh_rate;

#if !defined (__APPLE__)
                if (vid_vsync == vid_vsync_adaptive && M_StringStartsWith(vid_scaleapi, "opengl"))
                    if (SDL_GL_SetSwapInterval(-1) < 0)
                        C_Warning(1, "Adaptive vsync is not supported.");
#endif
                if (refreshrate < vid_capfps || !vid_capfps)
                {
                    I_CapFPS(0);

                    if (output)
                        C_Output("The framerate is synced with the display's refresh rate of %iHz.", refreshrate);
                }
                else
                {
                    I_CapFPS(vid_capfps);

                    if (output)
                    {
                        char    *temp = commify(vid_capfps);

                        C_Output("The framerate is capped at %s FPS.", temp);
                        free(temp);
                    }
                }
            }
        }
        else
        {
            I_CapFPS(vid_capfps);

            if (output)
            {
                if (vid_vsync)
                {
                    if (M_StringCompare(rendererinfo.name, vid_scaleapi_software))
                        C_Warning(1, "The framerate can't be synced with the display's refresh rate in software.");
                    else
                        C_Warning(1, "The framerate can't be synced with the display's refresh rate using this graphics card.");
                }

                if (vid_capfps)
                {
                    char    *temp = commify(vid_capfps);

                    C_Output("The framerate is capped at %s FPS.", temp);
                    free(temp);
                }
                else
                    C_Output("The framerate is uncapped.");
            }
        }
    }

    if (output)
    {
        wadfile_t   *playpalwad = lumpinfo[W_CheckNumForName("PLAYPAL")]->wadfile;

        if (M_StringCompare(leafname(playpalwad->path), DOOMRETRO_RESOURCEWAD))
            C_Output("Using the 256-%s palette from the " BOLD("PLAYPAL") " lump in the IWAD " BOLD("%s") ".",
                (english == english_american ? "color" : "colour"),
                lumpinfo[W_GetLastNumForName("PLAYPAL")]->wadfile->path);
        else
            C_Output("Using the 256-%s palette from the " BOLD("PLAYPAL") " lump in the %s " BOLD("%s") ".",
                (english == english_american ? "color" : "colour"),
                (playpalwad->type == IWAD ? "IWAD" : "PWAD"),
                playpalwad->path);

        if (gammaindex == 10)
            C_Output("There is no gamma correction.");
        else
        {
            char    text[128];
            int     len;

            M_snprintf(text, sizeof(text), "The gamma correction level is %.2f.", r_gamma);
            len = (int)strlen(text);

            if (text[len - 2] == '0' && text[len - 3] == '0')
            {
                text[len - 2] = '.';
                text[len - 1] = '\0';
            }

            C_Output(text);
        }
    }

    surface = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 8, 0, 0, 0, 0);
    screens[0] = surface->pixels;
    pixelformat = SDL_GetWindowPixelFormat(window);
    SDL_PixelFormatEnumToMasks(pixelformat, &bpp, &rmask, &gmask, &bmask, &amask);
    buffer = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, bpp, rmask, gmask, bmask, amask);

    pitch = buffer->pitch;
    pixels = buffer->pixels;

    SDL_FillRect(buffer, NULL, 0);

    if (nearestlinear)
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE);

    texture = SDL_CreateTexture(renderer, pixelformat, SDL_TEXTUREACCESS_STREAMING, SCREENWIDTH, SCREENHEIGHT);

    if (nearestlinear)
    {
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear, SDL_HINT_OVERRIDE);
        texture_upscaled = SDL_CreateTexture(renderer, pixelformat, SDL_TEXTUREACCESS_TARGET,
            upscaledwidth * SCREENWIDTH, upscaledheight * SCREENHEIGHT);
    }

    palette = SDL_AllocPalette(256);
    SDL_SetSurfacePalette(surface, palette);
    I_SetPalette(&PLAYPAL[st_palette * 768]);

    src_rect.w = SCREENWIDTH;
    src_rect.h = SCREENHEIGHT;
}

static void I_ClearFrame(void)
{
    SDL_RenderClear(renderer);
}

static void I_GetScreenDimensions(void)
{
    int width;
    int height;

    if (vid_fullscreen)
    {
        width = displays[displayindex].w;
        height = displays[displayindex].h;

#if defined(_WIN32)
        clearframefunc = &nullfunc;
#else
        clearframefunc = &I_ClearFrame;
#endif
    }
    else
    {
        GetWindowSize();

        width = windowwidth;
        height = windowheight;

        clearframefunc = &I_ClearFrame;
    }

    if (vid_widescreen)
    {
        SCREENWIDTH = BETWEEN(NONWIDEWIDTH, ((width * ACTUALHEIGHT / height + 1) & ~3), MAXWIDTH);
        WIDESCREENWIDTH = SCREENWIDTH;

        // r_fov * 0.82 is vertical FOV for 4:3 aspect ratio
        WIDEFOVDELTA = (int)(atan(width / (height / tan(r_fov * 0.82 * M_PI / 360.0))) * 360.0 / M_PI) - r_fov - 2;
        WIDESCREENDELTA = ((SCREENWIDTH - NONWIDEWIDTH) / SCREENSCALE) / 2;
        MAXWIDESCREENDELTA = MAX(53, WIDESCREENDELTA);
    }
    else
    {
        SCREENWIDTH = NONWIDEWIDTH;
        WIDESCREENWIDTH = BETWEEN(NONWIDEWIDTH, ((width * ACTUALHEIGHT / height + 1) & ~3), MAXWIDTH);
        WIDEFOVDELTA = 0;
        WIDESCREENDELTA = 0;
        MAXWIDESCREENDELTA = 53;

        clearframefunc = &I_ClearFrame;
    }

    SCREENAREA = SCREENWIDTH * SCREENHEIGHT;

    GetPixelSize();
}

void I_RestartGraphics(bool recreatewindow)
{
    if (recreatewindow)
        SDL_DestroyWindow(window);

    I_GetScreenDimensions();

    SetVideoMode(recreatewindow, false);

    AM_SetAutomapSize(r_screensize);

    if (!mapwindow)
        mapscreen = *screens;

    M_SetWindowCaption();

    C_ResetWrappedLines();

    setsizeneeded = true;

    if (r_playersprites)
        skippsprinterp = true;
}

void I_ToggleFullscreen(void)
{
    if (SDL_SetWindowFullscreen(window,
        (vid_fullscreen ? 0 : (vid_borderlesswindow ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN))) < 0)
    {
        menuactive = false;
        C_ShowConsole();
        C_Warning(0, "Unable to switch to %s.", (vid_fullscreen ? "a window" : "fullscreen"));

        return;
    }

    vid_fullscreen = !vid_fullscreen;
    I_RestartGraphics(vid_fullscreen && !vid_borderlesswindow);
    M_SaveCVARs();

    if (nearestlinear)
        I_UpdateBlitFunc(viewplayer && viewplayer->damagecount);

    if (vid_fullscreen)
        C_StringCVAROutput(stringize(vid_fullscreen), "on");
    else
    {
        C_StringCVAROutput(stringize(vid_fullscreen), "off");

        SDL_SetWindowSize(window, windowwidth, windowheight);

        displaywidth = windowwidth;
        displayheight = windowheight;
        displaycenterx = displaywidth / 2;
        displaycentery = displayheight / 2;

        PositionOnCurrentDisplay();
    }
}

void I_SetPillarboxes(void)
{
    I_SetPalette(&PLAYPAL[st_palette * 768]);

    if (!vid_pillarboxes)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

static void I_InitGammaTables(void)
{
    for (int i = 0; i < GAMMALEVELS; i++)
        for (int j = 0; j < 256; j++)
            gammatable[i][j] = (byte)(pow(j / 255.0, 1.0 / gammalevels[i]) * 255.0 + 0.5);

    for (int r = 0; r < 256; r++)
        for (int g = 0; g < 256; g++)
            for (int b = 0; b < 256; b++)
                saturationtable[r][g][b] = sqrt(r * r * 0.299 + g * g * 0.587 + b * b * 0.114);
}

void I_SetGamma(float value)
{
    for (gammaindex = 0; gammaindex < GAMMALEVELS && gammalevels[gammaindex] != value; gammaindex++);

    if (gammaindex == GAMMALEVELS)
        for (gammaindex = 0; gammalevels[gammaindex] != r_gamma_default; gammaindex++);
}

void I_InitKeyboard(void)
{
    if (keyboardalwaysrun == KEY_CAPSLOCK)
    {
        capslock = GetCapsLockState();

#if defined(_WIN32)
        if (alwaysrun != capslock)
            ToggleCapsLockState();
#elif defined(X11)
        if (alwaysrun && !capslock)
            SetCapsLockState(true);
        else if (!alwaysrun && capslock)
            SetCapsLockState(false);
#endif
    }
}

void I_InitGraphics(void)
{
    SDL_version linked = { 0 };
    SDL_version compiled = { 0 };

    SDL_GetVersion(&linked);
    SDL_VERSION(&compiled);

    if (linked.major != compiled.major || linked.minor != compiled.minor)
        I_Error(DOOMRETRO_NAME " requires SDL v%i.%i.%i.", compiled.major, compiled.minor, compiled.patch);

    if (linked.patch != compiled.patch)
        C_Warning(1, ITALICS(DOOMRETRO_NAME) " requires SDL v%i.%i.%i.", compiled.major, compiled.minor, compiled.patch);

    performancefrequency = SDL_GetPerformanceFrequency();

    for (int i = 0; i < NUMKEYS; i++)
        keys[i] = true;

    keys['v'] = keys['V'] = false;
    keys['s'] = keys['S'] = false;
    keys['i'] = keys['I'] = false;
    keys['r'] = keys['R'] = false;
    keys['a'] = keys['A'] = false;
    keys['l'] = keys['L'] = false;

    PLAYPAL = (harmony ? W_CacheLastLumpName("PLAYPAL") : W_CacheLumpName("PLAYPAL"));
    I_InitTintTables(PLAYPAL);
    FindNearestColors(PLAYPAL);

    I_InitGammaTables();
    I_SetGamma(r_gamma);

#if !defined(_WIN32)
    if (*vid_driver)
        SDL_setenv("SDL_VIDEODRIVER", vid_driver, true);
#endif

    GetDisplays();

#if defined(_DEBUG)
    vid_fullscreen = false;
#endif

    I_GetScreenDimensions();

#if defined(_WIN32)
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1", SDL_HINT_OVERRIDE);
#endif

    if (vid_fullscreen)
        SDL_ShowCursor(false);

    SetVideoMode(true, true);

    I_CreateExternalAutomap();

    if (vid_fullscreen)
        SetShowCursor(false);

#if defined(_WIN32)
    I_InitWindows32();
#endif

    SDL_SetWindowTitle(window, DOOMRETRO_NAME);

    I_UpdateBlitFunc(false);
    memset(screens[0], nearestblack, SCREENAREA);
    blitfunc();

    if (mapwindow)
        mapblitfunc();
}
