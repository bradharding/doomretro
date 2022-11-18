/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2022 by id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2022 by Brad Harding <mailto:brad@doomretro.com>.

  DOOM Retro is a fork of Chocolate DOOM. For a list of acknowledgments,
  see <https://github.com/bradharding/doomretro/wiki/ACKNOWLEDGMENTS>.

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

#include <ctype.h>

#include "c_console.h"
#include "d_deh.h"
#include "d_englsh.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_colors.h"
#include "i_gamecontroller.h"
#include "i_swap.h"
#include "m_config.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_setup.h"
#include "v_data.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_stuff.h"
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
static bool F_CastResponder(event_t *ev);

void WI_CheckForAccelerate(void);
void A_RandomJump(mobj_t *actor, player_t *player, pspdef_t *psp);

static bool             midstage;               // whether we're in "mid-stage"

//
// F_ConsoleFinaleText
//
static void F_ConsoleFinaleText(void)
{
    char    *text = M_StringJoin("\"", finaletext, "\"", NULL);
    char    *p = strtok(text, "\n");

    while (p)
    {
        C_Output(ITALICS("%s%s"), (p[0] == '\"' ? "" : "   "), p);
        p = strtok(NULL, "\n");
    }

    free(text);
}

//
// F_StartFinale
//
void F_StartFinale(void)
{
    char    *intertext = P_GetInterText(gamemap);
    char    *intersecret = P_GetInterSecretText(gamemap);

    viewactive = false;
    automapactive = false;

    // killough 03/28/98: clear accelerative text flags
    acceleratestage = false;
    midstage = false;

    C_AddConsoleDivider();

    if (*intertext || (*intersecret && secretexit))
    {
        char    *interbackdrop = P_GetInterBackrop(gamemap);
        int     mus = P_GetInterMusic(gamemap);

        if (!secretexit)
        {
            if (M_StringCompare(trimwhitespace(intertext), "clear"))
            {
                gameaction = ga_worlddone;
                return;
            }

            finaletext = intertext;
        }
        else
        {
            if (M_StringCompare(trimwhitespace(intersecret), "clear"))
            {
                gameaction = ga_worlddone;
                return;
            }

            finaletext = intersecret;
        }

        finaleflat = (*interbackdrop ? interbackdrop : "FLOOR4_8");

        if (mus > 0)
            S_ChangeMusInfoMusic(mus, true);
        else
            S_ChangeMusic((gamemode == commercial ? mus_read_m : mus_victor), true, false, false);
    }
    else if (P_GetMapEndCast(gamemap))
    {
        gameaction = ga_nothing;
        gamestate = GS_FINALE;
        F_StartCast();

        return;
    }
    else
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

                    case 5:
                        finaleflat = bgflatE5;
                        finaletext = s_E5TEXT;
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

bool F_Responder(event_t *ev)
{
    if (finalestage == F_STAGE_CAST)
        return F_CastResponder(ev);

    return false;
}

static fixed_t TextSpeed(void)
{
    return (midstage ? NEWTEXTSPEED : ((midstage = acceleratestage) ? (acceleratestage = false), NEWTEXTSPEED : TEXTSPEED));
}

//
// F_Ticker
//
void F_Ticker(void)
{
    if (menuactive || paused || consoleactive)
        return;

    WI_CheckForAccelerate();

    // advance animation
    finalecount++;

    if (finalestage == F_STAGE_CAST)
        F_CastTicker();

    if (finalestage == F_STAGE_TEXT)
    {
        if (finalecount > FixedMul((fixed_t)strlen(finaletext) * FRACUNIT, TextSpeed()) + (midstage ? NEWTEXTWAIT : TEXTWAIT)
            || (midstage && acceleratestage))
        {
            if (P_GetMapEndCast(gamemap))
                F_StartCast();
            else if (P_GetMapEndBunny(gamemap))
            {
                finalecount = 0;
                finalestage = F_STAGE_ARTSCREEN;
                wipegamestate = GS_NONE;
                S_StartMusic(mus_bunny);
            }
            else if (P_GetMapEndGame(gamemap))
            {
                finalecount = 0;
                finalestage = F_STAGE_ARTSCREEN;
                wipegamestate = GS_NONE;
            }
            else if (gamemode != commercial)
            {
                finalecount = 0;
                finalestage = F_STAGE_ARTSCREEN;
                wipegamestate = GS_NONE;

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
// F_TextWrite
//
static void F_TextWrite(void)
{
    // draw some of the text onto the screen
    const char  *ch = finaletext;
    int         cx = 12;
    int         cy = 10;
    char        prev = ' ';

    // erase the entire screen to a tiled background
    byte        *src = (byte *)W_CacheLumpName(finaleflat);
    byte        *dest = screens[0];

    for (int y = 0; y < SCREENHEIGHT; y++)
        for (int x = 0; x < SCREENWIDTH; x += 2)
        {
            byte    dot = src[(((y >> 1) & 63) << 6) + ((x >> 1) & 63)];

            *dest++ = dot;
            *dest++ = dot;
        }

    for (int count = MAX(0, FixedDiv((finalecount - 10) * FRACUNIT, TextSpeed()) >> FRACBITS); count; count--)
    {
        char    letter = *ch++;
        int     c;
        int     width;

        if (!letter)
            break;

        if (letter == '\n')
        {
            cx = 12;
            cy += (prev == '\n' ? 8 : 11);
            prev = letter;

            continue;
        }

        c = toupper(letter) - HU_FONTSTART;

        if (c < 0 || c >= HU_FONTSIZE)
        {
            cx += (prev == '.' || prev == '!' || prev == '?' || prev == '"' ? 5 : 3);
            prev = letter;

            continue;
        }

        if (cx > VANILLAWIDTH - 12)
            continue;

        if (STCFNxxx)
        {
            width = SHORT(hu_font[c]->width);
            V_DrawPatchWithShadow(cx + 1, cy + 1, hu_font[c], false);
        }
        else
        {
            if (prev == ' ')
            {
                if (letter == '"')
                    c = 64;
                else if (letter == '\'')
                    c = 65;
            }

            for (int k = 0; kern[k].char1; k++)
                if (prev == kern[k].char1 && c == kern[k].char2)
                {
                    cx += kern[k].adjust;
                    break;
                }

            width = (int)strlen(smallcharset[c]) / 10 - 1;
            M_DrawSmallChar(cx + 1, cy + 1, c, true);
        }

        prev = letter;
        cx += width;
    }
}

//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
#define CASTNUMMAX  18

typedef struct
{
    char        *name;
    char        **dehackedname;
    mobjtype_t  type;
} castinfo_t;

static castinfo_t castorder[CASTNUMMAX] =
{
    { CC_ZOMBIE,  &s_CC_ZOMBIE,  MT_POSSESSED },
    { CC_SHOTGUN, &s_CC_SHOTGUN, MT_SHOTGUY   },
    { CC_HEAVY,   &s_CC_HEAVY,   MT_CHAINGUY  },
    { CC_IMP,     &s_CC_IMP,     MT_TROOP     },
    { CC_DEMON,   &s_CC_DEMON,   MT_SERGEANT  },
    { CC_SPECTRE, &s_CC_SPECTRE, MT_SHADOWS   },
    { CC_LOST,    &s_CC_LOST,    MT_SKULL     },
    { CC_CACO,    &s_CC_CACO,    MT_HEAD      },
    { CC_HELL,    &s_CC_HELL,    MT_KNIGHT    },
    { CC_BARON,   &s_CC_BARON,   MT_BRUISER   },
    { CC_ARACH,   &s_CC_ARACH,   MT_BABY      },
    { CC_PAIN,    &s_CC_PAIN,    MT_PAIN      },
    { CC_REVEN,   &s_CC_REVEN,   MT_UNDEAD    },
    { CC_MANCU,   &s_CC_MANCU,   MT_FATSO     },
    { CC_ARCH,    &s_CC_ARCH,    MT_VILE      },
    { CC_SPIDER,  &s_CC_SPIDER,  MT_SPIDER    },
    { CC_CYBER,   &s_CC_CYBER,   MT_CYBORG    },
    { CC_HERO,    &s_CC_HERO,    MT_PLAYER    }
};

static int      castnum;
static int      casttics;
static state_t  *caststate;
static int      castrot;
static bool     castdeath;
static bool     castdeathflip;
static int      castframes;
static bool     castonmelee;
static bool     castattacking;

bool            firstevent;

// [crispy] randomize seestate and deathstate sounds in the cast
static int F_RandomizeSound(int sound)
{
    if (sound >= sfx_posit1 && sound <= sfx_posit3)
        return (sfx_posit1 + M_BigRandom() % 3);
    else if (sound == sfx_bgsit1 || sound == sfx_bgsit2)
        return (sfx_bgsit1 + M_BigRandom() % 2);
    else if (sound >= sfx_podth1 && sound <= sfx_podth3)
        return (sfx_podth1 + M_BigRandom() % 3);
    else if (sound == sfx_bgdth1 || sound == sfx_bgdth2)
        return (sfx_bgdth1 + M_BigRandom() % 2);
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
    caststate = &states[mobjinfo[castorder[0].type].seestate];
    casttics = caststate->tics;
    castrot = 0;
    castdeath = false;
    castdeathflip = false;
    finalestage = F_STAGE_CAST;
    castframes = 0;
    castonmelee = false;
    castattacking = false;

    S_ChangeMusic(mus_evil, true, false, false);

    if (mobjinfo[castorder[castnum].type].seesound)
        S_StartSound(NULL, F_RandomizeSound(mobjinfo[castorder[castnum].type].seesound));
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

        if (++castnum == CASTNUMMAX)
            castnum = 0;
        else
            D_FadeScreen(false);

        if (mobjinfo[castorder[castnum].type].seesound)
            S_StartSound(NULL, F_RandomizeSound(mobjinfo[castorder[castnum].type].seesound));

        caststate = &states[mobjinfo[castorder[castnum].type].seestate];
        castframes = 0;

        D_FadeScreen(false);
    }
    else
    {
        int st;

        // just advance to next state in animation
        if (!castdeath && caststate == &states[S_PLAY_ATK1])
            goto stopattack;    // Oh, gross hack!

        st = (caststate->action == &A_RandomJump && (M_BigRandom() & 255) < caststate->misc2 ? caststate->misc1 : caststate->nextstate);
        caststate = &states[st];
        castframes++;

        // sound hacks...
        switch (st)
        {
            case S_PLAY_ATK1:
                S_StartSound(NULL, sfx_dshtgn);
                break;

            case S_POSS_ATK2:
                S_StartSound(NULL, sfx_pistol);
                break;

            case S_SPOS_ATK2:
            case S_CPOS_ATK2:
            case S_CPOS_ATK3:
            case S_CPOS_ATK4:
            case S_SPID_ATK2:
            case S_SPID_ATK3:
                S_StartSound(NULL, sfx_shotgn);
                break;

            case S_VILE_ATK2:
                S_StartSound(NULL, sfx_vilatk);
                break;

            case S_SKEL_FIST2:
                S_StartSound(NULL, sfx_skeswg);
                break;

            case S_SKEL_FIST4:
                S_StartSound(NULL, sfx_skepch);
                break;

            case S_SKEL_MISS2:
                S_StartSound(NULL, sfx_skeatk);
                break;

            case S_FATT_ATK8:
            case S_FATT_ATK5:
            case S_FATT_ATK2:
            case S_BOSS_ATK2:
            case S_BOS2_ATK2:
            case S_HEAD_ATK2:
                S_StartSound(NULL, sfx_firsht);
                break;

            case S_TROO_ATK3:
                S_StartSound(NULL, sfx_claw);
                break;

            case S_SARG_ATK2:
                S_StartSound(NULL, sfx_sgtatk);
                break;

            case S_SKULL_ATK2:
            case S_PAIN_ATK3:
                S_StartSound(NULL, sfx_sklatk);
                break;

            case S_BSPI_ATK2:
                S_StartSound(NULL, sfx_plasma);
                break;

            case S_CYBER_ATK2:
            case S_CYBER_ATK4:
            case S_CYBER_ATK6:
                S_StartSound(NULL, sfx_rlaunc);
                break;
        }
    }

    if (!castdeath && castframes == 12)
    {
        // go into attack frame
        castattacking = true;

        if (castonmelee)
            caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
        else
            caststate = &states[mobjinfo[castorder[castnum].type].missilestate];

        castonmelee = !castonmelee;

        if (caststate == &states[S_NULL])
        {
            if (castonmelee)
                caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
            else
                caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
        }

        if (caststate == &states[S_PLAY_ATK1])
            S_StartSound(NULL, sfx_dshtgn);
    }

    if (castattacking && (castframes == 24 || caststate == &states[mobjinfo[castorder[castnum].type].seestate]))
    {
stopattack:
        castattacking = false;
        castframes = 0;
        caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    }

    casttics = caststate->tics;

    if (casttics == -1)
    {
        if (caststate->action == &A_RandomJump)
        {
            caststate = &states[((M_BigRandom() & 255) < caststate->misc2 ? caststate->misc1 : caststate->nextstate)];
            casttics = caststate->tics;
        }

        if (casttics == -1)
            casttics = 15;
    }
}

//
// F_CastResponder
//
static bool F_CastResponder(event_t *ev)
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

    if (ev->type == ev_controller && !(gamecontrollerbuttons & gamecontrollerfire) && !(gamecontrollerbuttons & gamecontrolleruse))
        return false;

    if (castdeath)
        return true;    // already in dying frames
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

    S_StartSound(viewplayer->mo, sfx_dshtgn);

    type = castorder[castnum].type;

    // go into death frame
    castdeath = true;

    if (r_corpses_mirrored && type != MT_CHAINGUY && type != MT_CYBORG)
        castdeathflip = (M_Random() & 1);

    caststate = &states[mobjinfo[type].deathstate];
    casttics = caststate->tics;

    if (casttics == -1 && caststate->action == &A_RandomJump)
    {
        caststate = &states[((M_BigRandom() & 255) < caststate->misc2 ? caststate->misc1 : caststate->nextstate)];
        casttics = caststate->tics;
    }

    castrot = 0;
    castframes = 0;
    castattacking = false;

    if (mobjinfo[type].deathsound)
        S_StartSound(NULL, F_RandomizeSound(mobjinfo[type].deathsound));

    return true;
}

//
// F_CastPrint
//
static void F_CastPrint(const char *text)
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
    cx = (VANILLAWIDTH - width) / 2;
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
    spritedef_t         *sprdef;
    spriteframe_t       *sprframe;
    int                 lump;
    int                 rot = 0;
    patch_t             *patch;
    int                 y = VANILLAHEIGHT - 30;
    const mobjtype_t    type = castorder[castnum].type;

    if (gamemission == pack_plut)
        patch = W_CacheLumpName("BOSSBAC2");
    else if (gamemission == pack_tnt)
        patch = W_CacheLumpName("BOSSBAC3");
    else
        patch = (FREEDOOM || hacx ? W_CacheLastLumpName(bgcastcall) : W_CacheLumpName(bgcastcall));

    if (SCREENWIDTH != NONWIDEWIDTH)
        memset(screens[0], FindDominantEdgeColor(patch), SCREENAREA);

    V_DrawWidePatch((SCREENWIDTH / SCREENSCALE - SHORT(patch->width)) / 2, 0, 0, patch);

    if (type == MT_PLAYER && M_StringCompare(castorder[castnum].name, *castorder[castnum].dehackedname))
        F_CastPrint(playername);
    else
        F_CastPrint(*castorder[castnum].dehackedname);

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
                    V_DrawFlippedSpectreShadowPatch(VANILLAWIDTH / 2, VANILLAHEIGHT - 28, patch);
                else
                    V_DrawFlippedShadowPatch(VANILLAWIDTH / 2, VANILLAHEIGHT - 28, patch);
            }
            else
            {
                if (type == MT_SHADOWS)
                    V_DrawFlippedSolidSpectreShadowPatch(VANILLAWIDTH / 2, VANILLAHEIGHT - 28, patch);
                else
                    V_DrawFlippedSolidShadowPatch(VANILLAWIDTH / 2, VANILLAHEIGHT - 28, patch);
            }
        }

        if (r_sprites_translucency && (type == MT_SKULL || (type == MT_PAIN && castdeath)))
            V_DrawFlippedTranslucentRedPatch(VANILLAWIDTH / 2, y, patch);
        else if (type == MT_SHADOWS)
            V_DrawFlippedFuzzPatch(VANILLAWIDTH / 2, y, patch);
        else
            V_DrawFlippedPatch(VANILLAWIDTH / 2, y, patch);
    }
    else
    {
        patch->leftoffset = (r_fixspriteoffsets ? newspriteoffset[lump] : spriteoffset[lump]) >> FRACBITS;

        if (r_shadows && ((type != MT_SKULL && type != MT_PAIN) || !castdeath))
        {
            if (r_shadows_translucency)
            {
                if (type == MT_SHADOWS)
                    V_DrawSpectreShadowPatch(VANILLAWIDTH / 2, VANILLAHEIGHT - 28, patch);
                else
                    V_DrawShadowPatch(VANILLAWIDTH / 2, VANILLAHEIGHT - 28, patch);
            }
            else
            {
                if (type == MT_SHADOWS)
                    V_DrawSolidSpectreShadowPatch(VANILLAWIDTH / 2, VANILLAHEIGHT - 28, patch);
                else
                    V_DrawSolidShadowPatch(VANILLAWIDTH / 2, VANILLAHEIGHT - 28, patch);
            }
        }

        if (r_sprites_translucency && (type == MT_SKULL || (type == MT_PAIN && castdeath)))
            V_DrawTranslucentRedPatch(VANILLAWIDTH / 2, y, patch);
        else if (type == MT_SHADOWS)
            V_DrawFuzzPatch(VANILLAWIDTH / 2, y, patch);
        else
            V_DrawPatch(VANILLAWIDTH / 2, y, 0, patch);
    }
}

