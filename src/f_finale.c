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

#include <ctype.h>

#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "i_swap.h"
#include "m_config.h"
#include "m_misc.h"
#include "m_random.h"
#include "s_sound.h"
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

#define TEXTSPEED       (3 * FRACUNIT)          // original value
#define TEXTWAIT        (250 * FRACUNIT)        // original value
#define NEWTEXTSPEED    ((FRACUNIT + 50) / 100) // new value
#define NEWTEXTWAIT     (1000 * FRACUNIT)       // new value

static char             *finaletext;
static char             *finaleflat;

static void F_StartCast(void);
static void F_CastTicker(void);
static dboolean F_CastResponder(event_t *ev);

void WI_checkForAccelerate(void);
void A_RandomJump(mobj_t *actor, player_t *player, pspdef_t *psp);

static int              midstage;               // whether we're in "mid-stage"

extern int              acceleratestage;        // accelerate intermission screens

//
// F_ConsoleFinaleText
//
static void F_ConsoleFinaleText(void)
{
    char    *text = M_StringDuplicate(finaletext);
    char    *p = strtok(text, "\n");

    while (p)
    {
        C_Output(p);
        p = strtok(NULL, "\n");
    }
}

//
// F_StartFinale
//
void F_StartFinale(void)
{
    viewactive = false;
    automapactive = false;

    // killough 3/28/98: clear accelerative text flags
    acceleratestage = 0;
    midstage = 0;

    C_AddConsoleDivider();

    // Okay - IWAD dependent stuff.
    // This has been changed severely, and
    //  some stuff might have changed in the process.
    switch (gamemode)
    {
        // DOOM 1 - E1, E3 or E4, but each nine missions
        case shareware:
        case registered:
        case retail:
            S_ChangeMusic(mus_victor, true, false, false);

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
            }

            break;

        // DOOM II and missions packs with E1, M34
        case commercial:
            S_ChangeMusic(mus_read_m, true, false, false);

            switch (gamemap)      // This is regular DOOM II
            {
                case 6:
                    finaleflat = bgflat06;
                    finaletext = (gamemission == pack_tnt ? s_T1TEXT : (gamemission == pack_plut ? s_P1TEXT : s_C1TEXT));
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
                    finaletext = (gamemission == pack_tnt ? s_T2TEXT : (gamemission == pack_plut ? s_P2TEXT : s_C2TEXT));
                    break;

                case 20:
                    finaleflat = bgflat20;
                    finaletext = (gamemission == pack_tnt ? s_T3TEXT : (gamemission == pack_plut ? s_P3TEXT : s_C3TEXT));
                    break;

                case 30:
                    finaleflat = bgflat30;
                    finaletext = (gamemission == pack_tnt ? s_T4TEXT : (gamemission == pack_plut ? s_P4TEXT : s_C4TEXT));
                    break;

                case 15:
                    finaleflat = bgflat15;
                    finaletext = (gamemission == pack_tnt ? s_T5TEXT : (gamemission == pack_plut ? s_P5TEXT : s_C5TEXT));
                    break;

                case 31:
                    finaleflat = bgflat31;
                    finaletext = (gamemission == pack_tnt ? s_T6TEXT : (gamemission == pack_plut ? s_P6TEXT : s_C6TEXT));
                    break;
            }

            break;

        // Indeterminate.
        default:
            S_ChangeMusic(mus_read_m, true, false, false);
            finaleflat = "F_SKY1";
            finaletext = s_C1TEXT;
            break;
    }

    if (strlen(finaletext) <= 1)
    {
        gameaction = ga_worlddone;
        return;
    }

    gameaction = ga_nothing;
    gamestate = GS_FINALE;

    finalestage = F_STAGE_TEXT;
    finalecount = 0;

    F_ConsoleFinaleText();
}

dboolean F_Responder(event_t *ev)
{
    if (finalestage == F_STAGE_CAST)
        return F_CastResponder(ev);

    return false;
}

