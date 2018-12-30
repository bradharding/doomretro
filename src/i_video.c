/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2019 by Brad Harding.

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
#include <mmsystem.h>

#include "SDL_syswm.h"
#elif defined(X11)
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#endif

#include "c_console.h"
#include "d_main.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_gamepad.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "s_sound.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"

#define MAXDISPLAYS         8

#define MAXUPSCALEWIDTH     (1600 / ORIGINALWIDTH)
#define MAXUPSCALEHEIGHT    (1200 / ORIGINALHEIGHT)

#define SHAKEANGLE          (M_RandomInt(-1000, 1000) * r_shake_damage / 100000.0)

#if !defined(SDL_VIDEO_RENDER_D3D11)
#define SDL_VIDEO_RENDER_D3D11  0
#endif

// CVARs
dboolean            m_acceleration = m_acceleration_default;
int                 r_color = r_color_default;
float               r_gamma = r_gamma_default;
int                 vid_capfps = vid_capfps_default;
int                 vid_display = vid_display_default;
#if !defined(_WIN32)
char                *vid_driver = vid_driver_default;
#endif
dboolean            vid_fullscreen = vid_fullscreen_default;
int                 vid_motionblur = vid_motionblur_default;
dboolean            vid_pillarboxes = vid_pillarboxes_default;
char                *vid_scaleapi = vid_scaleapi_default;
char                *vid_scalefilter = vid_scalefilter_default;
char                *vid_screenresolution = vid_screenresolution_default;
dboolean            vid_showfps = vid_showfps_default;
dboolean            vid_vsync = vid_vsync_default;
dboolean            vid_widescreen = vid_widescreen_default;
char                *vid_windowpos = vid_windowpos_default;
char                *vid_windowsize = vid_windowsize_default;

static dboolean     manuallypositioning;

SDL_Window          *window;
static int          windowid;
SDL_Renderer        *renderer;
static SDL_Texture  *texture;
static SDL_Texture  *texture_upscaled;
static SDL_Surface  *surface;
static SDL_Surface  *buffer;
static SDL_Palette  *palette;
static SDL_Color    colors[256];
static byte         *playpal;

byte                *oscreen;
byte                *mapscreen;
SDL_Window          *mapwindow;
SDL_Renderer        *maprenderer;
static SDL_Texture  *maptexture;
static SDL_Texture  *maptexture_upscaled;
static SDL_Surface  *mapsurface;
static SDL_Surface  *mapbuffer;
static SDL_Palette  *mappalette;

static dboolean     nearestlinear;
static int          upscaledwidth;
static int          upscaledheight;

static dboolean     software;

static int          displayindex;
static int          numdisplays;
static SDL_Rect     displays[MAXDISPLAYS];

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

dboolean            returntowidescreen;

dboolean            windowfocused = true;

static dboolean     keys[UCHAR_MAX];

static byte         gammatable[GAMMALEVELS][256];

const float gammalevels[GAMMALEVELS] =
{
    // Darker
    0.50f, 0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f,

    // No gamma correction
    1.0f,

    // Lighter
    1.05f, 1.10f, 1.15f, 1.20f, 1.25f, 1.30f, 1.35f, 1.40f, 1.45f, 1.50f,
    1.55f, 1.60f, 1.65f, 1.70f, 1.75f, 1.80f, 1.85f, 1.90f, 1.95f, 2.0f
};

int                 gammaindex;

static SDL_Rect     src_rect;
static SDL_Rect     map_rect;

int                 fps;
int                 refreshrate;

#if defined(_WIN32)
HANDLE              CapFPSEvent;
#endif

static dboolean     capslock;
dboolean            alwaysrun = alwaysrun_default;

extern dboolean     setsizeneeded;
extern int          st_palette;
extern int          windowborderwidth;
extern int          windowborderheight;

evtype_t            lasteventtype;

void ST_doRefresh(void);

dboolean MouseShouldBeGrabbed(void)
{
    // if the window doesn't have focus, never grab it
    if (!windowfocused)
        return false;

    // always grab the mouse when full screen (don't want to see the mouse pointer)
    if (vid_fullscreen)
        return true;

    // when menu is active or game is paused, release the mouse
    if (menuactive || consoleactive || paused)
        return false;

    // only grab mouse when playing levels
    return (gamestate == GS_LEVEL);
}

