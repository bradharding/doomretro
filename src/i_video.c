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

#if defined(X11)
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

#define SHAKEANGLE  ((double)M_BigRandomInt(-1000, 1000) * r_shake_damage / 200000.0)

int                 SCREENWIDTH;
int                 SCREENHEIGHT = VANILLAHEIGHT * 2;
int                 SCREENAREA;
int                 WIDESCREENDELTA;
int                 MAXWIDESCREENDELTA;
int                 WIDEFOVDELTA;

static int          WIDESCREENWIDTH;

bool                nowidescreen = false;
bool                vid_widescreen_copy;

int                 MAPWIDTH;
int                 MAPHEIGHT = VANILLAHEIGHT * 2;
int                 MAPAREA;
int                 MAPBOTTOM;

static bool         manuallypositioning;

SDL_Window          *window = NULL;
static unsigned int windowid;
static SDL_Renderer *renderer;
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
static SDL_Renderer *maprenderer;
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

static int          mousepointerx;
static int          mousepointery;

// Bit mask of mouse button state
static unsigned int mousebuttonstate;

static const int buttons[MAXMOUSEBUTTONS + 1] =
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

bool                usinggamecontroller = false;
bool                usingmouse = false;
bool                windowfocused = true;

int                 keydown = 0;
int                 keydown2 = 0;

static bool         keys[NUMKEYS];

static byte         gammatable[GAMMALEVELS][256];
static float        saturationtable[256][256][256];

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
static SDL_Rect     dest_rect;
static SDL_Rect     map_rect;

int                 framespersecond = 0;
int                 refreshrate;

static bool         capslock;

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
    if (((menuactive && !helpscreen) || consoleactive || gamestate == GS_TITLESCREEN)
        && m_pointer && usingmouse && !usinggamecontroller)
        return false;

    return true;
}

static void SetShowCursor(const bool show)
{
    SDL_PumpEvents();
    SDL_SetRelativeMouseMode(!show);
    SDL_GetRelativeMouseState(NULL, NULL);
}

static const int translatekey[] =
{
    0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0', KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE,
    KEY_TAB, ' ', '-', '=', '[', ']', '\\', 0, ';', '\'', '`', ',', '.', '/',
    KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8,
    KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_PRINTSCREEN, KEY_SCROLLLOCK, KEY_PAUSE,
    KEY_INSERT, KEY_HOME, KEY_PAGEUP, KEY_DELETE, KEY_END, KEY_PAGEDOWN,
    KEY_RIGHTARROW, KEY_LEFTARROW, KEY_DOWNARROW, KEY_UPARROW, KEY_NUMLOCK, '/',
    '*', '-', '=', KEY_ENTER, KEYP_1, KEYP_2, KEYP_3, KEYP_4, KEYP_5, KEYP_6,
    KEYP_7, KEYP_8, KEYP_9, KEYP_0, 0, 0, 0, 0, '=', 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_CTRL, KEY_SHIFT, KEY_ALT, 0, KEY_CTRL,
    KEY_SHIFT, KEY_ALT
};

bool keystate(const int key)
{
    const uint8_t   *state = SDL_GetKeyboardState(NULL);

    return state[SDL_GetScancodeFromKey(key)];
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
    Display *display = XOpenDisplay(0);

    XkbLockModifiers(display, XkbUseCoreKbd, 2, enabled * 2);
    XFlush(display);
    XCloseDisplay(display);
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

                if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_0
                    && !SDL_IsTextInputActive())
                    ev.data1 = translatekey[keypad[scancode - SDL_SCANCODE_KP_1]];
                else if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_RALT)
                    ev.data1 = translatekey[scancode];

                ev.data2 = Event->key.keysym.sym;

                if (ev.data2 < SDLK_SPACE || ev.data2 > SDLK_z)
                    ev.data2 = 0;

                altdown = (Event->key.keysym.mod & KMOD_ALT);

                if (ev.data1)
                {
                    ev.type = ev_keydown;

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
                        I_ToggleFullscreen(true);

                        return;
                    }
#endif

                    D_PostEvent(&ev);

                    if (gamestate != GS_LEVEL)
                        I_SaveMousePointerPosition();

                    usingmouse = false;
                    usinggamecontroller = false;
                }

                break;
            }

            case SDL_KEYUP:
            {
                const SDL_Scancode  scancode = Event->key.keysym.scancode;

                if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_0
                    && !SDL_IsTextInputActive())
                    ev.data1 = translatekey[keypad[scancode - SDL_SCANCODE_KP_1]];
                else if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_RALT)
                    ev.data1 = translatekey[scancode];

                altdown = (Event->key.keysym.mod & KMOD_ALT);
                keydown = 0;
                keydown2 = 0;

