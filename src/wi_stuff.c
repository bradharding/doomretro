/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2025 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2025 by Brad Harding <mailto:brad@doomretro.com>.

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

#include "c_console.h"
#include "d_deh.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_swap.h"
#include "m_array.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "p_setup.h"
#include "s_sound.h"
#include "v_data.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_interlvl.h"
#include "wi_stuff.h"
#include "z_zone.h"

//
// Data needed to add patches to fullscreen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//

//
// Difference between registered DOOM (1994) and
//  Ultimate DOOM - Final edition (retail, 1995?).
// This is supposedly ignored for commercial
//  release (aka DOOM II), which had 34 maps
//  in one episode. So there.
#define NUMMAPS     9

// GLOBAL LOCATIONS
#define WI_TITLEY   12

// SINGLE-PLAYER STUFF
#define SP_STATSX   50
#define SP_STATSY   58

#define SP_TIMEX    16
#define SP_TIMEY    (VANILLAHEIGHT - 25)

typedef enum
{
    ANIM_ALWAYS,
    ANIM_LEVEL
} animenum_t;

typedef struct
{
    int         x, y;
} point_t;

//
// Animation.
// There is another anim_t used in p_spec.
//
typedef struct
{
    animenum_t  type;

    // period in tics between animations
    int         period;

    // number of animation frames
    int         nanims;

    // location of animation
    point_t     loc;

    // ALWAYS: n/a,
    // LEVEL: level
    int         data1;

    // actual graphics for frames of animations
    patch_t     *p[3];

    // following must be initialized to zero before use!

    // next value of bcnt (used in conjunction with period)
    int         nexttic;

    // next frame number to animate
    int         ctr;
} anim_t;

static point_t lnodes[][NUMMAPS] =
{
    // Episode 0 World Map
    {
        { 185, 164 },   // location of level 0 (CJ)
        { 148, 143 },   // location of level 1 (CJ)
        {  69, 122 },   // location of level 2 (CJ)
        { 209, 102 },   // location of level 3 (CJ)
        { 116,  89 },   // location of level 4 (CJ)
        { 166,  55 },   // location of level 5 (CJ)
        {  71,  56 },   // location of level 6 (CJ)
        { 135,  29 },   // location of level 7 (CJ)
        {  71,  24 }    // location of level 8 (CJ)
    },

    // Episode 1 World Map should go here
    {
        { 254,  25 },   // location of level 0 (CJ)
        {  97,  50 },   // location of level 1 (CJ)
        { 188,  64 },   // location of level 2 (CJ)
        { 128,  78 },   // location of level 3 (CJ)
        { 214,  92 },   // location of level 4 (CJ)
        { 133, 130 },   // location of level 5 (CJ)
        { 208, 136 },   // location of level 6 (CJ)
        { 148, 140 },   // location of level 7 (CJ)
        { 235, 158 }    // location of level 8 (CJ)
    },

    // Episode 2 World Map should go here
    {
        { 156, 168 },   // location of level 0 (CJ)
        {  48, 154 },   // location of level 1 (CJ)
        { 174,  95 },   // location of level 2 (CJ)
        { 265,  75 },   // location of level 3 (CJ)
        { 130,  48 },   // location of level 4 (CJ)
        { 279,  23 },   // location of level 5 (CJ)
        { 198,  48 },   // location of level 6 (CJ)
        { 140,  25 },   // location of level 7 (CJ)
        { 281, 136 }    // location of level 8 (CJ)
    }
};