static fixed_t TextSpeed(void)
{
    return (midstage ? NEWTEXTSPEED : (midstage = acceleratestage) ? acceleratestage = 0, NEWTEXTSPEED : TEXTSPEED);
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
        if (finalecount > FixedMul((fixed_t)strlen(finaletext) * FRACUNIT, TextSpeed()) + (midstage ? NEWTEXTWAIT : TEXTWAIT)
            || (midstage && acceleratestage))
        {
            if (gamemode != commercial)
            {
                finalecount = 0;
                finalestage = F_STAGE_ARTSCREEN;
                wipegamestate = GS_NONE;        // force a wipe

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

//
//
// F_TextWrite
//
extern patch_t *hu_font[HU_FONTSIZE];

void M_DrawSmallChar(int x, int y, int i, dboolean shadow);

static void F_TextWrite(void)
{
    // draw some of the text onto the screen
    byte        *src;
    byte        *dest;
    int         w;
    int         count = MAX(0, FixedDiv((finalecount - 10) * FRACUNIT, TextSpeed()) >> FRACBITS);
    const char  *ch = finaletext;
    int         cx = 12;
    int         cy = 10;
    char        letter;
    char        prev = ' ';

    // erase the entire screen to a tiled background
    src = (byte *)W_CacheLumpName((char *)finaleflat);
    dest = screens[0];

    for (int y = 0; y < SCREENHEIGHT; y += 2)
        for (int x = 0; x < SCREENWIDTH / 32; x += 2)
        {
            for (int i = 0; i < 64; i++)
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
            cx += (prev == '.' || prev == '!' || prev == '?' || prev == '"' ? 5 : 3);
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
                if (letter == '"')
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

            w = (int)strlen(smallcharset[c]) / 10 - 1;
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
static mobjtype_t castordertype[] =
{
    MT_POSSESSED,
    MT_SHOTGUY,
    MT_CHAINGUY,
    MT_TROOP,
    MT_SERGEANT,
    MT_SHADOWS,
    MT_SKULL,
    MT_HEAD,
    MT_KNIGHT,
    MT_BRUISER,
    MT_BABY,
    MT_PAIN,
    MT_UNDEAD,
    MT_FATSO,
    MT_VILE,
    MT_SPIDER,
    MT_CYBORG,
    MT_PLAYER,
    -1
};

static int      castnum;
static int      casttics;
static state_t  *caststate;
static int      castrot;
static dboolean castdeath;
static dboolean castdeathflip;
static int      castframes;
static dboolean castonmelee;
static dboolean castattacking;

dboolean        firstevent;

// [crispy] randomize seestate and deathstate sounds in the cast
static int F_RandomizeSound(int sound)
{
    if (sound >= sfx_posit1 && sound <= sfx_posit3)
        return sfx_posit1 + M_Random() % 3;
    else if (sound == sfx_bgsit1 || sound == sfx_bgsit2)
        return sfx_bgsit1 + M_Random() % 2;
    else if (sound >= sfx_podth1 && sound <= sfx_podth3)
        return sfx_podth1 + M_Random() % 3;
    else if (sound == sfx_bgdth1 || sound == sfx_bgdth2)
        return sfx_bgdth1 + M_Random() % 2;
    else
        return sound;
}

//
// F_StartCast
//
static void F_StartCast(void)
{
    firstevent = true;
    wipegamestate = GS_NONE;    // force a screen wipe
    castnum = 0;
    caststate = &states[mobjinfo[castordertype[0]].seestate];
    casttics = caststate->tics;
    castrot = 0;
    castdeath = false;
    castdeathflip = false;
    finalestage = F_STAGE_CAST;
    castframes = 0;
    castonmelee = false;
    castattacking = false;

    if (!M_StringCompare(playername, playername_default))
        s_CC_HERO = playername;

    S_ChangeMusic(mus_evil, true, false, false);

    if (mobjinfo[castordertype[castnum]].seesound)
        S_StartSound(NULL, F_RandomizeSound(mobjinfo[castordertype[castnum]].seesound));
}

//
// F_CastTicker
//
static void F_CastTicker(void)
{
    if (--casttics > 0)
        return;                 // not time to change state yet

    if (caststate->tics == -1 || caststate->nextstate == S_NULL)
    {
        // switch from deathstate to next monster
        castdeath = false;
        castdeathflip = false;

        if (castordertype[++castnum] == -1)
            castnum = 0;

        if (mobjinfo[castordertype[castnum]].seesound)
            S_StartSound(NULL, F_RandomizeSound(mobjinfo[castordertype[castnum]].seesound));

        caststate = &states[mobjinfo[castordertype[castnum]].seestate];
        castframes = 0;
    }
    else
    {
        int st;
        int sfx = 0;

        // just advance to next state in animation
        if (!castdeath && caststate == &states[S_PLAY_ATK1])
            goto stopattack;    // Oh, gross hack!

        st = (caststate->action == A_RandomJump && M_Random() < caststate->misc2 ? caststate->misc1 : caststate->nextstate);
        caststate = &states[st];
        castframes++;

        // sound hacks...
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
        }

        if (sfx)
            S_StartSound(NULL, sfx);
    }

    if (!castdeath && castframes == 12)
    {
        // go into attack frame
        castattacking = true;

        if (castonmelee)
            caststate = &states[mobjinfo[castordertype[castnum]].meleestate];
        else
            caststate = &states[mobjinfo[castordertype[castnum]].missilestate];

        castonmelee = !castonmelee;

        if (caststate == &states[S_NULL])
        {
            if (castonmelee)
                caststate = &states[mobjinfo[castordertype[castnum]].meleestate];
            else
                caststate = &states[mobjinfo[castordertype[castnum]].missilestate];
        }

        if (caststate == &states[S_PLAY_ATK1])
            S_StartSound(NULL, sfx_dshtgn);
    }

    if (castattacking)
        if (castframes == 24 || caststate == &states[mobjinfo[castordertype[castnum]].seestate])
        {
stopattack:
            castattacking = false;
            castframes = 0;
            caststate = &states[mobjinfo[castordertype[castnum]].seestate];
        }

    casttics = caststate->tics;

    if (casttics == -1)
    {
        if (caststate->action == A_RandomJump)
        {
            caststate = &states[M_Random() < caststate->misc2 ? caststate->misc1 : caststate->nextstate];
            casttics = caststate->tics;
        }

        if (casttics == -1)
            casttics = 15;
    }
}

//
// F_CastResponder
//
static dboolean F_CastResponder(event_t *ev)
{
    mobjtype_t  type;

    if (!ev->data1)
        return false;

    if (ev->type == ev_mouse && (ev->data1 & mousefire))
        firstevent = false;
    else if (firstevent)
    {
        firstevent = false;
        return true;
    }

    if (menuactive || paused || consoleactive)
        return false;

    if (ev->type == ev_keydown && ev->data1 != keyboarduse && ev->data1 != keyboarduse2
        && ev->data1 != keyboardfire && ev->data1 != KEY_LEFTARROW && ev->data1 != KEY_RIGHTARROW
        && ev->data1 != KEY_ENTER && ev->data1 != ' ')
        return false;

    if (ev->type == ev_keyup)
        return false;

    if (ev->type == ev_mouse && !(ev->data1 & mousefire) && !(ev->data1 & mouseuse))
        return false;

    if (ev->type == ev_gamepad && !(ev->data1 & gamepadfire) && !(ev->data1 & gamepaduse))
        return false;

    if (castdeath)
        return true;                    // already in dying frames
    else
    {
        // rotate (taken from Eternity Engine)
        if (ev->data1 == KEY_LEFTARROW)
        {
            castrot = (castrot == 14 ? 0 : castrot + 2);
            return true;
        }
        else if (ev->data1 == KEY_RIGHTARROW)
        {
            castrot = (!castrot ? 14 : castrot - 2);
            return true;
        }
    }

    S_StartSound(NULL, sfx_dshtgn);

    type = castordertype[castnum];

    // go into death frame
    castdeath = true;

    if (r_corpses_mirrored && type != MT_CHAINGUY && type != MT_CYBORG)
        castdeathflip = M_Random() & 1;

    caststate = &states[mobjinfo[type].deathstate];
    casttics = caststate->tics;

    if (casttics == -1 && caststate->action == A_RandomJump)
    {
        caststate = &states[(M_Random() < caststate->misc2 ? caststate->misc1 : caststate->nextstate)];
        casttics = caststate->tics;
    }

    castrot = 0;
    castframes = 0;
    castattacking = false;

    if (mobjinfo[type].deathsound)
        S_StartSound(NULL, F_RandomizeSound(mobjinfo[type].deathsound));

    return true;
}

static void F_CastPrint(char *text)
{
    const char  *ch = text;
    int         c;
    int         cx;
    int         width = 0;

    while (ch)
    {
        if (!(c = *ch++))
            break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE)
        {
            width += 4;
            continue;
        }

        width += SHORT(hu_font[c]->width);
    }

    // draw it
    cx = (ORIGINALWIDTH - width) / 2;
    ch = text;

    while (ch)
    {
        if (!(c = *ch++))
            break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        V_DrawPatchWithShadow(cx + 1, 181, hu_font[c], false);
        cx += SHORT(hu_font[c]->width);
    }
}

//
// F_CastDrawer
//
static void F_CastDrawer(void)
{
    spritedef_t     *sprdef;
    spriteframe_t   *sprframe;
    int             lump;
    int             rot = 0;
    patch_t         *patch;
    int             y = ORIGINALHEIGHT - 30;
    mobjtype_t      type = castordertype[castnum];

    // erase the entire screen to a background
    V_DrawPatch(0, 0, 0, W_CacheLumpName(bgcastcall));

    F_CastPrint(type == MT_PLAYER ? playername : mobjinfo[type].name1);

    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[caststate->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
        rot = castrot;

    lump = sprframe->lump[rot];
    patch = W_CacheLumpNum(lump + firstspritelump);
    patch->topoffset = (r_fixspriteoffsets ? newspritetopoffset[lump] : spritetopoffset[lump]) >> FRACBITS;

    if (type == MT_SKULL)
        y -= 30;
    else if (type == MT_PAIN || (type == MT_HEAD && !castdeath))
        y -= 20;

    if ((sprframe->flip & (1 << rot)) || castdeathflip)
    {
        patch->leftoffset = (spritewidth[lump] - (r_fixspriteoffsets ? newspriteoffset[lump] : spriteoffset[lump])) >> FRACBITS;

        if (r_shadows && ((type != MT_SKULL && type != MT_PAIN) || !castdeath))
        {
            if (r_shadows_translucency)
            {
                if (type == MT_SHADOWS)
                    V_DrawFlippedSpectreShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
                else
                    V_DrawFlippedShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
            }
            else
            {
                if (type == MT_SHADOWS)
                    V_DrawFlippedSolidSpectreShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
                else
                    V_DrawFlippedSolidShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
            }
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
        patch->leftoffset = (r_fixspriteoffsets ? newspriteoffset[lump] : spriteoffset[lump]) >> FRACBITS;

        if (r_shadows && ((type != MT_SKULL && type != MT_PAIN) || !castdeath))
        {
            if (r_shadows_translucency)
            {
                if (type == MT_SHADOWS)
                    V_DrawSpectreShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
                else
                    V_DrawShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
            }
            else
            {
                if (type == MT_SHADOWS)
                    V_DrawSolidSpectreShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
                else
                    V_DrawSolidShadowPatch(ORIGINALWIDTH / 2, ORIGINALHEIGHT - 28, patch);
            }
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
static void F_DrawPatchCol(int x, patch_t *patch, int col, fixed_t fracstep)
{
    column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
    byte        *desttop = screens[0] + x;

    // step through the posts in a column
    while (column->topdelta != 0xFF)
    {
        int     count = (column->length << FRACBITS) / fracstep;
        fixed_t frac = 0;
        byte    *dest = desttop + column->topdelta * SCREENWIDTH;
        byte    *source = (byte *)column + 3;

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
static void F_BunnyScroll(void)
{
    int             scrolled = BETWEEN(0, ORIGINALWIDTH - (finalecount - 230) / 2, ORIGINALWIDTH);
    patch_t         *p1 = W_CacheLumpName("PFUB2");
    patch_t         *p2 = W_CacheLumpName("PFUB1");
    char            name[10];
    int             stage;
    static int      laststage;
    const fixed_t   yscale = (ORIGINALHEIGHT << FRACBITS) / SCREENHEIGHT;
    const fixed_t   xscale = (ORIGINALWIDTH << FRACBITS) / SCREENWIDTH;
    fixed_t         frac = 0;

    for (int x = 0; x < ORIGINALWIDTH; x++)
    {
        do
        {
            if (x + scrolled < ORIGINALWIDTH)
                F_DrawPatchCol(frac / xscale, p1, x + scrolled, yscale);
            else
                F_DrawPatchCol(frac / xscale, p2, x + scrolled - ORIGINALWIDTH, yscale);

            frac += xscale;
        } while ((frac >> FRACBITS) <= x);
    }

    if (finalecount < 1130)
        return;
    else if (finalecount < 1180)
    {
        V_DrawPatchWithShadow((ORIGINALWIDTH - 13 * 8) / 2 + 1, (ORIGINALHEIGHT - 8 * 8) / 2 + 1, W_CacheLumpName("END0"), false);
        laststage = 0;
        return;
    }

    if ((stage = MIN((finalecount - 1180) / 5, 6)) > laststage)
    {
        S_StartSound(NULL, sfx_pistol);
        laststage = stage;
    }

    M_snprintf(name, sizeof(name), "END%i", stage);
    V_DrawPatchWithShadow((ORIGINALWIDTH - 13 * 8) / 2 + 1, (ORIGINALHEIGHT - 8 * 8) / 2 + 1, W_CacheLumpName(name), false);
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

        V_DrawPatch(0, 0, 0, W_CacheLumpName(lumpname));
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