static void SetShowCursor(dboolean show)
{
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
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_CTRL, KEY_SHIFT, KEY_ALT, 0, KEY_CTRL, KEY_SHIFT, KEY_ALT, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int TranslateKey2(int key)
{
    switch (key)
    {
        case KEY_LEFTARROW:     return SDL_SCANCODE_LEFT;
        case KEY_RIGHTARROW:    return SDL_SCANCODE_RIGHT;
        case KEY_DOWNARROW:     return SDL_SCANCODE_DOWN;
        case KEY_UPARROW:       return SDL_SCANCODE_UP;
        case KEY_ESCAPE:        return SDL_SCANCODE_ESCAPE;
        case KEY_ENTER:         return SDL_SCANCODE_RETURN;
        case KEY_TAB:           return SDL_SCANCODE_TAB;
        case KEY_F1:            return SDL_SCANCODE_F1;
        case KEY_F2:            return SDL_SCANCODE_F2;
        case KEY_F3:            return SDL_SCANCODE_F3;
        case KEY_F4:            return SDL_SCANCODE_F4;
        case KEY_F5:            return SDL_SCANCODE_F5;
        case KEY_F6:            return SDL_SCANCODE_F6;
        case KEY_F7:            return SDL_SCANCODE_F7;
        case KEY_F8:            return SDL_SCANCODE_F8;
        case KEY_F9:            return SDL_SCANCODE_F9;
        case KEY_F10:           return SDL_SCANCODE_F10;
        case KEY_F11:           return SDL_SCANCODE_F11;
        case KEY_F12:           return SDL_SCANCODE_F12;
        case KEY_BACKSPACE:     return SDL_SCANCODE_BACKSPACE;
        case KEY_DELETE:        return SDL_SCANCODE_DELETE;
        case KEY_PAUSE:         return SDL_SCANCODE_PAUSE;
        case KEY_EQUALS:        return SDL_SCANCODE_EQUALS;
        case KEY_MINUS:         return SDL_SCANCODE_MINUS;
        case KEY_SHIFT:         return SDL_SCANCODE_RSHIFT;
        case KEY_CTRL:          return SDL_SCANCODE_RCTRL;
        case KEY_ALT:           return SDL_SCANCODE_RALT;
        case KEY_CAPSLOCK:      return SDL_SCANCODE_CAPSLOCK;
        case KEY_SCROLLLOCK:    return SDL_SCANCODE_SCROLLLOCK;
        case KEYP_0:            return SDL_SCANCODE_KP_0;
        case KEYP_1:            return SDL_SCANCODE_KP_1;
        case KEYP_3:            return SDL_SCANCODE_KP_3;
        case KEYP_5:            return SDL_SCANCODE_KP_5;
        case KEYP_7:            return SDL_SCANCODE_KP_7;
        case KEYP_9:            return SDL_SCANCODE_KP_9;
        case KEYP_PERIOD:       return SDL_SCANCODE_KP_PERIOD;
        case KEYP_MULTIPLY:     return SDL_SCANCODE_KP_MULTIPLY;
        case KEYP_DIVIDE:       return SDL_SCANCODE_KP_DIVIDE;
        case KEY_INSERT:        return SDL_SCANCODE_INSERT;
        case KEY_NUMLOCK:       return SDL_SCANCODE_NUMLOCKCLEAR;
        default:                return key;
    }
}

dboolean keystate(int key)
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    return keystate[TranslateKey2(key)];
}

void I_CapFPS(int fps)
{
#if defined(_WIN32)
    static UINT CapFPSTimer;

    if (CapFPSTimer)
    {
        timeKillEvent(CapFPSTimer);
        CapFPSTimer = 0;
    }

    if (!fps || fps == TICRATE)
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
            CapFPSTimer = timeSetEvent(1000 / fps, 0, (LPTIMECALLBACK)CapFPSEvent, 0, (TIME_PERIODIC | TIME_CALLBACK_EVENT_SET));

            if (!CapFPSTimer)
            {
                CloseHandle(CapFPSEvent);
                CapFPSEvent = NULL;
            }
        }
    }
#endif
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
    window = NULL;

    if (mapwindow)
        I_DestroyExternalAutomap();
}

void I_ShutdownGraphics(void)
{
    I_CapFPS(0);
    FreeSurfaces();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    free(oscreen);
    oscreen = NULL;
}

#if defined(_WIN32)
static void ToggleCapsLockState(void)
{
    keybd_event(VK_CAPITAL, 0x45, 0, (uintptr_t)0);
    keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_KEYUP, (uintptr_t)0);
}
#elif defined(X11)
static void SetCapsLockState(dboolean enabled)
{
    Display *dpy = XOpenDisplay(0);

    XkbLockModifiers(dpy, XkbUseCoreKbd, 2, enabled * 2);
    XFlush(dpy);
    XCloseDisplay(dpy);
}
#endif

dboolean GetCapsLockState(void)
{
#if defined(_WIN32)
    return !!(GetKeyState(VK_CAPITAL) & 0xFFFF);
#else
    return !!(SDL_GetModState() & KMOD_CAPS);
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
    if (value < 0)
        return -AccelerateMouse(-value);

    if (value > 10)
        return (value * 2 - 10);
    else
        return value;
}

static short __inline clamp(short value, short deadzone)
{
    return (ABS(value) < deadzone ? 0 : (gp_analog ? MAX(-SHRT_MAX, value) : SIGN(value) * SHRT_MAX));
}

dboolean        altdown;
dboolean        noinput = true;
dboolean        waspaused;
static dboolean button;

