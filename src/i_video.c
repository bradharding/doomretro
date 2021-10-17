/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2021 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2021 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "SDL_opengl.h"

#include "am_map.h"
#include "c_cmds.h"
#include "c_console.h"
#include "d_deh.h"
#include "d_main.h"
#include "doomstat.h"
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
#include "m_random.h"
#include "r_main.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "version.h"
#include "w_wad.h"

#if defined(_WIN32)
void I_InitWindows32(void);
#endif

int             SCREENWIDTH;
int             SCREENHEIGHT = VANILLAHEIGHT * SCREENSCALE;
int             SCREENAREA;
int             WIDESCREENDELTA;    // [crispy] horizontal widescreen offset
int             WIDEFOVDELTA;

dboolean        nowidescreen = false;

int             MAPWIDTH;
unsigned int    MAPHEIGHT = VANILLAHEIGHT * SCREENSCALE;
unsigned int    MAPAREA;
int             MAPBOTTOM;

#define I_SDLError(func)        I_Error(stringize(func) "() failed in %s() on line %i of %s with this error:\"%s\".", \
                                    __FUNCTION__, __LINE__ - 1, leafname(__FILE__), SDL_GetError())

#define MAXDISPLAYS             8

#define MAXUPSCALEWIDTH         (2160 / VANILLAWIDTH)
#define MAXUPSCALEHEIGHT        (1200 / VANILLAHEIGHT)

#define SHAKEANGLE              ((double)M_BigRandomInt(-1000, 1000) * r_shake_damage / 100000.0)

// CVARs
dboolean            alwaysrun = alwaysrun_default;
dboolean            m_acceleration = m_acceleration_default;
int                 r_color = r_color_default;
float               r_gamma = r_gamma_default;
dboolean            vid_borderlesswindow = vid_borderlesswindow_default;
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
int                 vid_vsync = vid_vsync_default;
dboolean            vid_widescreen = vid_widescreen_default;
char                *vid_windowpos = vid_windowpos_default;
char                *vid_windowsize = vid_windowsize_default;

static dboolean     manuallypositioning;

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

static byte         *oscreen;
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

dboolean            windowfocused = true;

static dboolean     keys[NUMKEYS];

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

int                 framespersecond = 0;
int                 refreshrate;

#if defined(_WIN32)
HANDLE              CapFPSEvent;
#endif

static dboolean     capslock;

evtype_t            lasteventtype = ev_none;

extern int          windowborderwidth;
extern int          windowborderheight;

dboolean MouseShouldBeGrabbed(void)
{
    // if the window doesn't have focus, never grab it
    if (!windowfocused)
        return false;

    // always grab the mouse when fullscreen (don't want to see the mouse pointer)
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
    SDL_PumpEvents();
    SDL_SetRelativeMouseMode(!show);
    SDL_GetRelativeMouseState(NULL, NULL);
}

static int translatekey[] =
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

dboolean keystate(int key)
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

void FreeSurfaces(void)
{
    SDL_FreePalette(palette);
    SDL_FreeSurface(surface);
    SDL_FreeSurface(buffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(texture_upscaled);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = NULL;
}

void I_ShutdownGraphics(void)
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
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
    return (ABS(value) < deadzone ? 0 : (gp_analog ? MAX(-SDL_JOYSTICK_AXIS_MAX, value) : SIGN(value) * SDL_JOYSTICK_AXIS_MAX));
}

dboolean    altdown = false;
dboolean    waspaused = false;

