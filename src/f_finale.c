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

#include <ctype.h>

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "hu_stuff.h"
#include "i_gamepad.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "SDL.h"
#include "v_data.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

typedef enum
{
    F_STAGE_TEXT,
    F_STAGE_ARTSCREEN,
    F_STAGE_CAST
} finalestage_t;

// Stage of animation:
static finalestage_t    finalestage;
static int              finalecount;

#define TEXTSPEED       (3 * FRACUNIT)          // original value       // phares
#define TEXTWAIT        (250 * FRACUNIT)        // original value       // phares
#define NEWTEXTSPEED    ((FRACUNIT + 50) / 100) // new value            // phares
#define NEWTEXTWAIT     (1000 * FRACUNIT)       // new value            // phares

static char     *finaletext;
static char     *finaleflat;

void F_StartCast(void);
void F_CastTicker(void);
dboolean F_CastResponder(event_t *ev);
void F_CastDrawer(void);

void WI_checkForAccelerate(void);    // killough 3/28/98: used to
extern int acceleratestage;          // accelerate intermission screens
static int midstage;                 // whether we're in "mid-stage"

extern dboolean r_shadows;

//
// F_StartFinale
//
void F_StartFinale(void)
{
    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    viewactive = false;
    automapactive = false;

    // killough 3/28/98: clear accelerative text flags
    acceleratestage = midstage = 0;

    // Okay - IWAD dependend stuff.
    // This has been changed severly, and
    //  some stuff might have changed in the process.
    switch (gamemode)
    {
        // DOOM 1 - E1, E3 or E4, but each nine missions
        case shareware:
        case registered:
        case retail:
        {
            S_ChangeMusic(mus_victor, true, false);

            switch (gameepisode)
            {
                case 1:
                    finaleflat = bgflatE1;
                    finaletext = s_E1TEXT;
                    break;
                case 2:
                    finaleflat = bgflatE2;
                    finaletext = s_E2TEXT;
                    break;
                case 3:
                    finaleflat = bgflatE3;
                    finaletext = s_E3TEXT;
                    break;
                case 4:
                    finaleflat = bgflatE4;
                    finaletext = s_E4TEXT;
                    break;
                default:
                    break;
            }
            break;
        }

        // DOOM II and missions packs with E1, M34
        case commercial:
        {
            S_ChangeMusic(mus_read_m, true, false);

            switch (gamemap)      // This is regular Doom II
            {
                case 6:
                    finaleflat = bgflat06;
                    finaletext = (gamemission == pack_tnt ? s_T1TEXT :
                        (gamemission == pack_plut ? s_P1TEXT : s_C1TEXT));
                    break;
                case 8:
                    if (gamemission == pack_nerve)
                    {
                        finaleflat = bgflat06;
                        finaletext = s_N1TEXT;
                    }
                    break;
                case 11:
                    finaleflat = bgflat11;
                    finaletext = (gamemission == pack_tnt ? s_T2TEXT :
                        (gamemission == pack_plut ? s_P2TEXT : s_C2TEXT));
                    break;
                case 20:
                    finaleflat = bgflat20;
                    finaletext = (gamemission == pack_tnt ? s_T3TEXT :
                        (gamemission == pack_plut ? s_P3TEXT : s_C3TEXT));
                    break;
                case 30:
                    finaleflat = bgflat30;
                    finaletext = (gamemission == pack_tnt ? s_T4TEXT :
                        (gamemission == pack_plut ? s_P4TEXT : s_C4TEXT));
                    break;
                case 15:
                    finaleflat = bgflat15;
                    finaletext = (gamemission == pack_tnt ? s_T5TEXT :
                        (gamemission == pack_plut ? s_P5TEXT : s_C5TEXT));
                    break;
                case 31:
                    finaleflat = bgflat31;
                    finaletext = (gamemission == pack_tnt ? s_T6TEXT :
                        (gamemission == pack_plut ? s_P6TEXT : s_C6TEXT));
                    break;
                default:
                    // Ouch.
                    break;
            }
            break;
        }

        // Indeterminate.
        default:
            S_ChangeMusic(mus_read_m, true, false);
            finaleflat = "F_SKY1";
            finaletext = s_C1TEXT;
            break;
    }

    finalestage = F_STAGE_TEXT;
    finalecount = 0;
}