static void I_GetEvent(void)
{
    SDL_Event   SDLEvent;
    SDL_Event   *Event = &SDLEvent;

    SDL_PumpEvents();

    while (SDL_PollEvent(Event))
    {
        event_t         event;

#if !defined(_WIN32)
        static dboolean enterdown;
#endif

        switch (Event->type)
        {
            case SDL_TEXTINPUT:
                for (int i = 0, len = (int)strlen(Event->text.text); i < len; i++)
                {
                    const char  ch = Event->text.text[i];

                    if (isprint(ch))
                    {
                        event_t textevent = { ev_text, ch, 0, 0 };

                        D_PostEvent(&textevent);
                    }
                }

                break;

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
                    if (altdown)
                    {
                        if (event.data1 == KEY_F4)
                        {
                            I_Sleep(300);
                            I_Quit(true);
                        }
                        else if (event.data1 == KEY_TAB)
                        {
                            event.data1 = 0;
                            event.data2 = 0;
                        }
                    }

                    if (!isdigit(event.data2))
                    {
                        idclev = false;
                        idmus = false;
                    }

                    if (idbehold && keys[event.data2])
                    {
                        idbehold = false;
                        HU_ClearMessages();
                    }

#if !defined(_WIN32)
                    // Handle ALT+ENTER on non-Windows systems
                    if (altdown && event.data1 == KEY_ENTER && !enterdown)
                    {
                        enterdown = true;
                        I_ToggleFullscreen();
                        return;
                    }
#endif

                    D_PostEvent(&event);
                }

                break;

            case SDL_KEYUP:
                event.type = ev_keyup;
                event.data1 = translatekey[Event->key.keysym.scancode];
                altdown = (Event->key.keysym.mod & KMOD_ALT);
                keydown = 0;

#if !defined(_WIN32)
                // Handle ALT+ENTER on non-Windows systems
                if (event.data1 == KEY_ENTER)
                    enterdown = false;
#endif

                if (event.data1)
                    D_PostEvent(&event);

                break;

            case SDL_MOUSEBUTTONDOWN:
                if (noinput)
                    return;

                idclev = false;
                idmus = false;

                if (idbehold)
                {
                    HU_ClearMessages();
                    idbehold = false;
                }

                mousebuttonstate |= buttons[Event->button.button];
                button = true;
                break;

            case SDL_MOUSEBUTTONUP:
                keydown = 0;
                mousebuttonstate &= ~buttons[Event->button.button];
                button = true;
                break;

            case SDL_MOUSEWHEEL:
                keydown = 0;
                event.type = ev_mousewheel;
                event.data1 = Event->wheel.y;
                event.data2 = 0;
                event.data3 = 0;
                D_PostEvent(&event);
                break;

            case SDL_CONTROLLERAXISMOTION:
                switch (Event->caxis.axis)
                {
                    case SDL_CONTROLLER_AXIS_LEFTX:
                        if (gp_swapthumbsticks)
                            gamepadthumbRX = clamp(Event->caxis.value, gamepadrightdeadzone);
                        else
                            gamepadthumbLX = clamp(Event->caxis.value, gamepadleftdeadzone);

                        break;

                    case SDL_CONTROLLER_AXIS_LEFTY:
                        if (gp_swapthumbsticks)
                            gamepadthumbRY = clamp(Event->caxis.value, gamepadrightdeadzone);
                        else
                            gamepadthumbLY = clamp(Event->caxis.value, gamepadleftdeadzone);

                        break;

                    case SDL_CONTROLLER_AXIS_RIGHTX:
                        if (gp_swapthumbsticks)
                            gamepadthumbLX = clamp(Event->caxis.value, gamepadleftdeadzone);
                        else
                            gamepadthumbRX = clamp(Event->caxis.value, gamepadrightdeadzone);

                        break;

                    case SDL_CONTROLLER_AXIS_RIGHTY:
                        if (gp_swapthumbsticks)
                            gamepadthumbLY = clamp(Event->caxis.value, gamepadleftdeadzone);
                        else
                            gamepadthumbRY = clamp(Event->caxis.value, gamepadrightdeadzone);

                        break;

                    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                        if (Event->caxis.value >= GAMEPAD_TRIGGER_THRESHOLD)
                            gamepadbuttons |= GAMEPAD_LEFT_TRIGGER;
                        else
                            gamepadbuttons &= ~GAMEPAD_LEFT_TRIGGER;

                        break;

                    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                        if (Event->caxis.value >= GAMEPAD_TRIGGER_THRESHOLD)
                            gamepadbuttons |= GAMEPAD_RIGHT_TRIGGER;
                        else
                            gamepadbuttons &= ~GAMEPAD_RIGHT_TRIGGER;

                        break;
                }

                lasteventtype = ev_gamepad;
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                gamepadbuttons |= (1 << Event->cbutton.button);
                event.type = ev_gamepad;
                D_PostEvent(&event);
                break;

            case SDL_CONTROLLERBUTTONUP:
                gamepadbuttons &= ~(1 << Event->cbutton.button);
                keydown = 0;
                event.type = ev_gamepad;
                D_PostEvent(&event);
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
                if (Event->window.windowID == windowid)
                {
                    switch (Event->window.event)
                    {
                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                            windowfocused = true;

                            if (menuactive || consoleactive)
                                S_ResumeSound();

                            I_InitKeyboard();

                            break;

                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            windowfocused = false;

                            if (gamestate == GS_LEVEL && !paused)
                            {
                                if (menuactive || consoleactive)
                                    S_PauseSound();
                                else
                                    sendpause = true;
                            }

                            I_ShutdownKeyboard();

                            break;

                        case SDL_WINDOWEVENT_EXPOSED:
                            SDL_SetPaletteColors(palette, colors, 0, 256);
                            break;

                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            if (!vid_fullscreen)
                            {
                                char    size[16];

                                windowwidth = Event->window.data1;
                                windowheight = Event->window.data2;
                                M_snprintf(size, sizeof(size), "%sx%s", commify(windowwidth), commify(windowheight));
                                vid_windowsize = strdup(size);
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
                                char    pos[16];

                                windowx = Event->window.data1;
                                windowy = Event->window.data2;
                                M_snprintf(pos, sizeof(pos), "(%i,%i)", windowx, windowy);
                                vid_windowpos = strdup(pos);
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

static void I_ReadMouse(void)
{
    int x, y;

    if (startingnewgame)
        SDL_GetRelativeMouseState(NULL, NULL);

    SDL_GetRelativeMouseState(&x, &y);

    if (x || y || button)
    {
        event_t ev;

        ev.type = ev_mouse;
        ev.data1 = mousebuttonstate;

        if (m_acceleration)
        {
            ev.data2 = AccelerateMouse(x);
            ev.data3 = -AccelerateMouse(y);
        }
        else
        {
            ev.data2 = x;
            ev.data3 = -y;
        }

        D_PostEvent(&ev);
        button = false;
    }
}

//
// I_StartTic
//
void I_StartTic(void)
{
    I_GetEvent();
    I_ReadMouse();
    I_UpdateGamepadVibration();
}

static void UpdateGrab(void)
{
    dboolean        grab = MouseShouldBeGrabbed();
    static dboolean currently_grabbed;

    if (grab && !currently_grabbed)
        SetShowCursor(false);
    else if (!grab && currently_grabbed)
    {
        SetShowCursor(true);
        SDL_WarpMouseInWindow(window, windowwidth - 10 * windowwidth / SCREENWIDTH, windowheight - 16);
        SDL_GetRelativeMouseState(NULL, NULL);
    }

    currently_grabbed = grab;
}

static void GetUpscaledTextureSize(int width, int height)
{
    const int   actualheight = SCREENWIDTH * 3 / 4;

    if (width * actualheight < height * SCREENWIDTH)
        height = width * actualheight / SCREENWIDTH;
    else
        width = height * SCREENWIDTH / actualheight;

    upscaledwidth = MIN(width / SCREENWIDTH + !!(width % SCREENWIDTH), MAXUPSCALEWIDTH);
    upscaledheight = MIN(height / SCREENHEIGHT + !!(height % SCREENHEIGHT), MAXUPSCALEHEIGHT);
}

void (*blitfunc)(void);
void (*mapblitfunc)(void);

void nullfunc(void) {}

static uint64_t performancefrequency;
uint64_t        starttime;
int             frames = -1;

static void CalculateFPS(void)
{
    uint64_t    currenttime = SDL_GetPerformanceCounter();

    frames++;

    if (starttime < currenttime - performancefrequency)
    {
        fps = frames;
        frames = 0;
        starttime = currenttime;
    }

    C_UpdateFPS();
}

#if defined(_WIN32)
void I_WindowResizeBlit(void)
{
    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
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
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(renderer);
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
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_ShowFPS(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(renderer);
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
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_Shake(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
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
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_ShowFPS_Shake(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, buffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void I_Blit_Automap(void)
{
    SDL_LowerBlit(mapsurface, &map_rect, mapbuffer, &map_rect);
    SDL_UpdateTexture(maptexture, &map_rect, mapbuffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(maprenderer);
    SDL_RenderCopy(maprenderer, maptexture, &map_rect, NULL);
    SDL_RenderPresent(maprenderer);
}

void I_Blit_Automap_NearestLinear(void)
{
    SDL_LowerBlit(mapsurface, &map_rect, mapbuffer, &map_rect);
    SDL_UpdateTexture(maptexture, &map_rect, mapbuffer->pixels, SCREENWIDTH * 4);
    SDL_RenderClear(maprenderer);
    SDL_SetRenderTarget(maprenderer, maptexture_upscaled);
    SDL_RenderCopy(maprenderer, maptexture, &map_rect, NULL);
    SDL_SetRenderTarget(maprenderer, NULL);
    SDL_RenderCopy(maprenderer, maptexture_upscaled, NULL, NULL);
    SDL_RenderPresent(maprenderer);
}

void I_UpdateBlitFunc(dboolean shake)
{
    dboolean    override = (vid_fullscreen && !(displayheight % ORIGINALHEIGHT));

    if (shake && !software)
        blitfunc = (vid_showfps ? (nearestlinear && !override ? I_Blit_NearestLinear_ShowFPS_Shake :
            I_Blit_ShowFPS_Shake) : (nearestlinear && !override ? I_Blit_NearestLinear_Shake : I_Blit_Shake));
    else
        blitfunc = (vid_showfps ? (nearestlinear && !override ? I_Blit_NearestLinear_ShowFPS : I_Blit_ShowFPS) :
            (nearestlinear && !override ? I_Blit_NearestLinear : I_Blit));

    mapblitfunc = (mapwindow ? (nearestlinear && !override ? I_Blit_Automap_NearestLinear : I_Blit_Automap) : nullfunc);
}

//
// I_SetPalette
//
void I_SetPalette(byte *playpal)
{
    if (r_color == r_color_max)
    {
        for (int i = 0; i < 256; i++)
        {
            colors[i].r = gammatable[gammaindex][*playpal++];
            colors[i].g = gammatable[gammaindex][*playpal++];
            colors[i].b = gammatable[gammaindex][*playpal++];
        }
    }
    else
    {
        double  color = r_color / 100.0;

        for (int i = 0; i < 256; i++)
        {
            byte    r = gammatable[gammaindex][*playpal++];
            byte    g = gammatable[gammaindex][*playpal++];
            byte    b = gammatable[gammaindex][*playpal++];
            double  p = sqrt(r * r * 0.299 + g * g * 0.587 + b * b * 0.114);

            colors[i].r = (byte)(p + (r - p) * color);
            colors[i].g = (byte)(p + (g - p) * color);
            colors[i].b = (byte)(p + (b - p) * color);
        }
    }

    SDL_SetPaletteColors(palette, colors, 0, 256);

    if (vid_pillarboxes)
        SDL_SetRenderDrawColor(renderer, colors[0].r, colors[0].g, colors[0].b, SDL_ALPHA_OPAQUE);
}

void I_SetSimplePalette(byte *playpal)
{
    for (int i = 0; i < 256; i++)
    {
        colors[i].r = *playpal++;
        colors[i].g = *playpal++;
        colors[i].b = *playpal++;
    }

    SDL_SetPaletteColors(palette, colors, 0, 256);
}

static void I_RestoreFocus(void)
{
#if defined(_WIN32)
    SDL_SysWMinfo   info;

    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);
    SetFocus(info.info.win.window);
#endif
}

static void GetDisplays(void)
{
    numdisplays = MIN(SDL_GetNumVideoDisplays(), MAXDISPLAYS);

    for (int i = 0; i < numdisplays; i++)
        SDL_GetDisplayBounds(i, &displays[i]);
}

void I_CreateExternalAutomap(dboolean output)
{
    Uint32      rmask, gmask, bmask, amask;
    int         bpp;
    int         flags = SDL_RENDERER_TARGETTEXTURE;
    static int  am_displayindex;

    mapscreen = *screens;
    mapblitfunc = nullfunc;

    if (!am_external)
        return;

    GetDisplays();

    if (numdisplays == 1)
    {
        if (output)
            C_Warning("Only one display was found. An external automap couldn't be created.");

        return;
    }

    am_displayindex = !displayindex;

    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0", SDL_HINT_OVERRIDE);

    mapwindow = SDL_CreateWindow("Automap", SDL_WINDOWPOS_UNDEFINED_DISPLAY(am_displayindex),
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(am_displayindex), 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (vid_vsync)
        flags |= SDL_RENDERER_PRESENTVSYNC;

    maprenderer = SDL_CreateRenderer(mapwindow, -1, flags);
    SDL_RenderSetLogicalSize(maprenderer, SCREENWIDTH, SCREENHEIGHT);
    mapsurface = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 8, 0, 0, 0, 0);

    if (SDL_PixelFormatEnumToMasks(SDL_GetWindowPixelFormat(mapwindow), &bpp, &rmask, &gmask, &bmask, &amask))
        mapbuffer = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 32, rmask, gmask, bmask, amask);
    else
        mapbuffer = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 32, 0, 0, 0, 0);

    SDL_FillRect(mapbuffer, NULL, 0);

    maptexture = SDL_CreateTexture(maprenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREENWIDTH, SCREENHEIGHT);

    if (nearestlinear)
    {
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear, SDL_HINT_OVERRIDE);

        maptexture_upscaled = SDL_CreateTexture(maprenderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_TARGET, upscaledwidth * SCREENWIDTH, upscaledheight * SCREENHEIGHT);

        mapblitfunc = I_Blit_Automap_NearestLinear;
    }
    else
        mapblitfunc = I_Blit_Automap;

    mappalette = SDL_AllocPalette(256);
    SDL_SetSurfacePalette(mapsurface, mappalette);
    SDL_SetPaletteColors(mappalette, colors, 0, 256);

    mapscreen = mapsurface->pixels;
    map_rect.w = SCREENWIDTH;
    map_rect.h = SCREENHEIGHT - SBARHEIGHT;

    I_RestoreFocus();

    if (output)
    {
        const char  *displayname = SDL_GetDisplayName(am_displayindex);

        if (*displayname)
            C_Output("Created an external automap on display %i called \"%s\".", am_displayindex + 1, displayname);
        else
            C_Output("Created an external automap on display %i.", am_displayindex + 1);
    }
}

void I_DestroyExternalAutomap(void)
{
    SDL_FreePalette(mappalette);
    SDL_FreeSurface(mapsurface);
    SDL_FreeSurface(mapbuffer);
    SDL_DestroyTexture(maptexture);
    SDL_DestroyTexture(maptexture_upscaled);
    SDL_DestroyRenderer(maprenderer);
    SDL_DestroyWindow(mapwindow);
    mapwindow = NULL;
    mapscreen = NULL;
    mapblitfunc = nullfunc;
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
    else if (sscanf(vid_windowpos, "(%10d,%10d)", &x, &y) != 2)
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
    char    *width = malloc(11);
    char    *height = malloc(11);

    if (sscanf(vid_windowsize, "%10[^x]x%10[^x]", width, height) != 2)
    {
        windowheight = SCREENHEIGHT + windowborderheight;
        windowwidth = SCREENHEIGHT * 16 / 10 + windowborderwidth;
        vid_windowsize = vid_windowsize_default;
        M_SaveCVARs();
    }
    else
    {
        char    *width_str = uncommify(width);
        char    *height_str = uncommify(height);
        int     w = atoi(width_str);
        int     h = atoi(height_str);

        if (w < ORIGINALWIDTH + windowborderwidth || h < ORIGINALWIDTH * 3 / 4 + windowborderheight)
        {
            char    size[16];
            char    *windowwidth_str = commify((windowwidth = ORIGINALWIDTH + windowborderwidth));
            char    *windowheight_str = commify((windowheight = ORIGINALWIDTH * 3 / 4 + windowborderheight));

            M_snprintf(size, sizeof(size), "%sx%s", windowwidth_str, windowheight_str);
            vid_windowsize = strdup(size);
            M_SaveCVARs();

            free(windowwidth_str);
            free(windowheight_str);
        }
        else
        {
            windowwidth = w;
            windowheight = h;
        }

        free(width_str);
        free(height_str);
    }

    free(width);
    free(height);
}

static dboolean ValidScreenMode(int width, int height)
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
        int width = -1;
        int height = -1;

        if (sscanf(vid_screenresolution, "%10dx%10d", &width, &height) != 2 || !ValidScreenMode(width, height))
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

    if (!windowx && !windowy)
        SDL_SetWindowPosition(window, displays[displayindex].x + (displays[displayindex].w - windowwidth) / 2,
            displays[displayindex].y + (displays[displayindex].h - windowheight) / 2);
    else
        SDL_SetWindowPosition(window, windowx, windowy);
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

static void SetVideoMode(dboolean output)
{
    int                 rendererflags = SDL_RENDERER_TARGETTEXTURE;
    int                 windowflags = SDL_WINDOW_RESIZABLE;
    int                 width, height;
    Uint32              rmask, gmask, bmask, amask;
    int                 bpp;
    SDL_RendererInfo    rendererinfo;

    displayindex = vid_display - 1;

    if (displayindex < 0 || displayindex >= numdisplays)
    {
        if (output)
            C_Warning("Unable to find display %i.", vid_display);

        displayindex = vid_display_default - 1;
    }

    if (output)
    {
        const char  *displayname = SDL_GetDisplayName(displayindex);

        if (*displayname)
            C_Output("Using display %i of %i called \"%s\".", displayindex + 1, numdisplays, displayname);
        else
            C_Output("Using display %i of %i.", displayindex + 1, numdisplays);
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

    if (M_StringCompare(vid_scaleapi, vid_scaleapi_opengl))
        windowflags |= SDL_WINDOW_OPENGL;

    if (vid_fullscreen)
    {
        char    *ratio;

        if (!screenwidth && !screenheight)
        {
            width = displays[displayindex].w;
            height = displays[displayindex].h;
            ratio = getaspectratio(width, height);

            window = SDL_CreateWindow(PACKAGE_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), width, height, (windowflags | SDL_WINDOW_FULLSCREEN));

            if (output)
            {
                char    *width_str = commify(width);
                char    *height_str = commify(height);

                C_Output("Staying at the native desktop resolution of %s\xD7%s with a %s aspect ratio.", width_str, height_str, ratio);

                free(width_str);
                free(height_str);
            }
        }
        else
        {
            width = screenwidth;
            height = screenheight;
            ratio = getaspectratio(width, height);

            window = SDL_CreateWindow(PACKAGE_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), width, height, (windowflags | SDL_WINDOW_FULLSCREEN));

            if (output)
            {
                char    *width_str = commify(width);
                char    *height_str = commify(height);

                C_Output("Switched to a resolution of %s\xD7%s with a %s aspect ratio.", width_str, height_str, ratio);

                free(width_str);
                free(height_str);
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
            window = SDL_CreateWindow(PACKAGE_NAME, SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex),
                SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex), width, height, windowflags);

            if (output)
            {
                char    *width_str = commify(width);
                char    *height_str = commify(height);

                C_Output("Created a resizable window with dimensions %s\xD7%s centered on the screen.", width_str, height_str);

                free(width_str);
                free(height_str);
            }
        }
        else
        {
            window = SDL_CreateWindow(PACKAGE_NAME, windowx, windowy, width, height, windowflags);

            if (output)
            {
                char    *width_str = commify(width);
                char    *height_str = commify(height);

                C_Output("Created a resizable window with dimensions %s\xD7%s at (%i,%i).", width_str, height_str, windowx, windowy);

                free(width_str);
                free(height_str);
            }
        }
    }

    GetUpscaledTextureSize(width, height);

    windowid = SDL_GetWindowID(window);

    SDL_GetWindowSize(window, &displaywidth, &displayheight);
    displaycenterx = displaywidth / 2;
    displaycentery = displayheight / 2;

    renderer = SDL_CreateRenderer(window, -1, rendererflags);
    SDL_RenderSetLogicalSize(renderer, SCREENWIDTH, SCREENWIDTH * 3 / 4);

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
                C_Warning("<i>"PACKAGE_NAME"</i> requires at least <i>OpenGL 2.1</i>.");

                vid_scaleapi = vid_scaleapi_direct3d;
                M_SaveCVARs();
                SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, vid_scaleapi, SDL_HINT_OVERRIDE);

                if (output)
                    C_Output("The screen is now rendered using hardware acceleration with the "
                        "<i><b>Direct3D %s</b></i> API instead.", (SDL_VIDEO_RENDER_D3D11 ? "11.0" : "9.0"));
            }
            else
            {
                if (output)
                    C_Output("The screen is rendered using hardware acceleration with the <i><b>OpenGL %i.%i</b></i> API.",
                        major, minor);

                if (!M_StringCompare(vid_scaleapi, vid_scaleapi_opengl))
                {
                    vid_scaleapi = vid_scaleapi_opengl;
                    M_SaveCVARs();
                }
            }
        }
#if !defined(_WIN32)
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_opengles))
        {
            if (output)
                C_Output("The screen is rendered using hardware acceleration with the <i><b>OpenGL ES</b></i> API.");
        }
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_opengles2))
        {
            if (output)
                C_Output("The screen is rendered using hardware acceleration with the <i><b>OpenGL ES 2</b></i> API.");
        }