//
// Animation locations for episode 0 (1).
// Using patches saves a lot of space,
//  as they replace 320x200 full screen frames.
//
static anim_t epsd0animinfo[] =
{
    { ANIM_ALWAYS, TICRATE / 3, 3, { 224, 104 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, { 184, 160 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, { 112, 136 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, {  72, 112 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, {  88,  96 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, {  64,  48 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, { 192,  40 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, { 136,  16 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, {  80,  16 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, {  64,  24 }, 0 }
};

static anim_t epsd1animinfo[] =
{
    { ANIM_LEVEL, TICRATE / 3, 1, { 128, 136 }, 1 },
    { ANIM_LEVEL, TICRATE / 3, 1, { 128, 136 }, 2 },
    { ANIM_LEVEL, TICRATE / 3, 1, { 128, 136 }, 3 },
    { ANIM_LEVEL, TICRATE / 3, 1, { 128, 136 }, 4 },
    { ANIM_LEVEL, TICRATE / 3, 1, { 128, 136 }, 5 },
    { ANIM_LEVEL, TICRATE / 3, 1, { 128, 136 }, 6 },
    { ANIM_LEVEL, TICRATE / 3, 1, { 128, 136 }, 7 },
    { ANIM_LEVEL, TICRATE / 3, 3, { 192, 144 }, 8 },
    { ANIM_LEVEL, TICRATE / 3, 1, { 128, 136 }, 8 }
};

static anim_t epsd2animinfo[] =
{
    { ANIM_ALWAYS, TICRATE / 3, 3, { 104, 168 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, {  40, 136 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, { 160,  96 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, { 104,  80 }, 0 },
    { ANIM_ALWAYS, TICRATE / 3, 3, { 120,  32 }, 0 },
    { ANIM_ALWAYS, TICRATE / 4, 3, {  40,   0 }, 0 }
};

static int numanims[] =
{
    arrlen(epsd0animinfo),
    arrlen(epsd1animinfo),
    arrlen(epsd2animinfo)
};

static anim_t *anims[] =
{
    epsd0animinfo,
    epsd1animinfo,
    epsd2animinfo
};

//
// GENERAL DATA
//

//
// Locally used stuff.
//

// in seconds
#define SHOWNEXTLOCDELAY    4

// used to accelerate or skip a stage
bool                    acceleratestage;

// specifies current state
static stateenum_t      state;

// contains information passed into intermission
static wbstartstruct_t  *wbs;

// used for general timing
static int              cnt;

// used for timing of background animation
static int              bcnt;

static int              cnt_kills;
static int              cnt_items;
static int              cnt_secret;
static int              cnt_time;
static int              cnt_par;
static int              cnt_pause;

// # of commercial levels
static int              numcmaps;

//
// GRAPHICS
//

// You Are Here graphic
static patch_t          *yah[3];

// splat
static patch_t          *splat[2];

// %, :, . graphics
static patch_t          *percent;
static patch_t          *colon;
static patch_t          *period;

// 0-9 graphic
static patch_t          *num[10];

// "Finished!" graphics
static patch_t          *finished;

// "Entering" graphic
static patch_t          *entering;

// "secret"
static patch_t          *sp_secret;

// "Kills", "Scrt", "Items"
static patch_t          *kills;
static patch_t          *items;

// Time sucks.
static patch_t          *timepatch;
static patch_t          *par;
static patch_t          *sucks;

// Name graphics of each level (centered)
static patch_t          **lnames;

static int              enterpic;
static int              exitpic;

typedef struct
{
    interlevelframe_t   *frames;
    int                 xpos;
    int                 ypos;
    int                 frameindex;
    bool                framestart;
    int                 durationleft;
} wi_animationstate_t;

typedef struct
{
    interlevel_t        *interlevelexiting;
    interlevel_t        *interlevelentering;
    wi_animationstate_t *exitingstates;
    wi_animationstate_t *enteringstates;
    wi_animationstate_t *states;
    char                *backgroundlump;
} wi_animation_t;

static wi_animation_t   *animation;

static bool CheckConditions(interlevelcond_t *conditions, bool enteringcondition)
{
    bool                conditionsmet = true;
    int                 map = (enteringcondition ? wbs->next : wbs->last) + 1;
    interlevelcond_t    *condition;

    array_foreach(condition, conditions)
    {
        switch (condition->condition)
        {
            case AnimCondition_MapNumGreater:
                conditionsmet &= (map > condition->param);
                break;

            case AnimCondition_MapNumEqual:
                conditionsmet &= (map == condition->param);
                break;

            case AnimCondition_MapVisited:
                conditionsmet &= (map > condition->param);
                break;

            case AnimCondition_MapNotSecret:
                conditionsmet &= !P_IsSecret(1, map);
                break;

            case AnimCondition_SecretVisited:
                conditionsmet &= wbs->didsecret;
                break;

            case AnimCondition_Tally:
                conditionsmet &= !enteringcondition;
                break;

            case AnimCondition_IsEntering:
                conditionsmet &= enteringcondition;
                break;

            default:
                break;
        }
    }

    return conditionsmet;
}

static void UpdateAnimationStates(wi_animationstate_t *animstates)
{
    wi_animationstate_t *animstate;

    array_foreach(animstate, animstates)
    {
        interlevelframe_t   *frame = &animstate->frames[animstate->frameindex];

        if (frame->type & Frame_Infinite)
            continue;

        if (!animstate->durationleft)
        {
            int tics = 1;

            if (!animstate->framestart)
            {
                if (++animstate->frameindex == array_size(animstate->frames))
                    animstate->frameindex = 0;

                frame = &animstate->frames[animstate->frameindex];
            }

            switch (frame->type)
            {
                case Frame_RandomStart:
                    if (animstate->framestart)
                    {
                        tics = M_Random() % frame->duration;
                        break;
                    }

                case Frame_FixedDuration:
                    tics = frame->duration;
                    break;

                case Frame_RandomDuration:
                    tics = MAX(frame->duration, M_Random() % frame->maxduration);
                    break;

                default:
                    break;
            }

            animstate->durationleft = MAX(1, tics);
        }

        animstate->durationleft--;
        animstate->framestart = false;
    }
}

static bool UpdateAnimation(bool enteringcondition)
{
    if (!animation)
        return false;

    animation->states = NULL;
    animation->backgroundlump = NULL;

    if (!enteringcondition)
    {
        if (animation->interlevelexiting)
        {
            animation->states = animation->exitingstates;
            animation->backgroundlump = animation->interlevelexiting->backgroundlump;
        }
    }
    else if (animation->interlevelentering)
    {
        animation->states = animation->enteringstates;
        animation->backgroundlump = animation->interlevelentering->backgroundlump;
    }

    UpdateAnimationStates(animation->states);
    return true;
}

static bool DrawAnimation(void)
{
    wi_animationstate_t *animstate;

    if (!animation)
        return false;

    if (animation->backgroundlump)
        V_DrawPagePatch(0, W_CacheLumpName(animation->backgroundlump));

    array_foreach(animstate, animation->states)
        V_DrawMenuPatch(animstate->xpos, animstate->ypos,
            W_CacheLumpName(animstate->frames[animstate->frameindex].imagelump), false, SCREENWIDTH);

    return true;
}

static wi_animationstate_t *SetupAnimationStates(interlevellayer_t *layers, bool enteringcondition)
{
    wi_animationstate_t *animstates = NULL;
    interlevellayer_t   *layer;

    array_foreach(layer, layers)
    {
        interlevelanim_t    *anim;

        if (!CheckConditions(layer->conditions, enteringcondition))
            continue;

        array_foreach(anim, layer->anims)
        {
            wi_animationstate_t animstate = { 0 };

            if (!CheckConditions(anim->conditions, enteringcondition))
                continue;

            animstate.xpos = anim->xpos;
            animstate.ypos = anim->ypos;
            animstate.frames = anim->frames;
            animstate.framestart = true;
            array_push(animstates, animstate);
        }
    }

    return animstates;
}

static bool SetupAnimation(void)
{
    if (!animation)
        return false;

    if (animation->interlevelexiting)
        animation->exitingstates = SetupAnimationStates(animation->interlevelexiting->layers, false);

    if (animation->interlevelentering)
        animation->enteringstates = SetupAnimationStates(animation->interlevelentering->layers, true);

    return true;
}

static bool NextLocAnimation(void)
{
    return (animation && animation->enteringstates);
}

static bool UpdateMusic(bool enteringcondition)
{
    int musicnum = -1;

    if (!animation)
        return false;

    if (enteringcondition)
    {
        if (animation->interlevelentering)
            musicnum = W_GetNumForName(animation->interlevelentering->musiclump);
    }
    else if (animation->interlevelexiting)
        musicnum = W_GetNumForName(animation->interlevelexiting->musiclump);

    if (musicnum > 0)
    {
        S_ChangeMusInfoMusic(musicnum, true);
        return true;
    }

    return false;
}

// slam background
static void WI_SlamBackground(void)
{
    memcpy(screens[0], screens[1], SCREENAREA);
}

// [BH] Draws character of "<Levelname>"
static void WI_DrawWILVchar(int x, int y, int i)
{
    const int   width = (int)strlen(wilv[i]) / 13;

    for (int y1 = 0; y1 < 13; y1++)
        for (int x1 = 0; x1 < width; x1++)
            V_DrawPixel(x + x1, y + y1, (int)wilv[i][y1 * width + x1], false, true);
}

static char mapname[128];
static char nextmapname[128];

static const int chartoi[130] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 000 to 009
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 010 to 019
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 020 to 029
    -1, -1, -1, 27, -1, -1, -1, -1, -1, 28, // 030 to 039
    -1, -1, -1, -1, -1, 29, 26, 30, 31, 32, // 040 to 049
    33, 34, 35, 36, 37, 38, 39, 40, -1, -1, // 050 to 059
    -1, -1, -1, -1, -1,  0,  1,  2,  3,  4, // 060 to 069
     5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 070 to 079
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 080 to 089
    25, -1, -1, -1, -1, -1, -1,  0,  1,  2, // 090 to 099
     3,  4,  5,  6,  7,  8,  9, 10, 11, 12, // 100 to 109
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, // 110 to 119
    23, 24, 25, -1, -1, -1, -1, -1, -1, -1  // 120 to 129
};

static void WI_DrawWILV(int y, char *str)
{
    const int   len = (int)strlen(str);
    int         width = 0;
    int         x;

    for (int i = 0, j; i < len; i++)
    {
        if (str[i] == '\'' && (!i || str[i - 1] == ' '))
            j = 41;
        else
            j = chartoi[(int)str[i]];

        width += (j == -1 ? 6 : ((int)strlen(wilv[j]) / 13 - 2));
    }

    x = (VANILLAWIDTH - width) / 2 + 1;

    for (int i = 0, j; i < len; i++)
    {
        if (str[i] == '\'' && (!i || str[i - 1] == ' '))
            j = 41;
        else
            j = chartoi[(int)str[i]];

        if (j == -1)
            x += 6;
        else
        {
            WI_DrawWILVchar(x, y, j);
            x += (int)strlen(wilv[j]) / 13 - 2;
        }
    }
}

// Draws "<LevelName> Finished!"
static void WI_DrawLF(void)
{
    int         y = WI_TITLEY;
    const int   titlepatch = P_GetMapTitlePatch(wbs->epsd + 1, wbs->last + 1);

    // draw <LevelName>
    if (titlepatch > 0)
    {
        patch_t     *patch = W_CacheLumpNum(titlepatch);
        const short height = SHORT(patch->height);

        if (height < VANILLAHEIGHT)
            V_DrawMenuPatch((VANILLAWIDTH - SHORT(patch->width)) / 2 + 1, y + 1, patch, false, SCREENWIDTH);
        else
            V_DrawPagePatch(0, patch);

        y += height + 2;
    }
    else
    {
        char    name[9];

        if (gamemode == commercial)
            M_snprintf(name, sizeof(name), "CWILV%02i", wbs->last);
        else
            M_snprintf(name, sizeof(name), "WILV%i%i", wbs->epsd, wbs->last);

        if (W_GetNumLumps(name) > 1 && !nerve)
        {
            patch_t     *patch = lnames[wbs->last];
            const short height = SHORT(patch->height);

            if (height < VANILLAHEIGHT)
                V_DrawMenuPatch((VANILLAWIDTH - SHORT(patch->width)) / 2 + 1, y + 1, patch, false, SCREENWIDTH);
            else
                V_DrawPagePatch(0, patch);

            y += height + 2;
        }
        else
        {
            WI_DrawWILV(y, mapname);
            y += 14;
        }
    }

    if (y >= VANILLAHEIGHT)
        y = WI_TITLEY + 24;

    // draw "Finished!"
    if (SHORT(finished->height) < VANILLAHEIGHT)
        V_DrawMenuPatch((VANILLAWIDTH - SHORT(finished->width)) / 2 + 1, y + 1, finished, false, SCREENWIDTH);
    else
        V_DrawPagePatch(0, finished);
}

// Draws "Entering <LevelName>"
static void WI_DrawEL(void)
{
    int         y = WI_TITLEY;
    const int   titlepatch = P_GetMapTitlePatch(wbs->epsd + 1, wbs->next + 1);

    // draw "Entering"
    if (SHORT(entering->height) < VANILLAHEIGHT)
        V_DrawMenuPatch((VANILLAWIDTH - SHORT(entering->width)) / 2 + 1, y + 1, entering, false, SCREENWIDTH);
    else
        V_DrawPagePatch(0, entering);

    // draw "<LevelName>"
    y += SHORT(entering->height) + 4;

    if (titlepatch > 0)
    {
        patch_t *patch = W_CacheLumpNum(titlepatch);

        if (SHORT(patch->height) < VANILLAHEIGHT)
            V_DrawMenuPatch((VANILLAWIDTH - SHORT(patch->width)) / 2 + 1, y + 1, patch, false, SCREENWIDTH);
        else
            V_DrawPagePatch(0, patch);
    }
    else
    {
        char    name[9];

        if (gamemode == commercial)
            M_snprintf(name, sizeof(name), "CWILV%02i", wbs->next);
        else
            M_snprintf(name, sizeof(name), "WILV%i%i", wbs->epsd, wbs->next);

        if (W_GetNumLumps(name) > 1 && !nerve)
        {
            patch_t     *patch = lnames[wbs->next];
            const short height = SHORT(patch->height);

            if (height < VANILLAHEIGHT)
                V_DrawMenuPatch((VANILLAWIDTH - SHORT(patch->width)) / 2 + 1, y + 1, patch, false, SCREENWIDTH);
            else
                V_DrawPagePatch(0, patch);
        }
        else
            WI_DrawWILV(y, nextmapname);
    }
}

static void WI_DrawOnLnode(int n, patch_t *c[])
{
    bool    fits = false;
    int     i = 0;

    do
    {
        const int   left = lnodes[wbs->epsd][n].x - SHORT(c[i]->leftoffset);
        const int   top = lnodes[wbs->epsd][n].y - SHORT(c[i]->topoffset);
        const int   right = left + SHORT(c[i]->width);
        const int   bottom = top + SHORT(c[i]->height);

        if (left >= 0 && right < VANILLAWIDTH && top >= 0 && bottom < VANILLAHEIGHT)
            fits = true;
        else
            i++;
    } while (!fits && i != 2 && c[i]);

    if (fits && i < 2)
    {
        if (c[i] == splat[0] || c[i] == splat[1])
            V_DrawTranslucentNoGreenPatch(lnodes[wbs->epsd][n].x, lnodes[wbs->epsd][n].y, c[i]);
        else if (c[i] == yah[0] || c[i] == yah[1])
            V_DrawNoGreenPatchWithShadow(lnodes[wbs->epsd][n].x + 1, lnodes[wbs->epsd][n].y + 1, c[i]);
        else
            V_DrawPatch(lnodes[wbs->epsd][n].x, lnodes[wbs->epsd][n].y, 0, c[i]);
    }
}

static void WI_InitAnimatedBack(bool firstcall)
{
    if (SetupAnimation())
        return;

    if (exitpic > 0 || (enterpic > 0 && state != StatCount))
        return;

    if (gamemode == commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (int i = 0; i < numanims[wbs->epsd]; i++)
    {
        anim_t  *a = &anims[wbs->epsd][i];

        // init variables
        // [JN] Do not reset animation timers upon switching to "Entering" state
        // (WI_initShowNextLoc). Fixes notable blinking of Tower of Babel drawing.
        if (firstcall)
            a->ctr = -1;

        // specify the next time to draw it
        if (a->type == ANIM_ALWAYS)
            a->nexttic = bcnt + 1 + M_Random() % a->period;
        else if (a->type == ANIM_LEVEL)
            a->nexttic = bcnt + 1;
    }
}

static void WI_UpdateAnimatedBack(void)
{
    if (UpdateAnimation(state != StatCount))
        return;

    if (exitpic > 0 || (enterpic > 0 && state != StatCount))
        return;

    if (gamemode == commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (int i = 0; i < numanims[wbs->epsd]; i++)
    {
        anim_t  *a = &anims[wbs->epsd][i];

        if (bcnt == a->nexttic)
        {
            switch (a->type)
            {
                case ANIM_ALWAYS:
                    if (++a->ctr >= a->nanims)
                        a->ctr = 0;

                    a->nexttic = bcnt + a->period;
                    break;

                case ANIM_LEVEL:
                    // gawd-awful hack for level anims
                    if (!(state == StatCount && i == 7) && wbs->next == a->data1)
                    {
                        if (++a->ctr == a->nanims)
                            a->ctr--;

                        a->nexttic = bcnt + a->period;
                    }

                    break;
            }
        }
    }
}

static void WI_DrawAnimatedBack(void)
{
    anim_t  *a;

    if (DrawAnimation())
        return;

    if (exitpic > 0 || (enterpic > 0 && state != StatCount))
        return;

    if (gamemode == commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (int i = 0; i < numanims[wbs->epsd]; i++)
    {
        a = &anims[wbs->epsd][i];

        if (a->ctr >= 0)
            V_DrawPatch(a->loc.x, a->loc.y, 0, a->p[a->ctr]);
    }

    // [crispy] show Fortress Of Mystery if it has been completed
    if (wbs->epsd == 1 && wbs->didsecret)
    {
        a = &anims[1][7];
        V_DrawPatch(a->loc.x, a->loc.y, 0, a->p[2]);
    }
}

//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
//
static int WI_DrawNum(int x, int y, int n, int digits)
{
    const int   fontwidth = SHORT(num[0]->width);

    if (digits < 0)
    {
        if (!n)
            // make variable-length zeros 1 digit long
            digits = 1;
        else
        {
            int temp = n;

            // figure out # of digits in #
            digits = 0;

            while (temp)
            {
                temp /= 10;
                digits++;
            }
        }
    }

    // draw the new number
    while (digits--)
    {
        x -= fontwidth;
        x += 2 * (n % 10 == 1);
        V_DrawMenuPatch(x + 1, y + 1, num[n % 10], false, SCREENWIDTH);
        x -= 2 * (n % 10 == 1);
        n /= 10;
    }

    return x;
}

static void WI_DrawPercent(int x, int y, int p)
{
    if (p < 0)
        return;

    V_DrawMenuPatch(x + 1, y + 1, percent, false, SCREENWIDTH);
    WI_DrawNum(x, y, p, -1);
}

//
// Display level completion time and par,
//  or "sucks" message if overflow.
//
static void WI_DrawTime(int x, int y, int t)
{
    if (t < 0)
        return;

    x += (SHORT(num[0]->width) - 11) * 4;

    if (sucktime && t > sucktime * 61 * 59)
        V_DrawMenuPatch(SP_TIMEX + SHORT(timepatch->width) + 12, y + 1, sucks, false, SCREENWIDTH);
    else
    {
        int div = 1;

        if (WICOLONs == 1)
        {
            x = WI_DrawNum(x, y, ((t * 1000 / TICRATE) % 1000) / 10, 2);
            x -= SHORT(period->width);
            V_DrawMenuPatch(x + 1, y + 1, period, false, SCREENWIDTH);
        }

        t /= TICRATE;

        do
        {
            x = WI_DrawNum(x, y, (t / div) % 60, 2) - SHORT(colon->width);
            div *= 60;

            // draw
            if (div == 60 || t / div)
                V_DrawMenuPatch(x + 1, y + 1, colon, false, SCREENWIDTH);
        } while (t / div);

        if (t < 60)
            WI_DrawNum(x, y, 0, 2);
    }
}

static void WI_UnloadData(void);

void WI_End(void)
{
    WI_UnloadData();
}

static void WI_InitNoState(void)
{
    state = NoState;
    acceleratestage = false;
    cnt = (gamemode == commercial || animation ? TICRATE : 10);

    D_FadeScreen(false);
}

static void WI_UpdateNoState(void)
{
    WI_UpdateAnimatedBack();

    if (!--cnt)
        G_WorldDone();
}

static bool snl_pointeron;

static void WI_InitShowNextLoc(void)
{
    UpdateMusic(true);

    if (gamemode != commercial && gamemap == 8)
    {
        G_WorldDone();
        return;
    }

    state = ShowNextLoc;
    acceleratestage = false;
    cnt = SHOWNEXTLOCDELAY * TICRATE;

    D_FadeScreen(false);

    WI_InitAnimatedBack(false);
}

static void WI_UpdateShowNextLoc(void)
{
    WI_UpdateAnimatedBack();

    if (!--cnt || acceleratestage)
        WI_InitNoState();
    else
        snl_pointeron = ((cnt & 31) < 20);
}

static void WI_DrawShowNextLoc(void)
{
    if (P_GetMapEndGame(gameepisode, gamemap)
        || P_GetMapEndCast(gameepisode, gamemap)
        || (legacyofrust && gamemap == 7))
        return;

    WI_SlamBackground();

    // draw animated background
    WI_DrawAnimatedBack();

    if (exitpic > 0 || (enterpic > 0 && state != StatCount))
    {
        WI_DrawEL();
        return;
    }

    if (gamemode != commercial)
    {
        if (wbs->epsd > 2)
        {
            WI_DrawEL();
            return;
        }

        if (!animation || !animation->enteringstates)
        {
            const int   last = (wbs->last == 8 ? wbs->next - 1 : wbs->last);

            // draw a splat on taken cities.
            for (int i = 0; i <= last; i++)
                WI_DrawOnLnode(i, splat);

            // splat the secret level?
            if (wbs->didsecret)
                WI_DrawOnLnode(8, splat);

            // draw flashing ptr
            if (snl_pointeron)
                WI_DrawOnLnode(wbs->next, yah);
        }
    }

    if (gamemission == pack_nerve && wbs->last == 7)
        return;

    // draws which level you are entering...
    if (gamemode != commercial || wbs->next != 30)
        WI_DrawEL();
}

static void WI_DrawNoState(void)
{
    snl_pointeron = true;
    WI_DrawShowNextLoc();
}

static int  sp_state;

static void WI_InitStats(void)
{
    const int   tabs[MAXTABS] = { 85 };
    char        *temp1;
    char        *temp2;

    state = StatCount;
    acceleratestage = false;
    sp_state = 1;
    cnt_kills = -1;
    cnt_items = -1;
    cnt_secret = -1;
    cnt_time = -1;
    cnt_par = -1;
    cnt_pause = TICRATE;

    if (M_StringCompare(maptitle, mapnumandtitle))
        C_PlayerMessage("%s finished %s!",
            (M_StringCompare(playername, playername_default) ? "You" : playername), mapname);
    else
        C_PlayerMessage("%s finished " ITALICS("%s%s"),
            (M_StringCompare(playername, playername_default) ? "you" : playername), mapname,
            (ispunctuation(mapname[strlen(mapname) - 1]) ? "" : "!"));

    temp1 = commify(wbs->skills);
    temp2 = commify(wbs->maxkills);
    C_TabbedOutput(tabs, "Kills\t%s of %s (%i%%)",
        temp1, temp2, (wbs->skills * 100) / wbs->maxkills);
    free(temp1);
    free(temp2);

    temp1 = commify(wbs->sitems);
    temp2 = commify(wbs->maxitems);
    C_TabbedOutput(tabs, "Items\t%s of %s (%i%%)",
        temp1, temp2, (wbs->sitems * 100) / wbs->maxitems);
    free(temp1);
    free(temp2);

    if (totalsecrets)
    {
        temp1 = commify(wbs->ssecret);
        temp2 = commify(wbs->maxsecret);
        C_TabbedOutput(tabs, "Secrets\t%s of %s (%i%%)",
            temp1, temp2, (wbs->ssecret * 100) / wbs->maxsecret);
        free(temp1);
        free(temp2);
    }

    if (sucktime && wbs->stime / TICRATE > sucktime * 61 * 59)
        C_TabbedOutput(tabs, "Time\t%s", s_STSTR_SUCKS);
    else
        C_TabbedOutput(tabs, "Time\t" MONOSPACED("%02i") ":" MONOSPACED("%02i") "." MONOSPACED("%02i"),
            wbs->stime / TICRATE / 60, wbs->stime / TICRATE % 60, ((wbs->stime * 1000 / TICRATE) % 1000) / 10);

    if (wbs->partime)
        C_TabbedOutput(tabs, "Par time\t" MONOSPACED("%02i") ":" MONOSPACED("%02i") "." MONOSPACED("%02i"),
            wbs->partime / TICRATE / 60, wbs->partime / TICRATE % 60,
            ((wbs->partime * 1000 / TICRATE) % 1000) / 10);

    if (totaltime > maptime)
    {
        int         tics = totaltime / TICRATE;
        const int   milliseconds = (tics * 1000 / TICRATE) % 1000;
        const int   hours = tics / 3600;
        const int   minutes = ((tics %= 3600)) / 60;
        const int   seconds = tics % 60;

        if (hours)
            C_TabbedOutput(tabs, "Total time\t" MONOSPACED("%i") ":" MONOSPACED("%02i")
                ":" MONOSPACED("%02i") "." MONOSPACED("%02i"),
                hours, minutes, seconds, milliseconds / 10);
        else
            C_TabbedOutput(tabs, "Total time\t" MONOSPACED("%02i") ":" MONOSPACED("%02i")
                "." MONOSPACED("%02i"),
                minutes, seconds, milliseconds / 10);
    }

    WI_InitAnimatedBack(true);
}

static void WI_UpdateStats(void)
{
    // e6y
    static bool play_early_explosion = true;

    WI_UpdateAnimatedBack();

    if (acceleratestage && sp_state != 10)
    {
        acceleratestage = false;
        cnt_kills = (wbs->skills * 100) / wbs->maxkills;
        cnt_items = (wbs->sitems * 100) / wbs->maxitems;
        cnt_secret = (wbs->ssecret * 100) / wbs->maxsecret;
        cnt_time = wbs->stime;
        cnt_par = wbs->partime;
        S_StartSound(NULL, sfx_barexp);
        sp_state = 10;
    }

    if (sp_state == 2)
    {
        cnt_kills += 2;

        if (!(bcnt & 3))
            S_StartSound(NULL, sfx_pistol);

        if (cnt_kills >= (wbs->skills * 100) / wbs->maxkills)
        {
            cnt_kills = (wbs->skills * 100) / wbs->maxkills;
            S_StartSound(NULL, sfx_barexp);
            sp_state++;
        }
    }
    else if (sp_state == 4)
    {
        cnt_items += 2;

        if (!(bcnt & 3))
            S_StartSound(NULL, sfx_pistol);

        if (cnt_items >= (wbs->sitems * 100) / wbs->maxitems)
        {
            cnt_items = (wbs->sitems * 100) / wbs->maxitems;
            S_StartSound(NULL, sfx_barexp);
            sp_state++;
        }
    }
    else if (sp_state == 6)
    {
        // [JN] If no secrets on the map, skip counting immediately.
        if (!totalsecrets)
            sp_state += 2;
        else
        {
            cnt_secret += 2;

            if (!(bcnt & 3))
                S_StartSound(NULL, sfx_pistol);

            if (cnt_secret >= (wbs->ssecret * 100) / wbs->maxsecret)
            {
                cnt_secret = (wbs->ssecret * 100) / wbs->maxsecret;

                if (totalsecrets)
                    S_StartSound(NULL, sfx_barexp);

                sp_state++;
            }
        }
    }
    else if (sp_state == 8)
    {
        // e6y: do not play count sound after explosion sound
        if (!(bcnt & 3) && play_early_explosion)
            S_StartSound(NULL, sfx_pistol);

        cnt_time += 3 * TICRATE;

        if (cnt_time > wbs->stime)
            cnt_time = wbs->stime;

        cnt_par += 3 * TICRATE;

        // e6y
        // if par time is hidden (if modifiedgame is true)
        // the game should play explosion sound immediately after
        // the counter will reach level time instead of par time
        if (modifiedgame && play_early_explosion)
            if (cnt_time >= wbs->stime)
            {
                S_StartSound(NULL, sfx_barexp);
                play_early_explosion = false;   // do not play it twice or more
            }

        if (cnt_par >= wbs->partime)
        {
            cnt_par = wbs->partime;

            if (cnt_time >= wbs->stime)
            {
                // e6y: do not play explosion sound if it was already played
                if (!modifiedgame)
                    S_StartSound(NULL, sfx_barexp);

                sp_state++;
            }
        }
    }
    else if (sp_state == 10)
    {
        if (acceleratestage)
        {
            S_StartSound(NULL, sfx_sgcock);

            if (NextLocAnimation() || gamemode != commercial)
                WI_InitShowNextLoc();
            else
                WI_InitNoState();
        }
    }
    else if (sp_state & 1)
    {
        play_early_explosion = true;    // e6y

        if (!--cnt_pause)
        {
            sp_state++;
            cnt_pause = TICRATE;
        }
    }
}

static void WI_DrawStats(void)
{
    // line height
    const int   lh = 3 * SHORT(num[0]->height) / 2;

    WI_SlamBackground();

    // draw animated background
    WI_DrawAnimatedBack();

    WI_DrawLF();

    V_DrawMenuPatch(SP_STATSX, SP_STATSY + 1, kills, false, SCREENWIDTH);
    WI_DrawPercent(VANILLAWIDTH - SP_STATSX - 14, SP_STATSY, cnt_kills);

    V_DrawMenuPatch(SP_STATSX, SP_STATSY + lh + 1, items, false, SCREENWIDTH);
    WI_DrawPercent(VANILLAWIDTH - SP_STATSX - 14, SP_STATSY + lh, cnt_items);

    if (totalsecrets)
    {
        if (!WISCRT2)
            M_DrawString(SP_STATSX, SP_STATSY + 2 * lh - 2, "secrets", false, true);
        else
            V_DrawMenuPatch(SP_STATSX, SP_STATSY + 2 * lh + 1, sp_secret, false, SCREENWIDTH);

        WI_DrawPercent(VANILLAWIDTH - SP_STATSX - 14, SP_STATSY + 2 * lh, cnt_secret);
    }

    V_DrawMenuPatch(SP_TIMEX + 1, SP_TIMEY + 1, timepatch, false, SCREENWIDTH);
    WI_DrawTime(VANILLAWIDTH / 2 - SP_TIMEX * 2 + (wbs->stime >= TICRATE * 60 * 60) * 16
        + (WICOLONs == 1) * 19, SP_TIMEY, cnt_time);

    if (wbs->partime)
    {
        V_DrawMenuPatch(VANILLAWIDTH / 2 + SP_TIMEX + !BTSX * (SP_TIMEX - FREEDOOM * 17 + 3)
            - (WICOLONs == 1) * 19, SP_TIMEY + 1, par, false, SCREENWIDTH);
        WI_DrawTime(VANILLAWIDTH - SP_TIMEX - 2 - (BTSX || FREEDOOM) * 17, SP_TIMEY, cnt_par);
    }
}

void WI_CheckForAccelerate(void)
{
    if ((viewplayer->cmd.buttons & BT_ATTACK) || gamekeydown[KEY_ENTER])
    {
        if (!viewplayer->attackdown)
            acceleratestage = true;

        viewplayer->attackdown = true;
    }
    else
        viewplayer->attackdown = false;

    if ((viewplayer->cmd.buttons & BT_USE) || gamekeydown[KEY_SPACE])
    {
        if (!viewplayer->usedown)
            acceleratestage = true;

        viewplayer->usedown = true;
    }
    else
        viewplayer->usedown = false;
}

static void WI_LoadData(void);

// Updates stuff each tic
void WI_Ticker(void)
{
    // counter for general background animation
    if (++bcnt == 1 && !UpdateMusic(false))
    {
        // intermission music
        S_ChangeMusic((gamemode == commercial ? mus_dm2int : mus_inter), true, false, false);
        S_StopSounds();
    }

    WI_LoadData();

    if (!menuactive && !consoleactive && !paused && windowfocused)
    {
        WI_CheckForAccelerate();

        switch (state)
        {
            case StatCount:
                WI_UpdateStats();
                break;

            case ShowNextLoc:
                WI_UpdateShowNextLoc();
                break;

            case NoState:
                WI_UpdateNoState();
                break;
        }
    }
}

typedef void (*load_callback_t)(const char *, patch_t **);

// Common load/unload function. Iterates over all the graphics
// lumps to be loaded/unloaded into memory.
static void WI_LoadUnloadData(load_callback_t callback)
{
    char    name[9];

    if (gamemode == commercial)
    {
        for (int i = 0; i < numcmaps; i++)
        {
            M_snprintf(name, sizeof(name), "CWILV%02i", i);
            callback(name, &lnames[i]);
        }
    }
    else
    {
        for (int i = 0; i < NUMMAPS; i++)
        {
            M_snprintf(name, sizeof(name), "WILV%i%i", wbs->epsd, i);
            callback(name, &lnames[i]);
        }

        // you are here
        callback("WIURH0", &yah[0]);

        // you are here (alt.)
        callback("WIURH1", &yah[1]);

        // splat
        callback("WISPLAT", &splat[0]);

        if (wbs->epsd < 3)
            for (int j = 0; j < numanims[wbs->epsd]; j++)
            {
                anim_t  *a = &anims[wbs->epsd][j];

                for (int i = 0; i < a->nanims; i++)
                {
                    // MONDO HACK!
                    if (wbs->epsd != 1 || j != 8)
                    {
                        // animations
                        M_snprintf(name, sizeof(name), "WIA%i%02i%02i", wbs->epsd, j, i);
                        callback(name, &a->p[i]);
                    }
                    else
                        // HACK ALERT!
                        a->p[i] = anims[1][4].p[i];
                }
            }
    }

    for (int i = 0; i <= 9; i++)
    {
        // numbers 0-9
        M_snprintf(name, sizeof(name), "WINUM%i", i);
        callback(name, &num[i]);
    }

    // percent sign
    callback("WIPCNT", &percent);

    // "finished"
    callback("WIF", &finished);

    // "entering"
    callback("WIENTER", &entering);

    // "kills"
    callback("WIOSTK", &kills);

    // "secret"
    callback("WISCRT2", &sp_secret);

    // "items"
    callback("WIOSTI", &items);

    // ":"
    callback("WICOLON", &colon);

    // "."
    callback("WIPERIOD", &period);

    // "time"
    callback("WITIME", &timepatch);

    // "sucks"
    callback("WISUCKS", &sucks);

    // "par"
    callback("WIPAR", &par);
}

static void WI_LoadCallback(const char *name, patch_t **variable)
{
    *variable = (patch_t *)W_CacheLumpName(name);
}

static void WI_LoadData(void)
{
    patch_t *lump;

    if (gamemode == commercial)
    {
        numcmaps = 32 + (W_CheckNumForName("CWILV32") >= 0);
        lnames = Z_Malloc(numcmaps * sizeof(patch_t *), PU_STATIC, NULL);
    }
    else
        lnames = Z_Malloc(NUMMAPS * sizeof(patch_t *), PU_STATIC, NULL);

    WI_LoadUnloadData(&WI_LoadCallback);

    // Background image
    if (enterpic > 0 && state != StatCount)
        lump = W_CacheLumpNum(enterpic);
    else if (exitpic > 0)
        lump = W_CacheLumpNum(exitpic);
    else if (FREEDOOM || hacx)
        lump = W_CacheLastLumpName("INTERPIC");
    else if (gamemode == commercial)
    {
        if (gamemission == pack_plut)
        {
            if (W_GetNumLumps("INTERPIC") > 2)
                lump = W_CacheLumpName("INTERPIC");
            else
                lump = W_CacheLumpName("INTERPI2");
        }
        else if (gamemission == pack_tnt)
        {
            if (W_GetNumLumps("INTERPIC") > 2)
                lump = W_CacheLumpName("INTERPIC");
            else
                lump = W_CacheLumpName("INTERPI3");
        }
        else if (nerve)
            lump = W_CacheLumpNameFromResourceWAD("INTERPIC");
        else
            lump = W_CacheLumpName("INTERPIC");
    }
    else if (gamemode == retail && wbs->epsd == 3)
    {
        if (REKKRSL)
            lump = W_CacheLumpName("INTERPIW");
        else if (W_GetNumLumps("INTERPIC") > 2)
            lump = W_CacheLumpName("INTERPIC");
        else
            lump = W_CacheLumpName("INTERPI1");
    }
    else if (sigil && wbs->epsd == 4)
        lump = W_CacheLumpNameFromResourceWAD("SIGILINT");
    else if (sigil2 && wbs->epsd == 5)
        lump = W_CacheLumpNameFromResourceWAD("SIGILIN2");
    else if (wbs->epsd <= 2)
    {
        char    temp[9];

        if (REKKR)
        {
            M_snprintf(temp, sizeof(temp), "WIMAP%iW", wbs->epsd);

            if (W_CheckNumForName(temp) < 0)
                M_snprintf(temp, sizeof(temp), "WIMAP%i", wbs->epsd);
        }
        else
            M_snprintf(temp, sizeof(temp), "WIMAP%i", wbs->epsd);

        lump = (chex ? W_CacheLastLumpName(temp) : W_CacheLumpName(temp));
    }
    else
        lump = W_CacheLumpName("INTERPIC");

    V_DrawPagePatch(1, lump);
}

static void WI_UnloadCallback(const char *name, patch_t **variable)
{
    W_ReleaseLumpName(name);
    *variable = NULL;
}

static void WI_UnloadData(void)
{
    WI_LoadUnloadData(&WI_UnloadCallback);
}

void WI_Drawer(void)
{
    switch (state)
    {
        case StatCount:
            WI_DrawStats();
            break;

        case ShowNextLoc:
            WI_DrawShowNextLoc();
            break;

        case NoState:
            WI_DrawNoState();
            break;
    }
}

static void WI_InitVariables(wbstartstruct_t *wbstartstruct)
{
    char    *temp = titlecase(maptitle);

    wbs = wbstartstruct;

    enterpic = P_GetMapEnterPic(wbs->epsd + 1, wbs->next + 1);
    exitpic = P_GetMapExitPic(gameepisode, gamemap);

    acceleratestage = false;
    cnt = 0;
    bcnt = 0;

    if (!wbs->maxkills)
        wbs->maxkills = 1;

    if (!wbs->maxitems)
        wbs->maxitems = 1;

    if (!wbs->maxsecret)
        wbs->maxsecret = 1;

    if (gamemode != retail && wbs->epsd > 2)
        wbs->epsd -= 3;

    M_StringCopy(mapname, temp, sizeof(mapname));
    free(temp);

    P_MapName(wbs->epsd + 1, wbs->next + 1);
    M_StringCopy(nextmapname, maptitle, sizeof(nextmapname));
}

void WI_Start(wbstartstruct_t *wbstartstruct)
{
    char    *enteranim = P_GetMapEnterAnim(gameepisode, gamemap);
    char    *exitanim = P_GetMapExitAnim(gameepisode, gamemap);

    WI_InitVariables(wbstartstruct);

    if (enteranim[0])
    {
        if (!animation)
            animation = calloc(1, sizeof(*animation));

        animation->interlevelentering = WI_ParseInterlevel(enteranim);
    }

    if (exitanim[0])
    {
        if (!animation)
            animation = calloc(1, sizeof(*animation));

        animation->interlevelexiting = WI_ParseInterlevel(exitanim);
    }

    WI_InitStats();

#if SDL_VERSION_ATLEAST(2, 24, 0)
    SDL_ResetKeyboard();
#endif
}