dboolean F_Responder(event_t *ev)
{
    if (finalestage == F_STAGE_CAST)
        return F_CastResponder(ev);

    return false;
}

static fixed_t TextSpeed(void)
{
    return (midstage ? NEWTEXTSPEED : (midstage = acceleratestage) ?
            acceleratestage = 0, NEWTEXTSPEED : TEXTSPEED);
}

//
// F_Ticker
//
void F_Ticker(void)
{
    if (menuactive || paused || consoleactive)
        return;

    WI_checkForAccelerate();

    // advance animation
    finalecount++;

    if (finalestage == F_STAGE_CAST)
        F_CastTicker();

    if (finalestage == F_STAGE_TEXT)
    {
        if (finalecount > FixedMul(strlen(finaletext) * FRACUNIT, TextSpeed())
            + (midstage ? NEWTEXTWAIT : TEXTWAIT) || (midstage && acceleratestage))
        {
            if (gamemode != commercial)
            {
                finalecount = 0;
                finalestage = 1;
                wipegamestate = (gamestate_t)(-1);      // force a wipe
                if (gameepisode == 3)
                    S_StartMusic(mus_bunny);
            }
            else if (midstage)
            {
                if (gamemap == 30 || (gamemission == pack_nerve && gamemap == 8))
                    F_StartCast();
                else
                    gameaction = ga_worlddone;
            }
        }
    }
}

static struct
{
    char        char1;
    char        char2;
    int         adjust;
} kern[] = {
    { '.', '1',  -1 },
    { '.', '7',  -1 },
    { '.', '\"', -1 },
    { ',', '1',  -1 },
    { ',', '7',  -1 },
    { ',', 'Y',  -1 },
    { 'T', '.',  -1 },
    { 'T', ',',  -1 },
    { 'Y', '.',  -1 },
    { 'Y', ',',  -1 },
    { 'D', '\'', -1 },
    { 0,   0,     0 }
};

//
//
// F_TextWrite
//
extern patch_t *hu_font[HU_FONTSIZE];

void M_DrawSmallChar(int x, int y, int i, dboolean shadow);

void F_TextWrite(void)
{
    // draw some of the text onto the screen
    byte        *src;
    byte        *dest;
    int         x, y, w;
    int         count = FixedDiv(((finalecount - 10) * FRACUNIT), TextSpeed()) >> FRACBITS;
    const char  *ch = finaletext;
    int         cx = 12;
    int         cy = 10;
    int         i;
    char        letter;
    char        prev = ' ';

    // erase the entire screen to a tiled background
    src = (byte *)W_CacheLumpName((char *)finaleflat, PU_CACHE);
    dest = screens[0];

    for (y = 0; y < SCREENHEIGHT; y += 2)
        for (x = 0; x < SCREENWIDTH / 32; x += 2)
        {
            for (i = 0; i < 64; i++)
            {
                int     j = i * 2;
                byte    dot = *(src + (((y / 2) & 63) << 6) + i);

                if (y * SCREENWIDTH + x + j < SCREENWIDTH * (SCREENHEIGHT - 1))
                    *(dest + j) = dot;
                j++;
                if (y * SCREENWIDTH + x + j < SCREENWIDTH * (SCREENHEIGHT - 1))
                    *(dest + j) = dot;
                j += SCREENWIDTH;
                if (y * SCREENWIDTH + x + j < SCREENWIDTH * (SCREENHEIGHT - 1))
                    *(dest + j) = dot;
                j--;
                if (y * SCREENWIDTH + x + j < SCREENWIDTH * (SCREENHEIGHT - 1))
                    *(dest + j) = dot;
            }
            dest += 128;
        }

    if (count < 0)
        count = 0;

    for (; count; count--)
    {
        char    c = *ch++;

        if (!c)
            break;
        if (c == '\n')
        {
            cx = 12;
            cy += (prev == '\n' ? 8 : 11);
            prev = c;
            continue;
        }

        letter = c;
        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
        {
            cx += (prev == '.' || prev == '!' || prev == '?' || prev == '\"' ? 5 : 3);
            prev = letter;
            continue;
        }

        if (STCFN034)
        {
            w = SHORT(hu_font[c]->width);
            V_DrawPatchWithShadow(cx + 1, cy + 1, hu_font[c], false);
        }
        else
        {
            int k = 0;

            if (prev == ' ')
            {
                if (letter == '\"')
                    c = 64;
                else if (letter == '\'')
                    c = 65;
            }
            while (kern[k].char1)
            {
                if (prev == kern[k].char1 && c == kern[k].char2)
                {
                    cx += kern[k].adjust;
                    break;
                }
                k++;
            }
            w = strlen(smallcharset[c]) / 10 - 1;
            M_DrawSmallChar(cx + 1, cy + 1, c, true);
        }
        prev = letter;
        cx += w;
    }
}