//
// F_DrawPatchCol
//
static void F_DrawPatchCol(int x, patch_t *patch, int col)
{
    column_t    *column = (column_t *)((byte *)patch + LONG(patch->columnoffset[col]));
    byte        *desttop = &screens[0][x];

    // step through the posts in a column
    while (column->topdelta != 0xFF)
    {
        int     srccol = 0;
        byte    *source = (byte *)column + 3;
        byte    *dest = &desttop[((column->topdelta * DY) >> FRACBITS) * SCREENWIDTH];
        int     count = (column->length * DY) >> FRACBITS;

        while (count--)
        {
            *dest = source[srccol >> FRACBITS];
            srccol += DYI;
            dest += SCREENWIDTH;
        }

        column = (column_t *)((byte *)column + column->length + 4);
    }
}

//
// F_BunnyScroll
//
static void F_BunnyScroll(void)
{
    int     scrolled = BETWEEN(0, VANILLAWIDTH - (finalecount - 230) / 2, VANILLAWIDTH);
    patch_t *p1 = (FREEDOOM || hacx ? W_CacheLastLumpName("PFUB2") : W_CacheLumpName(REKKRSL ? "PFUB2W" : "PFUB2"));
    patch_t *p2 = (FREEDOOM || hacx ? W_CacheLastLumpName("PFUB1") : W_CacheLumpName(REKKRSL ? "PFUB1W" : "PFUB1"));
    int     p1offset = (VANILLAWIDTH - SHORT(p1->width)) / 2;
    int     p2offset = VANILLAWIDTH + (SHORT(p2->width) == VANILLAWIDTH ? -p1offset : p1offset);
    int     pillarwidth = MAX(0, (SCREENWIDTH - (SHORT(p1->width) << FRACBITS) / DXI) / 2);

    if (pillarwidth && SCREENWIDTH != NONWIDEWIDTH)
        memset(screens[0], FindDominantEdgeColor(p1), SCREENAREA);

    for (int x = pillarwidth; x < SCREENWIDTH - pillarwidth; x++)
    {
        int x2 = ((x * DXI) >> FRACBITS) - WIDESCREENDELTA + scrolled;

        if (x2 < p2offset)
            F_DrawPatchCol(x, p1, x2 - p1offset);
        else
            F_DrawPatchCol(x, p2, x2 - p2offset);
    }

    if (finalecount >= 1130)
    {
        static int  laststage;

        if (finalecount < 1180)
        {
            if (finalecount == 1130)
                D_FadeScreen(false);

            V_DrawPatchWithShadow((VANILLAWIDTH - 104) / 2 + 1, (VANILLAHEIGHT - 64) / 2 + 1,
                (FREEDOOM || hacx ? W_CacheLastLumpName("END0") : W_CacheLumpName("END0")), false);
            laststage = 0;
        }
        else
        {
            char    name[10];
            int     stage = MIN((finalecount - 1180) / 5, 6);

            if (stage > laststage)
            {
                S_StartSound(NULL, sfx_pistol);
                laststage = stage;
            }

            M_snprintf(name, sizeof(name), "END%i", stage);
            V_DrawPatchWithShadow((VANILLAWIDTH - 104) / 2 + 1, (VANILLAHEIGHT - 64) / 2 + 1,
                (FREEDOOM || hacx ? W_CacheLastLumpName(name) : W_CacheLumpName(name)), false);
        }
    }
}