#if !defined(_WIN32)
                // Handle ALT+ENTER on non-Windows systems
                if (ev.data1 == KEY_ENTER)
                    enterdown = false;
#endif

                if (ev.data1)
                {
                    ev.type = ev_keyup;
                    D_PostEvent(&ev);
                }

                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            {
                const int   button = buttons[Event->button.button];

                idclev = false;
                idmus = false;

                if (idbehold)
                {
                    HU_ClearMessages();
                    idbehold = false;
                    C_Cheat(cheat_powerup[6].sequence);
                    C_Output(s_STSTR_BEHOLD);
                }

                mousebuttonstate |= button;

                if ((menuactive || consoleactive) && button == MOUSE_RIGHTBUTTON)
                    usingmouse = false;

                break;
            }

            case SDL_MOUSEBUTTONUP:
                keydown = 0;
                mousebuttonstate &= ~buttons[Event->button.button];
                break;

            case SDL_MOUSEWHEEL:
                keydown = 0;
                ev.type = ev_mousewheel;
                ev.data1 = Event->wheel.y;

                if (menuactive || consoleactive)
                    usingmouse = false;

                D_PostEvent(&ev);
                break;

            case SDL_TEXTINPUT:
            {
                char    *text = (char *)SDL_iconv_utf8_ucs4(Event->text.text);

                ev.data1 = (text[0] ? text[0] : Event->text.text[strlen(Event->text.text) - 1]);
                ev.type = ev_textinput;
                D_PostEvent(&ev);
                break;
            }

            case SDL_CONTROLLERDEVICEADDED:
                I_InitGameController();
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                I_ShutdownGameController();
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
                                paused = false;
                                S_ResumeMusic();

                                if (!mapwindow)
                                    S_StartSound(NULL, sfx_swtchx);

                                I_InitKeyboard();

                                if (reopenautomap)
                                {
                                    reopenautomap = false;
                                    AM_Start(true);
                                    viewactive = false;
                                }
                            }

                            break;

                        case SDL_WINDOWEVENT_FOCUS_LOST:
                        case SDL_WINDOWEVENT_MINIMIZED:
                            windowfocused = false;

                            if (!s_musicinbackground)
                                S_PauseMusic();

                            if (gamestate == GS_LEVEL && !menuactive && !consoleactive && !paused)
                                sendpause = true;
                            else if (!mapwindow)
                                S_StartSound(NULL, sfx_swtchn);

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

static int AccelerateMouse(int value)
{
    return (value < -10 ? value * 2 + 10 : (value < 10 ? value : value * 2 - 10));
}