//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
typedef struct
{
    char        **name;
    mobjtype_t  type;
} castinfo_t;

static castinfo_t castorder[] =
{
    { &s_CC_ZOMBIE,  MT_POSSESSED },
    { &s_CC_SHOTGUN, MT_SHOTGUY   },
    { &s_CC_HEAVY,   MT_CHAINGUY  },
    { &s_CC_IMP,     MT_TROOP     },
    { &s_CC_DEMON,   MT_SERGEANT  },
    { &s_CC_SPECTRE, MT_SHADOWS   },
    { &s_CC_LOST,    MT_SKULL     },
    { &s_CC_CACO,    MT_HEAD      },
    { &s_CC_HELL,    MT_KNIGHT    },
    { &s_CC_BARON,   MT_BRUISER   },
    { &s_CC_ARACH,   MT_BABY      },
    { &s_CC_PAIN,    MT_PAIN      },
    { &s_CC_REVEN,   MT_UNDEAD    },
    { &s_CC_MANCU,   MT_FATSO     },
    { &s_CC_ARCH,    MT_VILE      },
    { &s_CC_SPIDER,  MT_SPIDER    },
    { &s_CC_CYBER,   MT_CYBORG    },
    { &s_CC_HERO,    MT_PLAYER    },
    { NULL,          0            }
};

int     castnum;
int     casttics;
state_t *caststate;
int     castrot;
dboolean castdeath;
dboolean castdeathflip;
int     castframes;
int     castonmelee;
dboolean castattacking;
dboolean firstevent;

extern char *playername;

//
// F_StartCast
//
void F_StartCast(void)
{
    firstevent = true;
    wipegamestate = (gamestate_t)(-1);  // force a screen wipe
    castnum = 0;
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    casttics = caststate->tics;
    castrot = 0;
    castdeath = false;
    castdeathflip = false;
    finalestage = F_STAGE_CAST;
    castframes = 0;
    castonmelee = 0;
    castattacking = false;
    if (strcasecmp(playername, playername_default))
        s_CC_HERO = playername;
    S_ChangeMusic(mus_evil, true, false);
}