#elif defined(__MACOSX__)
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_metal))
        {
            if (output)
                C_Output("The screen is rendered using hardware acceleration with the <i><b>Metal</b></i> API.");
        }
#endif
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_direct3d))
        {
            if (output)
                C_Output("The screen is rendered using hardware acceleration with the <i><b>Direct3D %s</b></i> API.",
                    (SDL_VIDEO_RENDER_D3D11 ? "11.0" : "9.0"));

            if (!M_StringCompare(vid_scaleapi, vid_scaleapi_direct3d))
            {
                vid_scaleapi = vid_scaleapi_direct3d;
                M_SaveCVARs();
            }
        }
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_software))
        {
            software = true;
            nearestlinear = false;
            SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE);

            if (output)
                C_Output("The screen is rendered in software.");

            if (!M_StringCompare(vid_scaleapi, vid_scaleapi_software))
            {
                vid_scaleapi = vid_scaleapi_software;
                M_SaveCVARs();
            }

            if (output && (M_StringCompare(vid_scalefilter, vid_scalefilter_linear)
                || M_StringCompare(vid_scalefilter, vid_scalefilter_nearest_linear)))
                C_Warning("Linear filtering can't be used in software.");
        }

        if (output)
        {
            if (nearestlinear)
            {
                char    *upscaledwidth_str = commify(upscaledwidth * SCREENWIDTH);
                char    *upscaledheight_str = commify(upscaledheight * SCREENHEIGHT);
                char    *width_str = commify(height * 4 / 3);
                char    *height_str = commify(height);

                C_Output("The %i\xD7%i screen is scaled up to %s\xD7%s using nearest-neighbor interpolation.",
                    SCREENWIDTH, SCREENHEIGHT, upscaledwidth_str, upscaledheight_str);
                C_Output("It is then scaled down to %s\xD7%s using linear filtering.", width_str, height_str);

                free(upscaledwidth_str);
                free(upscaledheight_str);
                free(width_str);
                free(height_str);
            }
            else if (M_StringCompare(vid_scalefilter, vid_scalefilter_linear) && !software)
            {
                char    *width_str = commify(height * 4 / 3);
                char    *height_str = commify(height);

                C_Output("The %i\xD7%i screen is scaled up to %s\xD7%s using linear filtering.",
                    SCREENWIDTH, SCREENHEIGHT, width_str, height_str);

                free(width_str);
                free(height_str);
            }
            else
            {
                char    *width_str = commify(height * 4 / 3);
                char    *height_str = commify(height);

                C_Output("The %i\xD7%i screen is scaled up to %s\xD7%s using nearest-neighbor interpolation.",
                    SCREENWIDTH, SCREENHEIGHT, width_str, height_str);

                free(width_str);
                free(height_str);
            }
        }

        I_CapFPS(0);

        refreshrate = 0;

        if (rendererinfo.flags & SDL_RENDERER_PRESENTVSYNC)
        {
            SDL_DisplayMode displaymode;

            if (!SDL_GetWindowDisplayMode(window, &displaymode))
            {
                refreshrate = displaymode.refresh_rate;

                if (refreshrate < vid_capfps || !vid_capfps)
                {
                    if (output)
                        C_Output("The framerate is synced to the display's refresh rate of %iHz.", refreshrate);
                }
                else
                {
                    I_CapFPS(vid_capfps);

                    if (output)
                    {
                        char    *vid_capfps_str = commify(vid_capfps);

                        C_Output("The framerate is capped at %s FPS.", vid_capfps_str);

                        free(vid_capfps_str);
                    }
                }
            }
        }
        else
        {
            if (vid_capfps)
                I_CapFPS(vid_capfps);

            if (output)
            {
                if (vid_vsync)
                {
                    if (M_StringCompare(rendererinfo.name, vid_scaleapi_software))
                        C_Warning("Vertical sync can't be enabled in software.");
                    else
                        C_Warning("Vertical sync can't be enabled on this video card.");
                }

                if (vid_capfps)
                {
                    char    *vid_capfps_str = commify(vid_capfps);

                    C_Output("The framerate is capped at %s FPS.", vid_capfps_str);

                    free(vid_capfps_str);
                }
                else
                    C_Output("The framerate is uncapped.");
            }
        }
    }

    if (output)
    {
        wadfile_t   *playpalwad = lumpinfo[W_CheckNumForName("PLAYPAL")]->wadfile;

        C_Output("Using the 256-color palette from the <b>PLAYPAL</b> lump in %s <b>%s</b>.",
            (playpalwad->type == IWAD ? "IWAD" : "PWAD"), playpalwad->path);

        if (gammaindex == 10)
            C_Output("Gamma correction is off.");
        else
        {
            static char text[128];
            int         len;

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

    if (SDL_PixelFormatEnumToMasks(SDL_GetWindowPixelFormat(window), &bpp, &rmask, &gmask, &bmask, &amask))
        buffer = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 32, rmask, gmask, bmask, amask);
    else
        buffer = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 32, 0, 0, 0, 0);

    SDL_FillRect(buffer, NULL, 0);

    if (nearestlinear)
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREENWIDTH, SCREENHEIGHT);

    if (nearestlinear)
    {
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear, SDL_HINT_OVERRIDE);
        texture_upscaled = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, upscaledwidth * SCREENWIDTH,
            upscaledheight * SCREENHEIGHT);
    }

    palette = SDL_AllocPalette(256);
    SDL_SetSurfacePalette(surface, palette);
    I_SetPalette(playpal + st_palette * 768);

    src_rect.w = SCREENWIDTH;
    src_rect.h = SCREENHEIGHT - SBARHEIGHT * vid_widescreen;
}