static void I_ReadMouse(void)
{
    int                 x, y;
    static unsigned int prevmousebuttonstate;

    SDL_GetRelativeMouseState(&x, &y);

    if (x || y || (mousebuttonstate != prevmousebuttonstate || (mousebuttonstate && menuactive)))
    {
        event_t ev = { ev_mouse, mousebuttonstate, 0, 0 };

        if (((menuactive && !helpscreen) || consoleactive || gamestate == GS_TITLESCREEN)
            && !splashscreen && m_pointer)
        {
            if (x || y)
            {
                usingmouse = true;
                usinggamecontroller = false;
            }

            SDL_GetMouseState(&x, &y);

            if (vid_widescreen)
            {
                ev.data2 = (x - dest_rect.x) * SCREENWIDTH / dest_rect.w / 2;
                ev.data3 = (y - dest_rect.y) * SCREENHEIGHT / dest_rect.h / 2;
            }
            else
            {
                ev.data2 = x * WIDESCREENWIDTH / displaywidth / 2;
                ev.data3 = y * SCREENHEIGHT / displayheight / 2;
            }
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
    I_ReadGameController();
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

static void nullfunc(void) {}

uint64_t    performancecounter;
uint64_t    performancefrequency;
uint64_t    starttime;
int         frames = -1;

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

void I_CapFPS(const int cap)
{
    const uint64_t  targettime = 1000000 / cap;
    static uint64_t startingtime;

    while (true)
    {
        const uint64_t  currenttime = I_GetTimeUS();
        const uint64_t  elapsedtime = currenttime - startingtime;

        if (elapsedtime >= targettime)
        {
            startingtime = currenttime;
            break;
        }
        else
        {
            const uint64_t  remainingtime = targettime - elapsedtime;

            if (remainingtime > 1000)
                I_Sleep(((int)remainingtime - 1000) / 1000);
        }
    }
}

#if defined(_WIN32)
void I_WindowResizeBlit(void)
{
    if (vid_showfps)
        CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);

    if (nearestlinear)
    {
        SDL_SetRenderTarget(renderer, texture_upscaled);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
    }
    else
        SDL_RenderCopy(renderer, texture, NULL, NULL);

    SDL_RenderPresent(renderer);
}
#endif

static void I_Blit(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
}

static void I_Blit_ShowFPS(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_ShowFPS(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
}

static void I_Blit_Shake(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopyEx(renderer, texture, NULL, &dest_rect, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_Shake(void)
{
    UpdateGrab();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopyEx(renderer, texture, NULL, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
}

static void I_Blit_ShowFPS_Shake(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopyEx(renderer, texture, NULL, &dest_rect, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);
}

static void I_Blit_NearestLinear_ShowFPS_Shake(void)
{
    UpdateGrab();
    CalculateFPS();

    SDL_LowerBlit(surface, &src_rect, buffer, &src_rect);
    SDL_UpdateTexture(texture, NULL, pixels, pitch);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopyEx(renderer, texture, NULL, NULL, SHAKEANGLE, NULL, SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
}

static void I_Blit_Automap(void)
{
    SDL_LowerBlit(mapsurface, &map_rect, mapbuffer, &map_rect);
    SDL_UpdateTexture(maptexture, &map_rect, mappixels, mappitch);
    SDL_RenderClear(maprenderer);
    SDL_RenderCopy(maprenderer, maptexture, NULL, NULL);
    SDL_RenderPresent(maprenderer);
}

static void I_Blit_Automap_NearestLinear(void)
{
    SDL_LowerBlit(mapsurface, &map_rect, mapbuffer, &map_rect);
    SDL_UpdateTexture(maptexture, &map_rect, mappixels, mappitch);
    SDL_RenderClear(maprenderer);
    SDL_SetRenderTarget(maprenderer, maptexture_upscaled);
    SDL_RenderCopy(maprenderer, maptexture, NULL, NULL);
    SDL_SetRenderTarget(maprenderer, NULL);
    SDL_RenderCopy(maprenderer, maptexture_upscaled, NULL, NULL);
    SDL_RenderPresent(maprenderer);
}

void I_UpdateBlitFunc(const bool shaking)
{
    if (nearestlinear && (displayheight % VANILLAHEIGHT))
    {
        if (shaking && !software)
            blitfunc = (vid_showfps ? &I_Blit_NearestLinear_ShowFPS_Shake : &I_Blit_NearestLinear_Shake);
        else
            blitfunc = (vid_showfps ? &I_Blit_NearestLinear_ShowFPS : &I_Blit_NearestLinear);

        mapblitfunc = (mapwindow ? &I_Blit_Automap_NearestLinear : &nullfunc);
    }
    else
    {
        if (shaking && !software)
            blitfunc = (vid_showfps ? &I_Blit_ShowFPS_Shake : &I_Blit_Shake);
        else
            blitfunc = (vid_showfps ? &I_Blit_ShowFPS : &I_Blit);

        mapblitfunc = (mapwindow ? &I_Blit_Automap : &nullfunc);
    }
}

//
// I_SetPalette
//
void I_SetPalette(const byte *playpal)
{
    const byte  *gamma = gammatable[gammaindex];

    if (r_saturation == r_saturation_default)
    {
        for (int i = 0; i < 256; i++)
        {
            colors[i].r = gamma[*playpal++];
            colors[i].g = gamma[*playpal++];
            colors[i].b = gamma[*playpal++];
        }
    }
    else
    {
        const float saturation = r_saturation / 100.0f;

        for (int i = 0; i < 256; i++)
        {
            const byte  r = gamma[*playpal++];
            const byte  g = gamma[*playpal++];
            const byte  b = gamma[*playpal++];
            const float p = saturationtable[r][g][b];

            colors[i].r = (byte)BETWEENF(0.0f, p + (r - p) * saturation, 255.0f);
            colors[i].g = (byte)BETWEENF(0.0f, p + (g - p) * saturation, 255.0f);
            colors[i].b = (byte)BETWEENF(0.0f, p + (b - p) * saturation, 255.0f);
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

void I_SetPaletteWithBrightness(const byte *playpal, const float brightness)
{
    const byte  *gamma = gammatable[gammaindex];

    if (r_saturation == r_saturation_default)
    {
        for (int i = 0; i < 256; i++)
        {
            colors[i].r = (byte)(gamma[*playpal++] * brightness);
            colors[i].g = (byte)(gamma[*playpal++] * brightness);
            colors[i].b = (byte)(gamma[*playpal++] * brightness);
        }
    }
    else
    {
        const float saturation = r_saturation / 100.0f;

        for (int i = 0; i < 256; i++)
        {
            const byte  r = gamma[*playpal++];
            const byte  g = gamma[*playpal++];
            const byte  b = gamma[*playpal++];
            const float p = saturationtable[r][g][b];

            colors[i].r = (byte)(BETWEENF(0.0f, p + (r - p) * saturation, 255.0f) * brightness);
            colors[i].g = (byte)(BETWEENF(0.0f, p + (g - p) * saturation, 255.0f) * brightness);
            colors[i].b = (byte)(BETWEENF(0.0f, p + (b - p) * saturation, 255.0f) * brightness);
        }
    }

    SDL_SetPaletteColors(palette, colors, 0, 256);
}

static void GetDisplays(void)
{
    if ((numdisplays = MIN(SDL_GetNumVideoDisplays(), vid_display_max)) <= 0)
        I_SDLError("SDL_GetNumVideoDisplays", -1);

    for (int i = 0; i < numdisplays; i++)
        if (SDL_GetDisplayBounds(i, &displays[i]) < 0)
            I_SDLError("SDL_GetDisplayBounds", -1);

    if ((double)displays[displayindex].w / displays[displayindex].h <= NONWIDEASPECTRATIO)
    {
        nowidescreen = true;
        vid_widescreen = false;
    }
}

bool I_CreateExternalAutomap(void)
{
    const char  *displayname;
    uint32_t    rmask;
    uint32_t    gmask;
    uint32_t    bmask;
    uint32_t    amask;
    uint32_t    pixelformat;
    int         bpp = 0;

    mapscreen = *screens;
    mapblitfunc = &nullfunc;

    if (!am_external)
        return false;

    GetDisplays();

    if (am_display == vid_display)
    {
        if (!togglingvanilla)
            C_Warning(1, "The external automap can't be shown on display %i.", am_display);

        return false;
    }
    else if (am_display > numdisplays)
    {
        if (!togglingvanilla)
        {
            if (numdisplays == 1)
                C_Warning(1, "The external automap can't be shown on display %i. There is only 1 display.",
                    am_display);
            else
                C_Warning(1, "The external automap can't be shown on display %i. There are only %i displays.",
                    am_display, numdisplays);
        }

        return false;
    }

    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0", SDL_HINT_OVERRIDE);

    if (!(mapwindow = SDL_CreateWindow("Automap", SDL_WINDOWPOS_UNDEFINED_DISPLAY(am_display - 1),
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(am_display - 1), 0, 0,
        (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SKIP_TASKBAR))))
        I_SDLError("SDL_CreateWindow", -3);

    MAPHEIGHT = VANILLAHEIGHT * 2;
    MAPWIDTH = MIN(((displays[am_display - 1].w * MAPHEIGHT / displays[am_display - 1].h + 1) & ~3), MAXWIDTH);
    MAPAREA = MAPWIDTH * MAPHEIGHT;

    if (!(maprenderer = SDL_CreateRenderer(mapwindow, -1, SDL_RENDERER_TARGETTEXTURE)))
        I_SDLError("SDL_CreateRenderer", -1);

    if (SDL_RenderSetLogicalSize(maprenderer, MAPWIDTH, MAPHEIGHT) < 0)
        I_SDLError("SDL_RenderSetLogicalSize", -1);

    if (!(mapsurface = SDL_CreateRGBSurface(0, MAPWIDTH, MAPHEIGHT, 8, 0, 0, 0, 0)))
        I_SDLError("SDL_CreateRGBSurface", -1);

    if ((pixelformat = SDL_GetWindowPixelFormat(mapwindow)) == SDL_PIXELFORMAT_UNKNOWN)
        I_SDLError("SDL_GetWindowPixelFormat", -1);

    if (!SDL_PixelFormatEnumToMasks(pixelformat, &bpp, &rmask, &gmask, &bmask, &amask))
        I_SDLError("SDL_PixelFormatEnumToMasks", -1);

    if (!(mapbuffer = SDL_CreateRGBSurface(0, MAPWIDTH, MAPHEIGHT, bpp, rmask, gmask, bmask, amask)))
        I_SDLError("SDL_CreateRGBSurface", -1);

    mappitch = mapbuffer->pitch;
    mappixels = mapbuffer->pixels;

    SDL_FillRect(mapbuffer, NULL, 0);

    if (nearestlinear)
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE);

    if (!(maptexture = SDL_CreateTexture(maprenderer, pixelformat, SDL_TEXTUREACCESS_STREAMING,
        MAPWIDTH, MAPHEIGHT)))
        I_SDLError("SDL_CreateTexture", -2);

    if (nearestlinear)
    {
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear, SDL_HINT_OVERRIDE);

        if (!(maptexture_upscaled = SDL_CreateTexture(maprenderer, pixelformat,
            SDL_TEXTUREACCESS_TARGET, upscaledwidth * MAPWIDTH, upscaledheight * MAPHEIGHT)))
            I_SDLError("SDL_CreateTexture", -2);

        mapblitfunc = &I_Blit_Automap_NearestLinear;
    }
    else
        mapblitfunc = &I_Blit_Automap;

    if (!(mappalette = SDL_AllocPalette(256)))
        I_SDLError("SDL_AllocPalette", -1);

    if (SDL_SetSurfacePalette(mapsurface, mappalette) < 0)
        I_SDLError("SDL_SetSurfacePalette", -1);

    if (SDL_SetPaletteColors(mappalette, colors, 0, 256) < 0)
        I_SDLError("SDL_SetPaletteColors", -1);

    mapscreen = mapsurface->pixels;
    memset(mapscreen, nearestblack, MAPAREA);

    map_rect.w = MAPWIDTH;
    map_rect.h = MAPHEIGHT;

    if ((displayname = SDL_GetDisplayName(am_display - 1)))
        C_Output("\"%s\" (display %i of %i) is being used for the external automap.",
            displayname, am_display, numdisplays);
    else
        C_Output("Display %i of %i is being used for the external automap.",
            am_display, numdisplays);

    SDL_RaiseWindow(window);

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

    if (M_StringCompare(vid_windowpos, vid_windowpos_centered)
        || M_StringCompare(vid_windowpos, vid_windowpos_centred))
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
        windowx = BETWEEN(displays[displayindex].x, x,
            displays[displayindex].x + displays[displayindex].w - 50);
        windowy = BETWEEN(displays[displayindex].y, y,
            displays[displayindex].y + displays[displayindex].h - 50);
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

static bool ValidScreenMode(const int width, const int height)
{
    const int   modes = SDL_GetNumDisplayModes(displayindex);

    if (modes <= 0)
        I_SDLError("SDL_GetNumDisplayModes", -3);

    for (int i = 0; i < modes; i++)
    {
        SDL_DisplayMode mode;

        if (SDL_GetDisplayMode(displayindex, i, &mode) < 0)
            I_SDLError("SDL_GetDisplayMode", -1);

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

        if (sscanf(vid_screenresolution, "%10ix%10i", &width, &height) != 2
            || !ValidScreenMode(width, height))
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
    const int   hcf = gcd(width, height);
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
        SDL_SetWindowPosition(window,
            displays[displayindex].x + (displays[displayindex].w - windowwidth) / 2,
            displays[displayindex].y + (displays[displayindex].h - windowheight) / 2);
}

void I_SetMotionBlur(const int percent)
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

static void SetVideoMode(const bool createwindow, const bool output)
{
    int                 rendererflags = SDL_RENDERER_TARGETTEXTURE;
    int                 windowflags = (SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    int                 width, height;
    SDL_RendererInfo    rendererinfo;
    const char          *displayname = SDL_GetDisplayName((displayindex = vid_display - 1));
    bool                instead = false;
    uint32_t            rmask;
    uint32_t            gmask;
    uint32_t            bmask;
    uint32_t            amask;
    uint32_t            pixelformat;
    int                 bpp = 0;

    if (displayindex >= numdisplays)
    {
        if (output)
            C_Warning(1, "Display %i wasn't found.", vid_display);

        displayname = SDL_GetDisplayName((displayindex = vid_display_default - 1));
        instead = true;
    }

    if (output)
    {
        if (numdisplays == 1)
        {
            if (displayname)
                C_Output("\"%s\" is being used%s.",
                    displayname, (instead ? " instead" : ""));
        }
        else
        {
            if (displayname)
                C_Output("\"%s\" (display %i of %i) is being used%s.",
                    displayname, displayindex + 1, numdisplays, (instead ? " instead" : ""));
            else
                C_Output("Display %i of %i is being used%s.",
                    displayindex + 1, numdisplays, (instead ? " instead" : ""));
        }
    }

    if (nowidescreen && output)
    {
        consolecmds[C_GetIndex(stringize(vid_widescreen))].flags |= CF_READONLY;
        C_Warning(1, "The aspect ratio of display %i is too low to show "
            ITALICS(DOOMRETRO_NAME) " in widescreen.", displayindex + 1);
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
                I_Error("Graphics couldn't be %s.",
                    (english == english_american ? "initialized" : "initialised"));

            if (createwindow)
                if (!(window = SDL_CreateWindow(DOOMRETRO_NAME, SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                    SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), width, height,
                    (windowflags | (vid_borderlesswindow ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN)))))
                    I_SDLError("SDL_CreateWindow", -3);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("The native desktop resolution of %sx%s with an aspect ratio of %s is being used.",
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
                if (!(window = SDL_CreateWindow(DOOMRETRO_NAME,
                    SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex),
                    SDL_WINDOWPOS_UNDEFINED_DISPLAY(displayindex), width, height,
                    (windowflags | (vid_borderlesswindow ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN)))))
                    I_SDLError("SDL_CreateWindow", -3);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("A resolution of %sx%s with an aspect ratio of %s is being used.",
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
                if (!(window = SDL_CreateWindow(DOOMRETRO_NAME, SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex),
                    SDL_WINDOWPOS_CENTERED_DISPLAY(displayindex), width, height, windowflags)))
                    I_SDLError("SDL_CreateWindow", -2);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("A %sx%s resizable window is %s on the screen.",
                    temp1, temp2, (english == english_american ? vid_windowpos_centered : vid_windowpos_centred));

                free(temp1);
                free(temp2);
            }
        }
        else
        {
            if (createwindow)
                if (!(window = SDL_CreateWindow(DOOMRETRO_NAME, windowx, windowy, width, height, windowflags)))
                    I_SDLError("SDL_CreateWindow", -1);

            if (output)
            {
                char    *temp1 = commify(width);
                char    *temp2 = commify(height);

                C_Output("A %sx%s resizable window is at (%i, %i).", temp1, temp2, windowx, windowy);

                free(temp1);
                free(temp2);
            }
        }
    }

    GetUpscaledTextureSize(width, height);

    windowid = SDL_GetWindowID(window);

    SDL_GetWindowSize(window, &displaywidth, &displayheight);

    if (createwindow && !(renderer = SDL_CreateRenderer(window, -1, rendererflags)) && !software)
    {
        if ((renderer = SDL_CreateRenderer(window, -1, (SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE))))
        {
            C_Warning(1, "The " BOLD("vid_scaleapi") " CVAR has been changed from " BOLD("%s")
                " to " BOLD("\"software\"") ".", vid_scaleapi);
            vid_scaleapi = vid_scaleapi_software;
            M_SaveCVARs();
        }
    }

    if (SDL_RenderSetLogicalSize(renderer, !vid_widescreen * SCREENWIDTH, ACTUALHEIGHT) < 0)
        I_SDLError("SDL_RenderSetLogicalSize", -1);

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
                "and then back down to %sx%s using linear filtering.", temp1, temp2, temp5, temp6,
                (english == english_american ? "neighbor" : "neighbour"), temp3, temp4);

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
                vid_scaleapi = vid_scaleapi_direct3d9;
                M_SaveCVARs();

                SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, vid_scaleapi_direct3d9, SDL_HINT_OVERRIDE);

                if (output)
                    C_Output("This scaling is now done using hardware acceleration with "
                        ITALICS("Direct3D 9."));
#endif
            }
            else
            {
                if (output)
                    C_Output("This scaling is done using hardware acceleration with "
                        ITALICS("OpenGL v%i.%i."),
                        major, minor);

                if (!M_StringCompare(vid_scaleapi, vid_scaleapi_opengl))
                {
                    vid_scaleapi = vid_scaleapi_opengl;
                    M_SaveCVARs();
                }
            }
        }
#if defined(_WIN32)
        else if (M_StringCompare(rendererinfo.name, "direct3d"))
        {
            if (output)
                C_Output("This scaling is done using hardware acceleration with "
                    ITALICS("Direct3D 9."));

            if (!M_StringCompare(vid_scaleapi, vid_scaleapi_direct3d9))
            {
                vid_scaleapi = vid_scaleapi_direct3d9;
                M_SaveCVARs();
            }
        }
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_direct3d11))
        {
            if (output)
                C_Output("This scaling is done using hardware acceleration with "
                    ITALICS("Direct3D 11."));

            if (!M_StringCompare(vid_scaleapi, vid_scaleapi_direct3d11))
            {
                vid_scaleapi = vid_scaleapi_direct3d11;
                M_SaveCVARs();
            }
        }
#else
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_opengles))
        {
            if (output)
                C_Output("This scaling is done using hardware acceleration with "
                    ITALICS("OpenGL ES."));
        }
        else if (M_StringCompare(rendererinfo.name, vid_scaleapi_opengles2))
        {
            if (output)
                C_Output("This scaling is done using hardware acceleration with "
                    ITALICS("OpenGL ES 2."));
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
                    if (output)
                        C_Output("The framerate is synced with the display's refresh rate of %iHz.",
                            refreshrate);
                }
                else
                {
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
            if (output)
            {
                if (vid_vsync)
                {
                    if (M_StringCompare(rendererinfo.name, vid_scaleapi_software))
                        C_Warning(1, "The framerate can't be synced with the "
                            "display's refresh rate in software.");
                    else
                        C_Warning(1, "The framerate can't be synced with the "
                            "display's refresh rate using this graphics card.");
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
            C_Output("The 256-%s palette from the " BOLD("PLAYPAL") " lump in the IWAD "
                BOLD("%s") " is being used.", (english == english_american ? "color" : "colour"),
                lumpinfo[W_GetLastNumForName("PLAYPAL")]->wadfile->path);
        else
            C_Output("The 256-%s palette from the " BOLD("PLAYPAL") " lump in the %s " BOLD("%s")
                " is being used.", (english == english_american ? "color" : "colour"),
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
        I_SDLError("SDL_CreateRGBSurface", -1);

    screens[0] = surface->pixels;

    if ((pixelformat = SDL_GetWindowPixelFormat(window)) == SDL_PIXELFORMAT_UNKNOWN)
        I_SDLError("SDL_GetWindowPixelFormat", -1);

    if (!SDL_PixelFormatEnumToMasks(pixelformat, &bpp, &rmask, &gmask, &bmask, &amask))
        I_SDLError("SDL_PixelFormatEnumToMasks", -1);

    if (!(buffer = SDL_CreateRGBSurface(0, SCREENWIDTH, SCREENHEIGHT, bpp, rmask, gmask, bmask, amask)))
        I_SDLError("SDL_CreateRGBSurface", -1);

    pitch = buffer->pitch;
    pixels = buffer->pixels;

    SDL_FillRect(buffer, NULL, 0);

    if (nearestlinear)
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_nearest, SDL_HINT_OVERRIDE);

    if (!(texture = SDL_CreateTexture(renderer, pixelformat, SDL_TEXTUREACCESS_STREAMING, SCREENWIDTH, SCREENHEIGHT)))
        I_SDLError("SDL_CreateTexture", -1);

    if (nearestlinear)
    {
        SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, vid_scalefilter_linear, SDL_HINT_OVERRIDE);

        if (!(texture_upscaled = SDL_CreateTexture(renderer, pixelformat, SDL_TEXTUREACCESS_TARGET,
            upscaledwidth * SCREENWIDTH, upscaledheight * SCREENHEIGHT)))
            I_SDLError("SDL_CreateTexture", -2);
    }

    if (!(palette = SDL_AllocPalette(256)))
        I_SDLError("SDL_AllocPalette", -1);

    if (SDL_SetSurfacePalette(surface, palette) < 0)
        I_SDLError("SDL_SetSurfacePalette", -1);

    I_SetPalette(&PLAYPAL[st_palette * 768]);

    src_rect.w = SCREENWIDTH;
    src_rect.h = SCREENHEIGHT;
}

static void I_GetScreenDimensions(void)
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

    if (vid_widescreen)
    {
        dest_rect.w = width;

        if (vid_aspectratio == vid_aspectratio_auto)
        {
            dest_rect.h = height;
            dest_rect.x = 0;
            dest_rect.y = 0;
        }
        else
        {
            if (vid_aspectratio == vid_aspectratio_16_9)
            {
                if ((dest_rect.h = width * 9 / 16) > height)
                {
                    dest_rect.w = height * 16 / 9;
                    dest_rect.h = height;
                }
            }
            else if (vid_aspectratio == vid_aspectratio_16_10)
            {
                if ((dest_rect.h = width * 10 / 16) > height)
                {
                    dest_rect.w = height * 16 / 10;
                    dest_rect.h = height;
                }
            }
            else if (vid_aspectratio == vid_aspectratio_21_9)
            {
                if ((dest_rect.h = width * 9 / 21) > height)
                {
                    dest_rect.w = height * 21 / 9;
                    dest_rect.h = height;
                }
            }
            else if (vid_aspectratio == vid_aspectratio_32_9)
            {
                if ((dest_rect.h = width * 9 / 32) > height)
                {
                    dest_rect.w = height * 32 / 9;
                    dest_rect.h = height;
                }
            }

            dest_rect.x = (width - dest_rect.w) / 2;
            dest_rect.y = (height - dest_rect.h) / 2;
        }

        SCREENWIDTH = BETWEEN(NONWIDEWIDTH, ((dest_rect.w * ACTUALHEIGHT / dest_rect.h + 1) & ~3), MAXWIDTH);
        WIDESCREENWIDTH = SCREENWIDTH;

        // r_fov * 0.82 is vertical FOV for 4:3 aspect ratio
        WIDEFOVDELTA = MIN((int)(atan(dest_rect.w / (dest_rect.h / tan(r_fov * 0.82 * M_PI / 360.0))) * 360.0 / M_PI) - r_fov - 2,
            MAXWIDEFOVDELTA);
        WIDESCREENDELTA = SCREENWIDTH / 4 - VANILLAWIDTH / 2;
        MAXWIDESCREENDELTA = MAX(53, WIDESCREENDELTA);
    }
    else
    {
        dest_rect.w = NONWIDEWIDTH;
        dest_rect.h = ACTUALHEIGHT;
        dest_rect.x = 0;
        dest_rect.y = 0;

        SCREENWIDTH = NONWIDEWIDTH;
        WIDESCREENWIDTH = BETWEEN(NONWIDEWIDTH, ((width * ACTUALHEIGHT / height + 1) & ~3), MAXWIDTH);
        WIDEFOVDELTA = 0;
        WIDESCREENDELTA = 0;
        MAXWIDESCREENDELTA = 53;
    }

    SCREENAREA = SCREENWIDTH * SCREENHEIGHT;

    GetPixelSize();
}

void I_RestartGraphics(const bool recreatewindow)
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

void I_ToggleFullscreen(const bool output)
{
    if (SDL_SetWindowFullscreen(window,
        (vid_fullscreen ? 0 : (vid_borderlesswindow ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN))) < 0)
    {
        menuactive = false;
        C_ShowConsole(false);
        C_Warning(0, "Unable to switch to %s.", (vid_fullscreen ? "a window" : "fullscreen"));
        return;
    }

    vid_fullscreen = !vid_fullscreen;
    I_RestartGraphics(vid_fullscreen && !vid_borderlesswindow);
    S_StartSound(NULL, sfx_stnmov);
    M_SaveCVARs();

    if (nearestlinear)
        I_UpdateBlitFunc(viewplayer && viewplayer->damagecount);

    if (vid_fullscreen)
    {
        if (output)
            C_StringCVAROutput(stringize(vid_fullscreen), "on");
    }
    else
    {
        if (output)
            C_StringCVAROutput(stringize(vid_fullscreen), "off");

        SDL_SetWindowSize(window, windowwidth, windowheight);

        displaywidth = windowwidth;
        displayheight = windowheight;

        PositionOnCurrentDisplay();
    }
}

void I_SetPillarboxes(void)
{
    I_SetPalette(&PLAYPAL[st_palette * 768]);

    if (!vid_pillarboxes)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

static void I_InitPaletteTables(void)
{
    for (int i = 0; i < GAMMALEVELS; i++)
        for (int j = 0; j < 256; j++)
            gammatable[i][j] = (byte)(powf(j / 255.0f, 1.0f / gammalevels[i]) * 255.0f + 0.5f);

    for (int r = 0; r < 256; r++)
        for (int g = 0; g < 256; g++)
            for (int b = 0; b < 256; b++)
                saturationtable[r][g][b] = sqrtf(r * r * 0.299f + g * g * 0.587f + b * b * 0.114f);
}

void I_SetGamma(const float value)
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
        I_Error(DOOMRETRO_NAME " requires SDL v%i.%i.%i.",
            compiled.major, compiled.minor, compiled.patch);

    if (linked.patch != compiled.patch)
        C_Warning(1, ITALICS(DOOMRETRO_NAME) " requires SDL v%i.%i.%i.",
            compiled.major, compiled.minor, compiled.patch);

    performancecounter = SDL_GetPerformanceCounter();
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

    I_InitPaletteTables();
    I_SetGamma(r_gamma);

#if !defined(_WIN32)
    if (*vid_driver)
        SDL_setenv("SDL_VIDEODRIVER", vid_driver, true);
#endif

    GetDisplays();

#if defined(_DEBUG)
    vid_fullscreen = false;
#endif

    vid_widescreen_copy = vid_widescreen;
    vid_widescreen = false;

    I_GetScreenDimensions();

#if defined(_WIN32)
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1", SDL_HINT_OVERRIDE);
#endif

    SDL_SetHintWithPriority(SDL_HINT_RENDER_BATCHING, "0", SDL_HINT_OVERRIDE);

    if (vid_fullscreen)
        SDL_ShowCursor(false);

    SetVideoMode(true, true);

    I_CreateExternalAutomap();

    if (vid_fullscreen)
        SetShowCursor(false);

    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

#if defined(_WIN32)
    I_InitWindows32();
#endif

    SDL_SetWindowTitle(window, DOOMRETRO_NAME);

    I_UpdateBlitFunc(false);
    memset(screens[0], nearestblack, SCREENAREA);
    blitfunc();

    if (mapwindow)
        mapblitfunc();

    SDL_StopTextInput();

    I_Sleep(1000);
}