//
// F_CastTicker
//
void F_CastTicker(void)
{
    int st;
    int sfx;

    if (--casttics > 0)
        return;                         // not time to change state yet

    if (caststate->tics == -1 || caststate->nextstate == S_NULL)
    {
        // switch from deathstate to next monster
        castnum++;
        castdeath = false;
        castdeathflip = false;
        if (!castorder[castnum].name)
            castnum = 0;
        if (mobjinfo[castorder[castnum].type].seesound)
            S_StartSound(NULL, mobjinfo[castorder[castnum].type].seesound);
        caststate = &states[mobjinfo[castorder[castnum].type].seestate];
        castframes = 0;
    }
    else
    {
        // just advance to next state in animation
        if (caststate == &states[S_PLAY_ATK1])
            goto stopattack;            // Oh, gross hack!
        st = caststate->nextstate;
        caststate = &states[st];
        castframes++;

        // sound hacks....
        switch (st)
        {
            case S_PLAY_ATK1:
                sfx = sfx_dshtgn;
                break;
            case S_POSS_ATK2:
                sfx = sfx_pistol;
                break;
            case S_SPOS_ATK2:
                sfx = sfx_shotgn;
                break;
            case S_VILE_ATK2:
                sfx = sfx_vilatk;
                break;
            case S_SKEL_FIST2:
                sfx = sfx_skeswg;
                break;
            case S_SKEL_FIST4:
                sfx = sfx_skepch;
                break;
            case S_SKEL_MISS2:
                sfx = sfx_skeatk;
                break;
            case S_FATT_ATK8:
            case S_FATT_ATK5:
            case S_FATT_ATK2:
                sfx = sfx_firsht;
                break;
            case S_CPOS_ATK2:
            case S_CPOS_ATK3:
            case S_CPOS_ATK4:
                sfx = sfx_shotgn;
                break;
            case S_TROO_ATK3:
                sfx = sfx_claw;
                break;
            case S_SARG_ATK2:
                sfx = sfx_sgtatk;
                break;
            case S_BOSS_ATK2:
            case S_BOS2_ATK2:
            case S_HEAD_ATK2:
                sfx = sfx_firsht;
                break;
            case S_SKULL_ATK2:
                sfx = sfx_sklatk;
                break;
            case S_SPID_ATK2:
            case S_SPID_ATK3:
                sfx = sfx_shotgn;
                break;
            case S_BSPI_ATK2:
                sfx = sfx_plasma;
                break;
            case S_CYBER_ATK2:
            case S_CYBER_ATK4:
            case S_CYBER_ATK6:
                sfx = sfx_rlaunc;
                break;
            case S_PAIN_ATK3:
                sfx = sfx_sklatk;
                break;
            default:
                sfx = 0;
                break;
        }

        if (sfx)
            S_StartSound(NULL, sfx);
    }

    if (castframes == 12)
    {
        // go into attack frame
        castattacking = true;
        if (castonmelee)
            caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
        else
            caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
        castonmelee ^= 1;
        if (caststate == &states[S_NULL])
        {
            if (castonmelee)
                caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
            else
                caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
        }
        if (caststate == &states[S_PLAY_ATK1])
            S_StartSound (NULL, sfx_dshtgn);
    }

    if (castattacking)
    {
        if (castframes == 24 || caststate == &states[mobjinfo[castorder[castnum].type].seestate])
        {
stopattack:
            castattacking = false;
            castframes = 0;
            caststate = &states[mobjinfo[castorder[castnum].type].seestate];
        }
    }

    casttics = caststate->tics;
    if (casttics == -1)
        casttics = 15;
}

//
// F_CastResponder
//

extern int key_use;
extern int key_fire;

dboolean F_CastResponder(event_t *ev)
{
    mobjtype_t  type;

    if (!ev->data1)
        return false;

    if (ev->type == ev_mouse && (ev->data1 & MOUSE_LEFTBUTTON))
        firstevent = false;
    else if (firstevent)
    {
        firstevent = false;
        return true;
    }

    if (menuactive || paused || consoleactive)
        return false;

    if (ev->type == ev_keydown && ev->data1 != key_use && ev->data1 != key_fire
        && ev->data1 != KEY_LEFTARROW && ev->data1 != KEY_RIGHTARROW && ev->data1 != KEY_ENTER)
        return false;

    if (ev->type == ev_keyup)
        return false;

    if (ev->type == ev_mouse && !(ev->data1 & MOUSE_LEFTBUTTON))
        return false;

    if (ev->type == ev_gamepad && !(ev->data1 & GAMEPAD_RIGHT_TRIGGER) && !(ev->data1 & GAMEPAD_A))
        return false;

    if (castdeath)
        return true;                    // already in dying frames
    else
    {
        // rotate (taken from Eternity Engine)
        if (ev->data1 == KEY_LEFTARROW)
        {
            if (castrot == 14)
                castrot = 0;
            else
                castrot += 2;
            return true;
        }
        if (ev->data1 == KEY_RIGHTARROW)
        {
            if (castrot == 0)
                castrot = 14;
            else
                castrot -= 2;
            return true;
        }
    }

    S_StartSound(players[0].mo, sfx_dshtgn);

    type = castorder[castnum].type;
    
    // go into death frame
    castdeath = true;
    if (r_corpses_mirrored && type != MT_CHAINGUY && type != MT_CYBORG)
        castdeathflip = rand() & 1;
    caststate = &states[mobjinfo[type].deathstate];
    casttics = caststate->tics;
    castrot = 0;
    castframes = 0;
    castattacking = false;
    if (mobjinfo[type].deathsound)
        S_StartSound(NULL, mobjinfo[type].deathsound);

    return true;
}