void I_ToggleWidescreen(dboolean toggle)
{
    if (toggle)
    {
        vid_widescreen = true;
        SDL_RenderSetLogicalSize(renderer, 0, 0);
        SDL_RenderSetLogicalSize(renderer, SCREENWIDTH, SCREENWIDTH * 10 / 16);
        src_rect.h = SCREENHEIGHT - SBARHEIGHT;
    }
    else
    {
        vid_widescreen = false;

        if (gamestate == GS_LEVEL)
            ST_doRefresh();

        SDL_RenderSetLogicalSize(renderer, 0, 0);
        SDL_RenderSetLogicalSize(renderer, SCREENWIDTH, SCREENWIDTH * 3 / 4);
        src_rect.h = SCREENHEIGHT;
    }

    returntowidescreen = false;
    setsizeneeded = true;

    HU_InitMessages();

    SDL_SetPaletteColors(palette, colors, 0, 256);
}

#if defined(_WIN32)
void I_InitWindows32(void);
#endif

void I_RestartGraphics(void)
{
    FreeSurfaces();
    SetVideoMode(false);

    if (vid_widescreen)
        I_ToggleWidescreen(true);

    I_CreateExternalAutomap(false);

#if defined(_WIN32)
    I_InitWindows32();
#endif

    M_SetWindowCaption();

    forceconsoleblurredraw = true;
}