static const SDL_Scancode keypad[] =
{
    SDL_SCANCODE_KP_1, SDL_SCANCODE_DOWN, SDL_SCANCODE_KP_3, SDL_SCANCODE_LEFT, SDL_SCANCODE_KP_5,
    SDL_SCANCODE_RIGHT, SDL_SCANCODE_KP_7, SDL_SCANCODE_UP, SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0
};

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
                    const unsigned char ch = Event->text.text[i];

                    if (isprint(ch))
                    {
                        event_t textevent = { ev_textinput, ch, 0, 0 };

                        D_PostEvent(&textevent);
                    }
                }

                break;

            case SDL_KEYDOWN:
            {
                SDL_Scancode    scancode = Event->key.keysym.scancode;

                if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_0 && !SDL_IsTextInputActive())
                    scancode = keypad[scancode - SDL_SCANCODE_KP_1];

                event.type = ev_keydown;
                event.data1 = translatekey[scancode];
                event.data2 = Event->key.keysym.sym;

                if (event.data2 < SDLK_SPACE || event.data2 > SDLK_z)
                    event.data2 = 0;

                altdown = Event->key.keysym.mod & KMOD_ALT;

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
                        C_Input(cheat_powerup[6].sequence);
                        C_Output(s_STSTR_BEHOLD);
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
            }

            case SDL_KEYUP:
            {
                SDL_Scancode    scancode = Event->key.keysym.scancode;

                if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_0 && !SDL_IsTextInputActive())
                    scancode = keypad[scancode - SDL_SCANCODE_KP_1];

                event.type = ev_keyup;
                event.data1 = translatekey[scancode];
                altdown = Event->key.keysym.mod & KMOD_ALT;
                keydown = 0;

#if !defined(_WIN32)
                // Handle ALT+ENTER on non-Windows systems
                if (event.data1 == KEY_ENTER)
                    enterdown = false;