void F_CastPrint(char *text)
{
    const char  *ch;
    int         c;
    int         cx;
    int         w;
    int         width;

    // find width
    ch = text;
    width = 0;

    while (ch)
    {
        c = *ch++;
        if (!c)
            break;
        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c > HU_FONTSIZE)
        {
            width += 4;
            continue;
        }

        w = SHORT(hu_font[c]->width);
        width += w;
    }

    // draw it
    cx = (ORIGINALWIDTH - width) / 2;
    ch = text;
    while (ch)
    {
        c = *ch++;
        if (!c)
            break;
        c = toupper(c) - HU_FONTSTART;
        if (c < 0 || c > HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT(hu_font[c]->width);
        V_DrawPatchWithShadow(cx + 1, 181, hu_font[c], false);
        cx += w;
    }
}

//
// F_CastDrawer
//
extern dboolean r_translucency;

void F_CastDrawer(void)
{
    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;
    int                 rot = 0;
    dboolean            flip;
    patch_t             *patch;
    int                 y = ORIGINALHEIGHT - 30;
    mobjtype_t          type = castorder[castnum].type;

    // erase the entire screen to a background
    V_DrawPatch(0, 0, 0, W_CacheLumpName(bgcastcall, PU_CACHE));

    F_CastPrint(*(castorder[castnum].name));

    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[caststate->frame & FF_FRAMEMASK];
    if (sprframe->rotate)
        rot = castrot;
    lump = sprframe->lump[rot];
    flip = (dboolean)(sprframe->flip & (1 << rot));

    patch = W_CacheLumpNum(lump + firstspritelump, PU_CACHE);

    patch->topoffset = spritetopoffset[lump] >> FRACBITS;

    if (type == MT_SKULL)
        y -= 30;
    else if (type == MT_PAIN || (type == MT_HEAD && !castdeath))
        y -= 20;

    if (flip || castdeathflip)
    {
        patch->leftoffset = (spritewidth[lump] - spriteoffset[lump]) >> FRACBITS;

        if (r_shadows && ((type != MT_SKULL && type != MT_PAIN) || !castdeath))
        {
            if (r_translucency)
            {
                if (type == MT_SHADOWS)
                    V_DrawFlippedSpectreShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
                else
                    V_DrawFlippedShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
            }
            else
                V_DrawFlippedSolidShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
        }

        if (r_translucency && (type == MT_SKULL || (type == MT_PAIN && castdeath)))
            V_DrawFlippedTranslucentRedPatch(ORIGINALWIDTH / 2, y, patch);
        else if (type == MT_SHADOWS)
            V_DrawFlippedFuzzPatch(ORIGINALWIDTH / 2, y, patch);
        else
            V_DrawFlippedPatch(ORIGINALWIDTH / 2, y, patch);
    }
    else
    {
        patch->leftoffset = spriteoffset[lump] >> FRACBITS;

        if (r_shadows && ((type != MT_SKULL && type != MT_PAIN) || !castdeath))
        {
            if (r_translucency)
            {
                if (type == MT_SHADOWS)
                    V_DrawSpectreShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
                else
                    V_DrawShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
            }
            else
                V_DrawSolidShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
        }

        if (r_translucency && (type == MT_SKULL || (type == MT_PAIN && castdeath)))
            V_DrawTranslucentRedPatch(ORIGINALWIDTH / 2, y, patch);
        else if (type == MT_SHADOWS)
            V_DrawFuzzPatch(ORIGINALWIDTH / 2, y, patch);
        else
            V_DrawPatch(ORIGINALWIDTH / 2, y, 0, patch);
    }
}