//
// F_ArtScreenDrawer
//
static void F_ArtScreenDrawer(void)
{
    int lumpnum = P_GetMapEndPic(gamemap);

    if (lumpnum > 0)
    {
        if (!finalestage)
            F_TextWrite();
        else
            V_DrawPatch(0, 0, 0, W_CacheLumpNum(lumpnum));
    }
    else if (P_GetMapEndBunny(gamemap) || gameepisode == 3)
        F_BunnyScroll();
    else
    {
        patch_t *lump;

        switch (gameepisode)
        {
            case 1:
                lump = (gamemode == retail ? creditlump : W_CacheLumpName(REKKRSL ? "HELP2W" : "HELP2"));
                break;

            case 2:
                lump = (FREEDOOM || hacx ? W_CacheLastLumpName("VICTORY2") : W_CacheLumpName(REKKRSL ? "VICTORW2" : "VICTORY2"));
                break;

            case 4:
                lump = (FREEDOOM || hacx ? W_CacheLastLumpName("ENDPIC") : W_CacheLumpName(REKKRSL ? "ENDPICW" : "ENDPIC"));
                break;

            case 5:
                lump = W_CacheLumpName("SIGILEND");
                break;

            default:
                return;
        }

        if (SCREENWIDTH != NONWIDEWIDTH)
            memset(screens[0], FindDominantEdgeColor(lump), SCREENAREA);

        V_DrawWidePatch((SCREENWIDTH / SCREENSCALE - SHORT(lump->width)) / 2, 0, 0, lump);
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