#endif

                if (event.data1)
                    D_PostEvent(&event);

                break;
            }

            case SDL_MOUSEBUTTONDOWN:
                idclev = false;
                idmus = false;

                if (idbehold)
                {
                    HU_ClearMessages();
                    idbehold = false;
                    C_Input(cheat_powerup[6].sequence);
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
                event.type = ev_mousewheel;
                event.data1 = Event->wheel.y;
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

                        event.type = ev_gamepad;
                        D_PostEvent(&event);

                        break;

                    case SDL_CONTROLLER_AXIS_LEFTY:
                        if (gp_swapthumbsticks)
                            gamepadthumbRY = clamp(Event->caxis.value, gamepadrightdeadzone);
                        else
                            gamepadthumbLY = clamp(Event->caxis.value, gamepadleftdeadzone);

                        event.type = ev_gamepad;
                        D_PostEvent(&event);

                        break;

                    case SDL_CONTROLLER_AXIS_RIGHTX:
                        if (gp_swapthumbsticks)
                            gamepadthumbLX = clamp(Event->caxis.value, gamepadleftdeadzone);
                        else
                            gamepadthumbRX = clamp(Event->caxis.value, gamepadrightdeadzone);

                        event.type = ev_gamepad;
                        D_PostEvent(&event);

                        break;

                    case SDL_CONTROLLER_AXIS_RIGHTY:
                        if (gp_swapthumbsticks)
                            gamepadthumbLY = clamp(Event->caxis.value, gamepadleftdeadzone);
                        else
                            gamepadthumbRY = clamp(Event->caxis.value, gamepadrightdeadzone);

                        event.type = ev_gamepad;
                        D_PostEvent(&event);

                        break;

                    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                        if (Event->caxis.value >= GAMEPAD_TRIGGER_THRESHOLD)
                            gamepadbuttons |= GAMEPAD_LEFT_TRIGGER;
                        else
                            gamepadbuttons &= ~GAMEPAD_LEFT_TRIGGER;

                        event.type = ev_gamepad;
                        D_PostEvent(&event);

                        break;

                    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                        if (Event->caxis.value >= GAMEPAD_TRIGGER_THRESHOLD)
                            gamepadbuttons |= GAMEPAD_RIGHT_TRIGGER;
                        else
                            gamepadbuttons &= ~GAMEPAD_RIGHT_TRIGGER;

                        event.type = ev_gamepad;
                        D_PostEvent(&event);

                        break;
                }

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
                                S_ResumeMusic();

                            I_InitKeyboard();
                            break;

                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            windowfocused = false;

                            if (gamestate == GS_LEVEL && !paused)
                            {
                                if (menuactive || consoleactive)
                                    S_PauseMusic();
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

static void I_ReadMouse(void)
{
    int         x, y;
    static int  prevmousebuttonstate = -1;

    SDL_GetRelativeMouseState(&x, &y);

    if (x || y || mousebuttonstate != prevmousebuttonstate)
    {
        event_t ev;

        ev.type = ev_mouse;
        ev.data1 = mousebuttonstate;

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

        D_PostEvent(&ev);
        prevmousebuttonstate = mousebuttonstate;
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

    if (grab == currently_grabbed)
        return;

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
    if (width * ACTUALHEIGHT < height * SCREENWIDTH)
        height = width * ACTUALHEIGHT / SCREENWIDTH;
    else
        width = height * SCREENWIDTH / ACTUALHEIGHT;

    upscaledwidth = MIN((width + SCREENWIDTH - 1) / SCREENWIDTH, MAXUPSCALEWIDTH);
    upscaledheight = MIN((height + SCREENHEIGHT - 1) / SCREENHEIGHT, MAXUPSCALEHEIGHT);
}

void (*blitfunc)(void);
void (*mapblitfunc)(void);

static void nullfunc(void) {}

static uint64_t performancefrequency;
uint64_t        starttime;
int             frames = -1;

static void CalculateFPS(void)
{
    uint64_t    currenttime = SDL_GetPerformanceCounter();

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
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
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
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_ShowFPS(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
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
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_Shake(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
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
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
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
    SDL_UpdateTexture(texture, &src_rect, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderCopyEx(renderer, texture, &src_rect, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void I_Blit_Automap(void)
{
    SDL_LowerBlit(mapsurface, &map_rect, mapbuffer, &map_rect);
    SDL_UpdateTexture(maptexture, &map_rect, mappixels, mappitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(maprenderer, maptexture, &map_rect, NULL);
    SDL_RenderPresent(maprenderer);
}

static void I_Blit_Automap_NearestLinear(void)
{
    SDL_LowerBlit(mapsurface, &map_rect, mapbuffer, &map_rect);
    SDL_UpdateTexture(maptexture, &map_rect, mappixels, mappitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(maprenderer, maptexture_upscaled);
    SDL_RenderCopy(maprenderer, maptexture, &map_rect, NULL);
    SDL_SetRenderTarget(maprenderer, NULL);
    SDL_RenderCopy(maprenderer, maptexture_upscaled, NULL, NULL);
    SDL_RenderPresent(maprenderer);
}

void I_UpdateBlitFunc(dboolean shake)
{
    dboolean    nearest = (nearestlinear && (displayheight % VANILLAHEIGHT));

    if (shake && !software)
        blitfunc = (nearest ? (vid_showfps ? &I_Blit_NearestLinear_ShowFPS_Shake : &I_Blit_NearestLinear_Shake) :
            (vid_showfps ? &I_Blit_ShowFPS_Shake : &I_Blit_Shake));
    else
        blitfunc = (nearest ? (vid_showfps ? &I_Blit_NearestLinear_ShowFPS : &I_Blit_NearestLinear) :
            (vid_showfps ? &I_Blit_ShowFPS : &I_Blit));

    mapblitfunc = (mapwindow ? (nearest ? &I_Blit_Automap_NearestLinear : &I_Blit_Automap) : &nullfunc);
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
            byte    *gamma = gammatable[gammaindex];

            colors[i].r = gamma[*playpal++];
            colors[i].g = gamma[*playpal++];
            colors[i].b = gamma[*playpal++];
        }
    }
    else
    {
        double  color = r_color / 100.0;

        for (int i = 0; i < 256; i++)
        {
            byte    *gamma = gammatable[gammaindex];
            byte    r = gamma[*playpal++];
            byte    g = gamma[*playpal++];
            byte    b = gamma[*playpal++];
            double  p = sqrt((double)r * r * 0.299 + (double)g * g * 0.587 + (double)b * b * 0.114);

            colors[i].r = (byte)(p + (r - p) * color);
            colors[i].g = (byte)(p + (g - p) * color);
            colors[i].b = (byte)(p + (b - p) * color);
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

void I_SetPaletteWithBrightness(byte *playpal, double brightness)
{
    if (r_color == r_color_max)
    {
        for (int i = 0; i < 256; i++)
        {
            byte    *gamma = gammatable[gammaindex];

            colors[i].r = (byte)(gamma[*playpal++] * brightness);
            colors[i].g = (byte)(gamma[*playpal++] * brightness);
            colors[i].b = (byte)(gamma[*playpal++] * brightness);
        }
    }
    else
    {
        double  color = r_color / 100.0;

        for (int i = 0; i < 256; i++)
        {
            byte    *gamma = gammatable[gammaindex];
            double  r = gamma[*playpal++] * brightness;
            double  g = gamma[*playpal++] * brightness;
            double  b = gamma[*playpal++] * brightness;
            double  p = sqrt(r * r * 0.299 + g * g * 0.587 + b * b * 0.114);

            colors[i].r = (byte)(p + (r - p) * color);
            colors[i].g = (byte)(p + (g - p) * color);
            colors[i].b = (byte)(p + (b - p) * color);
        }
    }

    SDL_SetPaletteColors(palette, colors, 0, 256);
}

static void I_RestoreFocus(void)
{
#if defined(_WIN32)
    SDL_SysWMinfo   info;

    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(window, &info))
        SetFocus(info.info.win.window);
#endif
}

static void GetDisplays(void)
{
    numdisplays = MIN(SDL_GetNumVideoDisplays(), MAXDISPLAYS);

    for (int i = 0; i < numdisplays; i++)
        if (SDL_GetDisplayBounds(i, &displays[i]) < 0)
            I_SDLError(SDL_GetDisplayBounds);

    if ((double)displays[displayindex].w / displays[displayindex].h <= NONWIDEASPECTRATIO)
    {
        nowidescreen = true;
        vid_widescreen = false;
    }
}

void I_CreateExternalAutomap(int outputlevel)
{
    uint32_t    pixelformat;
    uint32_t    rmask;
    uint32_t    gmask;
    uint32_t    bmask;
    uint32_t    amask;
    int         bpp;
    int         am_displayindex = !displayindex;

    mapscreen = *screens;
    mapblitfunc = &nullfunc;

    if (!am_external)
        return;

    GetDisplays();

    if (numdisplays == 1)
    {
        if (outputlevel >= 1 && !togglingvanilla)
            C_Warning(1, "An external automap couldn't be created. Only one display was found.");

        return;
    }

    if (!(SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0", SDL_HINT_OVERRIDE)))
        I_SDLError(SDL_SetHintWithPriority);

    if (!mapwindow && !(mapwindow = SDL_CreateWindow("Automap", SDL_WINDOWPOS_UNDEFINED_DISPLAY(am_displayindex),
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(am_displayindex), 0, 0, (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SKIP_TASKBAR))))
        I_SDLError(SDL_CreateWindow);

    MAPHEIGHT = VANILLAHEIGHT * SCREENSCALE;
    MAPWIDTH = MIN((displays[am_displayindex].w * MAPHEIGHT / displays[am_displayindex].h + 1) & ~3, MAXWIDTH);
    MAPAREA = MAPWIDTH * MAPHEIGHT;

    if (!(maprenderer = SDL_CreateRenderer(mapwindow, -1, SDL_RENDERER_TARGETTEXTURE)))
        I_SDLError(SDL_CreateRenderer);

    if (SDL_RenderSetLogicalSize(maprenderer, MAPWIDTH, MAPHEIGHT) < 0)
        I_SDLError(SDL_RenderSetLogicalSize);

    if (!(mapsurface = SDL_CreateRGBSurface(0, MAPWIDTH, MAPHEIGHT, 8, 0, 0, 0, 0)))
        I_SDLError(SDL_CreateRGBSurface);

    pixelformat = SDL_GetWindowPixelFormat(mapwindow);

    if (!(SDL_PixelFormatEnumToMasks(pixelformat, &bpp, &rmask, &gmask, &bmask, &amask)))
        I_SDLError(SDL_PixelFormatEnumToMasks);

    if (!(mapbuffer = SDL_CreateRGBSurface(0, MAPWIDTH, MAPHEIGHT, bpp, rmask, gmask, bmask, amask)))
        I_SDLError(SDL_CreateRGBSurface);

    mappitch = mapbuffer->pitch;
    mappixels = mapbuffer->pixels;

    SDL_FillRect(mapbuffer, NULL, 0);

    if (nearestlinear && !(SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE)))
        I_SDLError(SDL_SetHintWithPriority);

    if (!(maptexture = SDL_CreateTexture(maprenderer, pixelformat, SDL_TEXTUREACCESS_STREAMING, MAPWIDTH, MAPHEIGHT)))
        I_SDLError(SDL_CreateTexture);

    if (nearestlinear)
    {
        if (!(SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear, SDL_HINT_OVERRIDE)))
            I_SDLError(SDL_SetHintWithPriority);

        if (!(maptexture_upscaled = SDL_CreateTexture(maprenderer, pixelformat, SDL_TEXTUREACCESS_TARGET,
            upscaledwidth * MAPWIDTH, upscaledheight * MAPHEIGHT)))
            I_SDLError(SDL_CreateTexture);

        mapblitfunc = &I_Blit_Automap_NearestLinear;
    }
    else
        mapblitfunc = &I_Blit_Automap;

    if (!(mappalette = SDL_AllocPalette(256)))
        I_SDLError(SDL_AllocPalette);

    if (SDL_SetSurfacePalette(mapsurface, mappalette) < 0)
        I_SDLError(SDL_SetSurfacePalette);

    if (SDL_SetPaletteColors(mappalette, colors, 0, 256) < 0)
        I_SDLError(SDL_SetPaletteColors);

    mapscreen = mapsurface->pixels;
    map_rect.w = MAPWIDTH;
    map_rect.h = MAPHEIGHT;

    I_RestoreFocus();

    if (outputlevel == 2)
    {
        const char  *displayname = SDL_GetDisplayName(am_displayindex);

        if (*displayname)
            C_Output("Created an external automap on \"%s\" (display %i).", displayname, am_displayindex + 1);
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
    char    width[11] = "";
    char    height[11] = "";

    if (sscanf(vid_windowsize, "%10[^x]x%10[^x]", width, height) != 2)
    {
        windowheight = SCREENHEIGHT + windowborderheight;
        windowwidth = SCREENHEIGHT * 16 / 10 + windowborderwidth;
        vid_windowsize = vid_windowsize_default;
        M_SaveCVARs();
    }
    else
    {
        char    *temp1 = uncommify(width);
        char    *temp2 = uncommify(height);
        int     w = atoi(temp1);
        int     h = atoi(temp2);

        if (w < VANILLAWIDTH + windowborderwidth || h < VANILLAWIDTH * 3 / 4 + windowborderheight)
        {
            char    size[16];
            char    *temp3 = commify((windowwidth = VANILLAWIDTH + windowborderwidth));
            char    *temp4 = commify((windowheight = VANILLAWIDTH * 3 / 4 + windowborderheight));

            M_snprintf(size, sizeof(size), "%sx%s", temp3, temp4);
            vid_windowsize = M_StringDuplicate(size);
            M_SaveCVARs();

            free(temp3);
            free(temp4);
        }
        else
        {
            windowwidth = w;
            windowheight = h;
        }

        free(temp1);
        free(temp2);
    }
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
        int width;
        int height;

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

    M_snprintf(ratio, sizeof(ratio), "%s %i:%i", (width == 8 ? "an" : "a"), width, height);
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

static void SetVideoMode(dboolean createwindow, dboolean output)
{
    int                 rendererflags = SDL_RENDERER_TARGETTEXTURE;
    int                 windowflags = SDL_WINDOW_RESIZABLE;
    int                 width, height;
    uint32_t            pixelformat;
    uint32_t            rmask;
    uint32_t            gmask;
    uint32_t            bmask;
    uint32_t            amask;
    int                 bpp = 0;
    SDL_RendererInfo    rendererinfo;
    const char          *displayname = SDL_GetDisplayName((displayindex = vid_display - 1));

    if (displayindex < 0 || displayindex >= numdisplays)
    {
        if (output)
            C_Warning(1, "Unable to find display %i.", vid_display);

        displayname = SDL_GetDisplayName((displayindex = vid_display_default - 1));
    }

    if (output)
    {
        if (displayname)
        {
            if (numdisplays == 1)
                C_Output("Using the \"%s\" display.", displayname);
            else
                C_Output("Using \"%s\" (display %i of %i).", displayname, displayindex + 1, numdisplays);
        }
        else
        {
            if (numdisplays != 1)
                C_Output("Using display %i of %i.", displayindex + 1, numdisplays);
        }
    }

    if (nowidescreen)
    {
        consolecmds[C_GetIndex(stringize(vid_widescreen))].flags |= CF_READONLY;
        C_Warning(0, "The aspect ratio of this display is too low to support widescreen modes.");
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

        if (!(SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter, SDL_HINT_OVERRIDE)))
            I_SDLError(SDL_SetHintWithPriority);
    }

    if (!(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, vid_scaleapi, SDL_HINT_OVERRIDE)))
        I_SDLError(SDL_SetHintWithPriority);

    software = M_StringCompare(vid_scaleapi, vid_scaleapi_software);

    GetWindowPosition();
    GetWindowSize();
    GetScreenResolution();

    if (M_StringStartsWith(vid_scaleapi, "opengl"))
        windowflags |= SDL_WINDOW_OPENGL;

    if (vid_fullscreen)
    {
        if (!screenwidth && !screenheight)
        {
            width = displays[displayindex].w;
            height = displays[displayindex].h;

            if (!width || !height)
                I_Error("Graphics couldn't be initialized.");

            if (createwindow && !(window = SDL_CreateWindow(DOOMRETRO_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), width, height,
                (windowflags | (vid_borderlesswindow ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN)))))
                I_SDLError(SDL_CreateWindow);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("Staying at the native desktop resolution of %sx%s with %s aspect ratio.",
                    temp1, temp2, getaspectratio(width, height));

                free(temp1);
                free(temp2);
            }
        }
        else
        {
            width = screenwidth;
            height = screenheight;

            if (createwindow && !(window = SDL_CreateWindow(DOOMRETRO_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), width, height,
                (windowflags | (vid_borderlesswindow ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN)))))
                I_SDLError(SDL_CreateWindow);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("Switched to a resolution of %sx%s with %s aspect ratio.", temp1, temp2, getaspectratio(width, height));

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
            if (createwindow && !(window = SDL_CreateWindow(DOOMRETRO_NAME, SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex),
                SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex), width, height, windowflags)))
                I_SDLError(SDL_CreateWindow);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("Created a %sx%s resizable window centered on the screen.", temp1, temp2);

                free(temp1);
                free(temp2);
            }
        }
        else
        {
            if (createwindow && !(window = SDL_CreateWindow(DOOMRETRO_NAME, windowx, windowy, width, height, windowflags)))
                I_SDLError(SDL_CreateWindow);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("Created a %sx%s resizable window at (%i,%i).", temp1, temp2, windowx, windowy);

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
        if (!(renderer = SDL_CreateRenderer(window, -1, (SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE))))
            I_SDLError(SDL_CreateRenderer);
        else
        {
            C_Warning(1, "The " BOLD("vid_scaleapi") " CVAR was changed from " BOLD("%s") " to " BOLD("\"software\"") ".", vid_scaleapi);
            vid_scaleapi = vid_scaleapi_software;
            M_SaveCVARs();
        }
    }

    if (SDL_RenderSetLogicalSize(renderer, SCREENWIDTH * !vid_widescreen, ACTUALHEIGHT) < 0)
        I_SDLError(SDL_RenderSetLogicalSize);

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

            C_Output("Every frame is scaled up from %sx%s to %sx%s using nearest-neighbor interpolation and then down to %sx%s using "
                "linear filtering.", temp1, temp2, temp5, temp6, temp3, temp4);

            free(temp5);
            free(temp6);
        }
        else if (M_StringCompare(vid_scalefilter, vid_scalefilter_linear) && !software)
            C_Output("Every frame is scaled up from %sx%s to %sx%s using linear filtering.", temp1, temp2, temp3, temp4);
        else
            C_Output("Every frame is scaled up from %sx%s to %sx%s using nearest-neighbor interpolation.", temp1, temp2, temp3, temp4);

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

                if (!(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, vid_scaleapi, SDL_HINT_OVERRIDE)))
                    I_SDLError(SDL_SetHintWithPriority);

                if (output)
                    C_Output("This scaling is now done using hardware acceleration with " ITALICS("Direct3D %s."),
                        (SDL_VIDEO_RENDER_D3D11 ? "v11.0" : "v9.0"));
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
                C_Output("This scaling is done using hardware acceleration with " ITALICS("Direct3D %s."),
                    (SDL_VIDEO_RENDER_D3D11 ? "v11.0" : "v9.0"));

            if (!M_StringCompare(vid_scaleapi, vid_scaleapi_direct3d))
            {
                vid_scaleapi = vid_scaleapi_direct3d;
                M_SaveCVARs();
            }
        }
#elif defined(__APPLE__)
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

            if (!(SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE)))
                I_SDLError(SDL_SetHintWithPriority);

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

        if (output)
        {
            typedef const GLubyte   *(APIENTRY *glStringFn_t)(GLenum);
            glStringFn_t            pglGetString = (glStringFn_t)SDL_GL_GetProcAddress("glGetString");

            if (pglGetString)
            {
                const char  *graphicscard = (const char *)pglGetString(GL_RENDERER);
                const char  *vendor = (const char *)pglGetString(GL_VENDOR);

                if (graphicscard && vendor)
                    C_Output("Using %s " ITALICS("%s") " graphics card from " ITALICS("%s."),
                        (isvowel(graphicscard[0]) || M_StringStartsWith(graphicscard, "NVIDIA") ? "an" : "a"), graphicscard, vendor);
            }
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

        C_Output("Using the 256-color palette from the " BOLD("PLAYPAL") " lump in the %s " BOLD("%s") ".",
            (playpalwad->type == IWAD ? "IWAD" : "PWAD"), playpalwad->path);

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

    if (!(surface = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, 8, 0, 0, 0, 0)))
        I_SDLError(SDL_CreateRGBSurface);

    screens[0] = surface->pixels;

    pixelformat = SDL_GetWindowPixelFormat(window);

    if (!(SDL_PixelFormatEnumToMasks(pixelformat, &bpp, &rmask, &gmask, &bmask, &amask)))
        I_SDLError(SDL_PixelFormatEnumToMasks);

    if (!(buffer = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, bpp, rmask, gmask, bmask, amask)))
        I_SDLError(SDL_CreateRGBSurface);

    pitch = buffer->pitch;
    pixels = buffer->pixels;

    SDL_FillRect(buffer, NULL, 0);

    if (nearestlinear && !(SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE)))
        I_SDLError(SDL_SetHintWithPriority);

    if (!(texture = SDL_CreateTexture(renderer, pixelformat, SDL_TEXTUREACCESS_STREAMING, SCREENWIDTH, SCREENHEIGHT)))
        I_SDLError(SDL_CreateTexture);

    if (nearestlinear)
    {
        if (!(SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear, SDL_HINT_OVERRIDE)))
            I_SDLError(SDL_SetHintWithPriority);

        if (!(texture_upscaled = SDL_CreateTexture(renderer, pixelformat, SDL_TEXTUREACCESS_TARGET,
            upscaledwidth * SCREENWIDTH, upscaledheight * SCREENHEIGHT)))
            I_SDLError(SDL_CreateTexture);
    }

    if (!(palette = SDL_AllocPalette(256)))
        I_SDLError(SDL_AllocPalette);

    if (SDL_SetSurfacePalette(surface, palette) < 0)
        I_SDLError(SDL_SetSurfacePalette);

    I_SetPalette(&PLAYPAL[st_palette * 768]);

    src_rect.w = SCREENWIDTH;
    src_rect.h = SCREENHEIGHT;
}