//
// F_DrawPatchCol
//
void F_DrawPatchCol(int x, patch_t *patch, int col, fixed_t fracstep)
{
    column_t    *column;
    byte        *source;
    byte        *dest;
    byte        *desttop;

    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

    desttop = screens[0] + x;

    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
        int         count = (column->length << FRACBITS) / fracstep;
        fixed_t     frac = 0;

        source = (byte *)column + 3;
        dest = desttop + column->topdelta * SCREENWIDTH;

        while (count--)
        {
            *dest = source[frac >> FRACBITS];
            dest += SCREENWIDTH;
            frac += fracstep;
        }
        column = (column_t *)((byte *)column + column->length + 4);
    }
}

//
// F_BunnyScroll
//
void F_BunnyScroll(void)
{
    int                 scrolled;
    int                 x;
    patch_t             *p1;
    patch_t             *p2;
    char                name[10];
    int                 stage;
    static int          laststage;
    const fixed_t       yscale = (ORIGINALHEIGHT << FRACBITS) / SCREENHEIGHT;
    const fixed_t       xscale = (ORIGINALWIDTH << FRACBITS) / SCREENWIDTH;
    fixed_t             frac = 0;

    p1 = W_CacheLumpName("PFUB2", PU_LEVEL);
    p2 = W_CacheLumpName("PFUB1", PU_LEVEL);

    scrolled = ORIGINALWIDTH - ((signed int)finalecount - 230) / 2;
    if (scrolled > ORIGINALWIDTH)
        scrolled = ORIGINALWIDTH;
    if (scrolled < 0)
        scrolled = 0;

    for (x = 0; x < ORIGINALWIDTH; x++)
    {
        do
        {
            if (x + scrolled < ORIGINALWIDTH)
                F_DrawPatchCol(frac / xscale, p1, x + scrolled, yscale);
            else
                F_DrawPatchCol(frac / xscale, p2, x + scrolled - ORIGINALWIDTH, yscale);
            frac += xscale;
        }
        while ((frac >> FRACBITS) <= x);
    }

    if (finalecount < 1130)
        return;
    if (finalecount < 1180)
    {
        V_DrawPatchWithShadow((ORIGINALWIDTH - 13 * 8) / 2 + 1, (ORIGINALHEIGHT - 8 * 8) / 2 + 1,
            W_CacheLumpName("END0", PU_CACHE), false);
        laststage = 0;
        return;
    }

    stage = (finalecount - 1180) / 5;
    if (stage > 6)
        stage = 6;
    if (stage > laststage)
    {
        S_StartSound(NULL, sfx_pistol);
        laststage = stage;
    }

    M_snprintf(name, 10, "END%i", stage);
    V_DrawPatchWithShadow((ORIGINALWIDTH - 13 * 8) / 2 + 1, (ORIGINALHEIGHT - 8 * 8) / 2 + 1,
        W_CacheLumpName(name, PU_CACHE), false);
}

static void F_ArtScreenDrawer(void)
{
    if (gameepisode == 3)
        F_BunnyScroll();
    else
    {
        char    *lumpname;

        switch (gameepisode)
        {
            case 1:
                lumpname = (gamemode == retail ? "CREDIT" : "HELP2");
                break;
            case 2:
                lumpname = "VICTORY2";
                break;
            case 4:
                lumpname = "ENDPIC";
                break;
            default:
                return;
        }

        V_DrawPatch(0, 0, 0, W_CacheLumpName(lumpname, PU_CACHE));
    }
}

//
// F_Drawer
//
void F_Drawer(void)
{
    switch (finalestage)
    {
        case F_STAGE_CAST:
            F_CastDrawer();
            break;
        case F_STAGE_TEXT:
            F_TextWrite();
            break;
        case F_STAGE_ARTSCREEN:
            F_ArtScreenDrawer();
            break;
    }
}