void I_ToggleFullscreen(void)
{
    if (SDL_SetWindowFullscreen(window, (vid_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP)) < 0)
    {
        menuactive = false;
        C_ShowConsole();
        C_Warning("Unable to switch to %s.", (!vid_fullscreen ? "fullscreen" : "a window"));
        return;
    }

    vid_fullscreen = !vid_fullscreen;
    M_SaveCVARs();

    if (nearestlinear)
        I_UpdateBlitFunc(viewplayer && !!viewplayer->damagecount);

    if (vid_fullscreen)
        C_StrCVAROutput(stringize(vid_fullscreen), "on");
    else
    {
        C_StrCVAROutput(stringize(vid_fullscreen), "off");

        SDL_SetWindowSize(window, windowwidth, windowheight);

        displaywidth = windowwidth;
        displayheight = windowheight;
        displaycenterx = displaywidth / 2;
        displaycentery = displayheight / 2;

        PositionOnCurrentDisplay();

        if (menuactive || consoleactive || paused || gamestate != GS_LEVEL)
            SDL_WarpMouseInWindow(window, windowwidth - 10 * windowwidth / SCREENWIDTH, windowheight - 16);
    }
}

void I_SetPillarboxes(void)
{
    I_SetPalette(playpal + st_palette * 768);

    if (!vid_pillarboxes)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

static void I_InitGammaTables(void)
{
    for (int i = 0; i < GAMMALEVELS; i++)
        for (int j = 0; j < 256; j++)
            gammatable[i][j] = (byte)(pow(j / 255.0, 1.0 / gammalevels[i]) * 255.0 + 0.5);
}

void I_SetGamma(float value)
{
    gammaindex = 0;

    while (gammaindex < GAMMALEVELS && gammalevels[gammaindex] != value)
        gammaindex++;

    if (gammaindex == GAMMALEVELS)
    {
        gammaindex = 0;

        while (gammalevels[gammaindex] != r_gamma_default)
            gammaindex++;
    }
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
    SDL_Event   dummy;
    SDL_version linked;
    SDL_version compiled;

    SDL_GetVersion(&linked);
    SDL_VERSION(&compiled);

    if (linked.major != compiled.major || linked.minor != compiled.minor)
        I_Error("The wrong version of %s was found. %s requires v%i.%i.%i.",
            SDL_FILENAME, PACKAGE_NAME, compiled.major, compiled.minor, compiled.patch);

    if (linked.patch != compiled.patch)
        C_Warning("The wrong version of <b>%s</b> was found. <i>%s</i> requires v%i.%i.%i.",
            SDL_FILENAME, PACKAGE_NAME, compiled.major, compiled.minor, compiled.patch);

    performancefrequency = SDL_GetPerformanceFrequency();

    for (int i = 0; i < UCHAR_MAX; i++)
        keys[i] = true;

    keys['v'] = keys['V'] = false;
    keys['s'] = keys['S'] = false;
    keys['i'] = keys['I'] = false;
    keys['r'] = keys['R'] = false;
    keys['a'] = keys['A'] = false;
    keys['l'] = keys['L'] = false;

    playpal = W_CacheLumpName("PLAYPAL");
    I_InitTintTables(playpal);
    FindNearestColors(playpal);

    I_InitGammaTables();

#if !defined(_WIN32)
    if (*vid_driver)
        SDL_setenv("SDL_VIDEODRIVER", vid_driver, true);
#endif

    SDL_InitSubSystem(SDL_INIT_VIDEO);
    GetDisplays();

#if defined(_DEBUG)
    vid_fullscreen = false;
#endif

    SetVideoMode(true);

    if (vid_fullscreen)
        SetShowCursor(false);

    mapscreen = oscreen = malloc(SCREENWIDTH * SCREENHEIGHT);
    I_CreateExternalAutomap(true);

#if defined(_WIN32)
    I_InitWindows32();
#endif

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    SDL_SetWindowTitle(window, PACKAGE_NAME);

    I_UpdateBlitFunc(false);
    blitfunc();

    while (SDL_PollEvent(&dummy));
}