static void I_GetScreenDimensions(void)
{
    if (vid_widescreen)
    {
        int width;
        int height;

        if (vid_fullscreen)
        {
            width = displays[displayindex].w;
            height = displays[displayindex].h;
        }
        else
        {
            GetWindowSize();

            width = windowwidth;
            height = windowheight;
        }

        SCREENWIDTH = MIN((width * ACTUALHEIGHT / height + 1) & ~3, MAXWIDTH);

        // r_fov * 0.82 is vertical FOV for 4:3 aspect ratio
        WIDEFOVDELTA = (int)(atan(width / (height / tan(r_fov * 0.82 * M_PI / 360.0))) * 360.0 / M_PI) - r_fov;
        WIDESCREENDELTA = ((SCREENWIDTH - NONWIDEWIDTH) / SCREENSCALE) / 2;
    }
    else
    {
        SCREENWIDTH = NONWIDEWIDTH;
        WIDEFOVDELTA = 0;
        WIDESCREENDELTA = 0;
    }

    SCREENAREA = SCREENWIDTH * SCREENHEIGHT;

    GetPixelSize();
}

void I_RestartGraphics(dboolean recreatewindow)
{
    if (recreatewindow)
        FreeSurfaces();

    I_GetScreenDimensions();

    SetVideoMode(recreatewindow, false);

    AM_SetAutomapSize(menuactive ? r_screensize_max : r_screensize);

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

    S_StartSound(NULL, sfx_stnmov);

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
    I_SetPalette(&PLAYPAL[st_palette * 768]);

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
    SDL_Event   dummy;
    SDL_version linked;
    SDL_version compiled;

    SDL_GetVersion(&linked);
    SDL_VERSION(&compiled);

    if (linked.major != compiled.major || linked.minor != compiled.minor)
        I_Error("The wrong version of %s was found. %s requires v%i.%i.%i.",
            SDL_FILENAME, DOOMRETRO_NAME, compiled.major, compiled.minor, compiled.patch);

    if (linked.patch != compiled.patch)
        C_Warning(1, "The wrong version of " BOLD("%s") " was found. " ITALICS("%s") " requires v%i.%i.%i.",
            SDL_FILENAME, DOOMRETRO_NAME, compiled.major, compiled.minor, compiled.patch);

    performancefrequency = SDL_GetPerformanceFrequency();

    for (int i = 0; i < NUMKEYS; i++)
        keys[i] = true;

    keys['v'] = keys['V'] = false;
    keys['s'] = keys['S'] = false;
    keys['i'] = keys['I'] = false;
    keys['r'] = keys['R'] = false;
    keys['a'] = keys['A'] = false;
    keys['l'] = keys['L'] = false;

    PLAYPAL = W_CacheLumpName("PLAYPAL");
    I_InitTintTables(PLAYPAL);
    FindNearestColors(PLAYPAL);

    I_InitGammaTables();
    I_SetGamma(r_gamma);

#if !defined(_WIN32)
    if (*vid_driver)
        SDL_setenv("SDL_VIDEODRIVER", vid_driver, true);
#endif

    SDL_InitSubSystem(SDL_INIT_VIDEO);
    GetDisplays();

#if defined(_DEBUG)
    vid_fullscreen = false;
#endif

    I_GetScreenDimensions();

#if defined(_WIN32)
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1", SDL_HINT_OVERRIDE);
#endif

    SetVideoMode(true, true);

    if (vid_fullscreen)
        SetShowCursor(false);

    mapscreen = oscreen = malloc(MAXSCREENAREA);
    I_CreateExternalAutomap(2);

#if defined(_WIN32)
    I_InitWindows32();
#endif

    SDL_SetWindowTitle(window, DOOMRETRO_NAME);

    I_UpdateBlitFunc(false);
    memset(screens[0], nearestblack, SCREENAREA);
    blitfunc();

    if (mapwindow)
    {
        memset(mapscreen, nearestblack, MAPAREA);
        mapblitfunc();
    }

    while (SDL_PollEvent(&dummy));
}
