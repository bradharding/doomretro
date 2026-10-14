/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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


#include "d_player.h"
#include "i_system.h"
#include "id24info.h"
#include "info.h"
#include "m_misc.h"
#include "p_local.h"
#include "sounds.h"
#include "sprites.h"
#include "states.h"

// References to other ID24 sprites, states, sounds and things are stored in the
// tables below relative to the start of each table, tagged with the kind of
// reference. They're resolved by ID24_Init() once the runtime bases are known.
#define I24SPR(n)       (0x10000000 + (n))
#define I24ST(n)        (0x20000000 + (n))
#define I24SFX(n)       (0x30000000 + (n))
#define I24MOARG(n)     (0x50000000 + (n))  // thing numbers in state arguments are one-based

#define NUMID24SPRITES  44
#define NUMID24SFX      37
#define NUMID24STATES   465
#define NUMID24MOBJS    57

#define I24SPR_BSH1  I24SPR(0)
#define I24SPR_BSH2  I24SPR(1)
#define I24SPR_BSHE  I24SPR(2)
#define I24SPR_CBR2  I24SPR(3)
#define I24SPR_CHR1  I24SPR(4)
#define I24SPR_CSPI  I24SPR(5)
#define I24SPR_CYB2  I24SPR(6)
#define I24SPR_FLMF  I24SPR(7)
#define I24SPR_FLMG  I24SPR(8)
#define I24SPR_GBAL  I24SPR(9)
#define I24SPR_GHUL  I24SPR(10)
#define I24SPR_GOR6  I24SPR(11)
#define I24SPR_GOR7  I24SPR(12)
#define I24SPR_GOR8  I24SPR(13)
#define I24SPR_GORA  I24SPR(14)
#define I24SPR_HBB2  I24SPR(15)
#define I24SPR_HBBQ  I24SPR(16)
#define I24SPR_HDB7  I24SPR(17)
#define I24SPR_HDB8  I24SPR(18)
#define I24SPR_HETB  I24SPR(19)
#define I24SPR_HETC  I24SPR(20)
#define I24SPR_HETF  I24SPR(21)
#define I24SPR_HETG  I24SPR(22)
#define I24SPR_IFLM  I24SPR(23)
#define I24SPR_LAMP  I24SPR(24)
#define I24SPR_PHED  I24SPR(25)
#define I24SPR_POB6  I24SPR(26)
#define I24SPR_POL7  I24SPR(27)
#define I24SPR_POLA  I24SPR(28)
#define I24SPR_PPOS  I24SPR(29)
#define I24SPR_STC1  I24SPR(30)
#define I24SPR_STC2  I24SPR(31)
#define I24SPR_STC3  I24SPR(32)
#define I24SPR_STG1  I24SPR(33)
#define I24SPR_STG2  I24SPR(34)
#define I24SPR_STG3  I24SPR(35)
#define I24SPR_STMI  I24SPR(36)
#define I24SPR_TLP6  I24SPR(37)
#define I24SPR_VASS  I24SPR(38)
#define I24SPR_VFLM  I24SPR(39)
#define I24SPR_INCN  I24SPR(40)
#define I24SPR_CBLD  I24SPR(41)
#define I24SPR_FCPU  I24SPR(42)
#define I24SPR_FTNK  I24SPR(43)

void A_BossDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckAmmo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ConsumeAmmo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CyberAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GunFlash(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GunFlashTo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Hoof(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MonsterProjectile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PlaySound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RadiusDamage(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RandomJump(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RefireTo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RemoveFlags(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnObject(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpidRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponJump(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponProjectile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_XScream(mobj_t *actor, player_t *player, pspdef_t *psp);

static const char *const id24_sprnames[NUMID24SPRITES] =
{
    "BSH1", "BSH2", "BSHE", "CBR2", "CHR1", "CSPI", "CYB2", "FLMF",
    "FLMG", "GBAL", "GHUL", "GOR6", "GOR7", "GOR8", "GORA", "HBB2",
    "HBBQ", "HDB7", "HDB8", "HETB", "HETC", "HETF", "HETG", "IFLM",
    "LAMP", "PHED", "POB6", "POL7", "POLA", "PPOS", "STC1", "STC2",
    "STC3", "STG1", "STG2", "STG3", "STMI", "TLP6", "VASS", "VFLM",
    "INCN", "CBLD", "FCPU", "FTNK"
};

static const char *const id24_sfxnames[NUMID24SFX] =
{
    "banact", "bandth", "banpai", "break", "cspact", "cspdth", "cspsit", "cspwlk",
    "gatcls", "gatlop", "gatopn", "ghlact", "ghldth", "ghlpai", "ghlsit", "hetchg",
    "hetsht", "hetxpl", "incbrn", "incfi1", "incfi2", "incht1", "incht2", "incht3",
    "klaxon", "ppoact", "ppodth", "ppohed", "ppopai", "tyrdth", "tyrsit", "tyrwlk",
    "vasact", "vasatk", "vasdth", "vaspai", "vassit"
};

static const state_t id24_states[NUMID24STATES] =
{
  //  sprite       frame                  tics  action                nextstate   state
    // Large corpse pile
    { I24SPR_POL7, 0,                      -1, NULL,                 S_NULL     }, // 0

    // Human bbq 1
    { I24SPR_HBBQ, FF_FULLBRIGHT,           5, NULL,                 I24ST(2)   }, // 1
    { I24SPR_HBBQ, (1 | FF_FULLBRIGHT),     5, NULL,                 I24ST(3)   }, // 2
    { I24SPR_HBBQ, (2 | FF_FULLBRIGHT),     5, NULL,                 I24ST(1)   }, // 3

    // Human bbq 2
    { I24SPR_HBB2, FF_FULLBRIGHT,           5, NULL,                 I24ST(5)   }, // 4
    { I24SPR_HBB2, (1 | FF_FULLBRIGHT),     5, NULL,                 I24ST(6)   }, // 5
    { I24SPR_HBB2, (2 | FF_FULLBRIGHT),     5, NULL,                 I24ST(4)   }, // 6

    // Hanging victim, both legs
    { I24SPR_GOR6, 0,                      -1, NULL,                 S_NULL     }, // 7

    // Hanging victim, crucified
    { I24SPR_GOR7, 0,                      -1, NULL,                 S_NULL     }, // 8

    // Hanging victim, arms bound
    { I24SPR_GOR8, 0,                      -1, NULL,                 S_NULL     }, // 9

    // Hanging baron of hell
    { I24SPR_GORA, 0,                      -1, NULL,                 S_NULL     }, // 10

    // Hanging victim, chained
    { I24SPR_HDB7, 0,                      -1, NULL,                 S_NULL     }, // 11

    // Hanging torso, chained
    { I24SPR_HDB8, 0,                      -1, NULL,                 S_NULL     }, // 12

    // Skull pole trio
    { I24SPR_POLA, 0,                      -1, NULL,                 S_NULL     }, // 13

    // Skull gibs
    { I24SPR_POB6, 0,                      -1, NULL,                 S_NULL     }, // 14

    // Stalagmite (gray)
    { SPR_SMT2,    0,                      -1, NULL,                 S_NULL     }, // 15

    // Bush, short
    { I24SPR_BSH1, 0,                      -1, NULL,                 S_NULL     }, // 16

    // Bush, short burned 1
    { I24SPR_BSH1, 1,                      -1, NULL,                 S_NULL     }, // 17

    // Bush, short burned 2
    { I24SPR_BSH1, 2,                      -1, NULL,                 S_NULL     }, // 18

    // Bush, tall
    { I24SPR_BSH2, 0,                      -1, NULL,                 S_NULL     }, // 19

    // Bush, tall burned 1
    { I24SPR_BSH2, 1,                      -1, NULL,                 S_NULL     }, // 20

    // Bush, tall burned 2
    { I24SPR_BSH2, 2,                      -1, NULL,                 S_NULL     }, // 21

    // Cave rock column
    { I24SPR_STMI, 0,                      -1, NULL,                 S_NULL     }, // 22

    // Cave stalagmite, large
    { I24SPR_STG1, 0,                      -1, NULL,                 S_NULL     }, // 23

    // Cave stalagmite, medium
    { I24SPR_STG2, 0,                      -1, NULL,                 S_NULL     }, // 24

    // Cave stalagmite, small
    { I24SPR_STG3, 0,                      -1, NULL,                 S_NULL     }, // 25

    // Cave stalactite, large
    { I24SPR_STC1, 0,                      -1, NULL,                 S_NULL     }, // 26

    // Cave stalactite, medium
    { I24SPR_STC2, 0,                      -1, NULL,                 S_NULL     }, // 27

    // Cave stalactite, small
    { I24SPR_STC3, 0,                      -1, NULL,                 S_NULL     }, // 28

    // Office chair
    { I24SPR_CHR1, 0,                      -1, NULL,                 S_NULL     }, // 29

    // Office lamp (breakable)
    { I24SPR_LAMP, FF_FULLBRIGHT,          -1, NULL,                 S_NULL     }, // 30
    { I24SPR_LAMP, (1 | FF_FULLBRIGHT),     3, &A_Scream,            I24ST(32)  }, // 31
    { I24SPR_LAMP, (1 | FF_FULLBRIGHT),     3, &A_Fall,              I24ST(33)  }, // 32
    { I24SPR_LAMP, (2 | FF_FULLBRIGHT),     5, NULL,                 I24ST(34)  }, // 33
    { I24SPR_LAMP, (3 | FF_FULLBRIGHT),     5, NULL,                 I24ST(33)  }, // 34

    // Ceiling lamp
    { I24SPR_TLP6, FF_FULLBRIGHT,          -1, NULL,                 S_NULL     }, // 35

    // Candelabra (short)
    { I24SPR_CBR2, FF_FULLBRIGHT,          -1, NULL,                 S_NULL     }, // 36

    // Ambient klaxon
    { SPR_TNT1,    0,                       1, &A_Look,              I24ST(37)  }, // 37
    { SPR_TNT1,    0,                      35, &A_PlaySound,         I24ST(38), .misc1 = I24SFX(24) }, // 38
    { SPR_TNT1,    0,                     250, &A_PlaySound,         S_NULL, .misc1 = I24SFX(10), .misc2 = 1 }, // 39
    { SPR_TNT1,    0,                     139, &A_PlaySound,         I24ST(40), .misc1 = I24SFX(9) }, // 40
    { SPR_TNT1,    0,                     105, &A_PlaySound,         S_NULL, .misc1 = I24SFX(8), .misc2 = 1 }, // 41

    // Ghoul
    { I24SPR_GHUL, 0,                      10, &A_Look,              I24ST(43)  }, // 42
    { I24SPR_GHUL, 1,                      10, &A_Look,              I24ST(42)  }, // 43
    { I24SPR_GHUL, 0,                       3, &A_Chase,             I24ST(45)  }, // 44
    { I24SPR_GHUL, 0,                       3, &A_Chase,             I24ST(46)  }, // 45
    { I24SPR_GHUL, 1,                       3, &A_Chase,             I24ST(47)  }, // 46
    { I24SPR_GHUL, 1,                       3, &A_Chase,             I24ST(48)  }, // 47
    { I24SPR_GHUL, 2,                       3, &A_Chase,             I24ST(49)  }, // 48
    { I24SPR_GHUL, 2,                       3, &A_Chase,             I24ST(50)  }, // 49
    { I24SPR_GHUL, 1,                       3, &A_Chase,             I24ST(51)  }, // 50
    { I24SPR_GHUL, 1,                       3, &A_Chase,             I24ST(44)  }, // 51
    { I24SPR_GHUL, (3 | FF_FULLBRIGHT),     4, &A_FaceTarget,        I24ST(53)  }, // 52
    { I24SPR_GHUL, (4 | FF_FULLBRIGHT),     4, &A_FaceTarget,        I24ST(54)  }, // 53
    { I24SPR_GHUL, (5 | FF_FULLBRIGHT),     4, &A_MonsterProjectile, I24ST(55), .args = { I24MOARG(12), 0, 0, 0, -524288 } }, // 54
    { I24SPR_GHUL, (6 | FF_FULLBRIGHT),     4, NULL,                 I24ST(44)  }, // 55
    { I24SPR_GHUL, (8 | FF_FULLBRIGHT),     3, NULL,                 I24ST(57)  }, // 56
    { I24SPR_GHUL, (10 | FF_FULLBRIGHT),    3, &A_Pain,              I24ST(44)  }, // 57
    { I24SPR_GHUL, (11 | FF_FULLBRIGHT),    5, NULL,                 I24ST(59)  }, // 58
    { I24SPR_GHUL, (12 | FF_FULLBRIGHT),    5, &A_Scream,            I24ST(60)  }, // 59
    { I24SPR_GHUL, (13 | FF_FULLBRIGHT),    5, NULL,                 I24ST(61)  }, // 60
    { I24SPR_GHUL, (14 | FF_FULLBRIGHT),    5, NULL,                 I24ST(62)  }, // 61
    { I24SPR_GHUL, (15 | FF_FULLBRIGHT),    5, &A_Fall,              I24ST(63)  }, // 62
    { I24SPR_GHUL, (16 | FF_FULLBRIGHT),    5, NULL,                 I24ST(64)  }, // 63
    { I24SPR_GHUL, (17 | FF_FULLBRIGHT),    5, NULL,                 I24ST(65)  }, // 64
    { I24SPR_GHUL, 18,                     -1, NULL,                 S_NULL     }, // 65
    { I24SPR_GBAL, FF_FULLBRIGHT,           4, NULL,                 I24ST(67)  }, // 66
    { I24SPR_GBAL, (1 | FF_FULLBRIGHT),     4, NULL,                 I24ST(66)  }, // 67
    { I24SPR_GBAL, (2 | FF_FULLBRIGHT),     5, NULL,                 I24ST(69)  }, // 68
    { SPR_APBX,    (1 | FF_FULLBRIGHT),     5, NULL,                 I24ST(70)  }, // 69
    { SPR_APBX,    (2 | FF_FULLBRIGHT),     5, NULL,                 I24ST(71)  }, // 70
    { SPR_APBX,    (3 | FF_FULLBRIGHT),     5, NULL,                 I24ST(72)  }, // 71
    { SPR_APBX,    (4 | FF_FULLBRIGHT),     5, NULL,                 S_NULL     }, // 72

    // Banshee
    { I24SPR_BSHE, FF_FULLBRIGHT,          10, &A_Look,              I24ST(74)  }, // 73
    { I24SPR_BSHE, (1 | FF_FULLBRIGHT),    10, &A_Look,              I24ST(73)  }, // 74
    { I24SPR_BSHE, FF_FULLBRIGHT,           2, &A_Chase,             I24ST(76)  }, // 75
    { I24SPR_BSHE, FF_FULLBRIGHT,           2, &A_Chase,             I24ST(77)  }, // 76
    { I24SPR_BSHE, FF_FULLBRIGHT,           2, &A_Chase,             I24ST(78)  }, // 77
    { I24SPR_BSHE, (1 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(79)  }, // 78
    { I24SPR_BSHE, (1 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(80)  }, // 79
    { I24SPR_BSHE, (1 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(81)  }, // 80
    { I24SPR_BSHE, (2 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(82)  }, // 81
    { I24SPR_BSHE, (2 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(83)  }, // 82
    { I24SPR_BSHE, (2 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(84)  }, // 83
    { I24SPR_BSHE, FF_FULLBRIGHT,           2, &A_Chase,             I24ST(85)  }, // 84
    { I24SPR_BSHE, FF_FULLBRIGHT,           2, &A_Chase,             I24ST(86)  }, // 85
    { I24SPR_BSHE, FF_FULLBRIGHT,           2, &A_Chase,             I24ST(87)  }, // 86
    { I24SPR_BSHE, (1 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(88)  }, // 87
    { I24SPR_BSHE, (1 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(89)  }, // 88
    { I24SPR_BSHE, (1 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(90)  }, // 89
    { I24SPR_BSHE, (2 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(91)  }, // 90
    { I24SPR_BSHE, (2 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(92)  }, // 91
    { I24SPR_BSHE, (2 | FF_FULLBRIGHT),     2, &A_Chase,             I24ST(93)  }, // 92
    { I24SPR_BSHE, FF_FULLBRIGHT,           0, &A_PlaySound,         I24ST(75), .misc1 = I24SFX(0) }, // 93
    { I24SPR_BSHE, (3 | FF_FULLBRIGHT),     1, &A_RadiusDamage,      I24ST(94), .args = { 100, 8 } }, // 94
    { I24SPR_BSHE, (3 | FF_FULLBRIGHT),     3, NULL,                 I24ST(96)  }, // 95
    { I24SPR_BSHE, (3 | FF_FULLBRIGHT),     3, &A_Pain,              I24ST(75)  }, // 96
    { I24SPR_BSHE, (3 | FF_FULLBRIGHT),     4, &A_Scream,            I24ST(98)  }, // 97
    { I24SPR_BSHE, (4 | FF_FULLBRIGHT),     6, &A_RadiusDamage,      I24ST(99), .args = { 128, 128 } }, // 98
    { I24SPR_BSHE, (5 | FF_FULLBRIGHT),     8, &A_Fall,              I24ST(100) }, // 99
    { I24SPR_BSHE, (6 | FF_FULLBRIGHT),     6, NULL,                 I24ST(101) }, // 100
    { I24SPR_BSHE, (7 | FF_FULLBRIGHT),     4, NULL,                 I24ST(102) }, // 101
    { SPR_TNT1,    0,                      20, NULL,                 S_NULL     }, // 102

    // Mindweaver
    { I24SPR_CSPI, 0,                      10, &A_Look,              I24ST(104) }, // 103
    { I24SPR_CSPI, 1,                      10, &A_Look,              I24ST(103) }, // 104
    { I24SPR_CSPI, 0,                      20, NULL,                 I24ST(106) }, // 105
    { I24SPR_CSPI, 0,                       0, &A_PlaySound,         I24ST(107), .misc1 = I24SFX(7) }, // 106
    { I24SPR_CSPI, 0,                       3, &A_Chase,             I24ST(108) }, // 107
    { I24SPR_CSPI, 0,                       3, &A_Chase,             I24ST(109) }, // 108
    { I24SPR_CSPI, 1,                       3, &A_Chase,             I24ST(110) }, // 109
    { I24SPR_CSPI, 1,                       3, &A_Chase,             I24ST(111) }, // 110
    { I24SPR_CSPI, 2,                       3, &A_Chase,             I24ST(112) }, // 111
    { I24SPR_CSPI, 2,                       3, &A_Chase,             I24ST(113) }, // 112
    { I24SPR_CSPI, 3,                       0, &A_PlaySound,         I24ST(114), .misc1 = I24SFX(7) }, // 113
    { I24SPR_CSPI, 3,                       3, &A_Chase,             I24ST(115) }, // 114
    { I24SPR_CSPI, 3,                       3, &A_Chase,             I24ST(116) }, // 115
    { I24SPR_CSPI, 4,                       3, &A_Chase,             I24ST(117) }, // 116
    { I24SPR_CSPI, 4,                       3, &A_Chase,             I24ST(118) }, // 117
    { I24SPR_CSPI, 5,                       3, &A_Chase,             I24ST(119) }, // 118
    { I24SPR_CSPI, 5,                       3, &A_Chase,             I24ST(106) }, // 119
    { I24SPR_CSPI, FF_FULLBRIGHT,          20, &A_FaceTarget,        I24ST(121) }, // 120
    { I24SPR_CSPI, (6 | FF_FULLBRIGHT),     4, &A_SPosAttack,        I24ST(122) }, // 121
    { I24SPR_CSPI, (7 | FF_FULLBRIGHT),     4, &A_SPosAttack,        I24ST(123) }, // 122
    { I24SPR_CSPI, (7 | FF_FULLBRIGHT),     1, &A_SpidRefire,        I24ST(121) }, // 123
    { I24SPR_CSPI, 8,                       3, NULL,                 I24ST(125) }, // 124
    { I24SPR_CSPI, 8,                       3, &A_Pain,              I24ST(106) }, // 125
    { I24SPR_CSPI, 9,                      20, &A_Scream,            I24ST(127) }, // 126
    { I24SPR_CSPI, 10,                      7, &A_Fall,              I24ST(128) }, // 127
    { I24SPR_CSPI, 11,                      7, NULL,                 I24ST(129) }, // 128
    { I24SPR_CSPI, 12,                      7, NULL,                 I24ST(130) }, // 129
    { I24SPR_CSPI, 13,                      7, NULL,                 I24ST(131) }, // 130
    { I24SPR_CSPI, 14,                      7, NULL,                 I24ST(132) }, // 131
    { I24SPR_CSPI, 15,                     -1, &A_BossDeath,         S_NULL     }, // 132
    { I24SPR_CSPI, 15,                      5, NULL,                 I24ST(134) }, // 133
    { I24SPR_CSPI, 14,                      5, NULL,                 I24ST(135) }, // 134
    { I24SPR_CSPI, 13,                      5, NULL,                 I24ST(136) }, // 135
    { I24SPR_CSPI, 12,                      5, NULL,                 I24ST(137) }, // 136
    { I24SPR_CSPI, 11,                      5, NULL,                 I24ST(138) }, // 137
    { I24SPR_CSPI, 10,                      5, NULL,                 I24ST(139) }, // 138
    { I24SPR_CSPI, 9,                       5, NULL,                 I24ST(106) }, // 139

    // Shocktrooper
    { I24SPR_PPOS, 0,                      10, &A_Look,              I24ST(141) }, // 140
    { I24SPR_PPOS, 1,                      10, &A_Look,              I24ST(140) }, // 141
    { I24SPR_PPOS, 0,                       2, &A_Chase,             I24ST(143), .flags = STATEF_SKILL5FAST }, // 142
    { I24SPR_PPOS, 0,                       2, &A_Chase,             I24ST(144), .flags = STATEF_SKILL5FAST }, // 143
    { I24SPR_PPOS, 1,                       2, &A_Chase,             I24ST(145), .flags = STATEF_SKILL5FAST }, // 144
    { I24SPR_PPOS, 1,                       2, &A_Chase,             I24ST(146), .flags = STATEF_SKILL5FAST }, // 145
    { I24SPR_PPOS, 2,                       2, &A_Chase,             I24ST(147), .flags = STATEF_SKILL5FAST }, // 146
    { I24SPR_PPOS, 2,                       2, &A_Chase,             I24ST(148), .flags = STATEF_SKILL5FAST }, // 147
    { I24SPR_PPOS, 3,                       2, &A_Chase,             I24ST(149), .flags = STATEF_SKILL5FAST }, // 148
    { I24SPR_PPOS, 3,                       2, &A_Chase,             I24ST(142), .flags = STATEF_SKILL5FAST }, // 149
    { I24SPR_PPOS, 4,                      10, &A_FaceTarget,        I24ST(151) }, // 150
    { I24SPR_PPOS, 5,                       2, &A_MonsterProjectile, I24ST(152), .args = { 35 }, .flags = STATEF_SKILL5FAST }, // 151
    { I24SPR_PPOS, 4,                       4, NULL,                 I24ST(153), .flags = STATEF_SKILL5FAST }, // 152
    { I24SPR_PPOS, 5,                       2, &A_MonsterProjectile, I24ST(154), .args = { 35 }, .flags = STATEF_SKILL5FAST }, // 153
    { I24SPR_PPOS, 4,                       4, NULL,                 I24ST(155), .flags = STATEF_SKILL5FAST }, // 154
    { I24SPR_PPOS, 5,                       2, &A_MonsterProjectile, I24ST(156), .args = { 35 }, .flags = STATEF_SKILL5FAST }, // 155
    { I24SPR_PPOS, 4,                       4, NULL,                 I24ST(142), .flags = STATEF_SKILL5FAST }, // 156
    { I24SPR_PPOS, 6,                       5, NULL,                 I24ST(158), .flags = STATEF_SKILL5FAST }, // 157
    { I24SPR_PPOS, 6,                       5, &A_Pain,              I24ST(142), .flags = STATEF_SKILL5FAST }, // 158
    { I24SPR_PPOS, 7,                       0, &A_FaceTarget,        I24ST(160) }, // 159
    { I24SPR_PPOS, 7,                       5, &A_SpawnObject,       I24ST(161), .args = { I24MOARG(13), 11468800, 0, 0, 2621440, 131072, 0, 98304 } }, // 160
    { I24SPR_PPOS, 8,                       5, &A_Scream,            I24ST(162) }, // 161
    { I24SPR_PPOS, 9,                       5, &A_Fall,              I24ST(163) }, // 162
    { I24SPR_PPOS, 10,                      5, NULL,                 I24ST(164) }, // 163
    { I24SPR_PPOS, 11,                      5, NULL,                 I24ST(165) }, // 164
    { I24SPR_PPOS, 12,                     -1, NULL,                 S_NULL     }, // 165
    { I24SPR_PPOS, 13,                      5, NULL,                 I24ST(167) }, // 166
    { I24SPR_PPOS, 14,                      5, &A_XScream,           I24ST(168) }, // 167
    { I24SPR_PPOS, 15,                      5, &A_Fall,              I24ST(169) }, // 168
    { I24SPR_PPOS, 16,                      0, &A_FaceTarget,        I24ST(170) }, // 169
    { I24SPR_PPOS, 16,                      5, &A_SpawnObject,       I24ST(171), .args = { I24MOARG(14), 11141120, 0, -524288, 2097152, 262144, 0, 131072 } }, // 170
    { I24SPR_PPOS, 17,                      5, NULL,                 I24ST(172) }, // 171
    { I24SPR_PPOS, 18,                      5, NULL,                 I24ST(173) }, // 172
    { I24SPR_PPOS, 19,                      5, NULL,                 I24ST(174) }, // 173
    { I24SPR_PPOS, 20,                     -1, NULL,                 S_NULL     }, // 174
    { I24SPR_PPOS, 12,                      5, NULL,                 I24ST(176) }, // 175
    { I24SPR_PPOS, 11,                      5, NULL,                 I24ST(177) }, // 176
    { I24SPR_PPOS, 10,                      5, NULL,                 I24ST(178) }, // 177
    { I24SPR_PPOS, 9,                       5, NULL,                 I24ST(179) }, // 178
    { I24SPR_PPOS, 8,                       5, NULL,                 I24ST(180) }, // 179
    { I24SPR_PPOS, 7,                       5, NULL,                 I24ST(142) }, // 180
    { I24SPR_PHED, 0,                       3, NULL,                 I24ST(182) }, // 181
    { I24SPR_PHED, 1,                       3, NULL,                 I24ST(183) }, // 182
    { I24SPR_PHED, 2,                       3, NULL,                 I24ST(184) }, // 183
    { I24SPR_PHED, 3,                       3, NULL,                 I24ST(185) }, // 184
    { I24SPR_PHED, 4,                       3, NULL,                 I24ST(186) }, // 185
    { I24SPR_PHED, 5,                       3, NULL,                 I24ST(187) }, // 186
    { I24SPR_PHED, 6,                       3, NULL,                 I24ST(188) }, // 187
    { I24SPR_PHED, 7,                       3, NULL,                 I24ST(189) }, // 188
    { I24SPR_PHED, 8,                       3, NULL,                 I24ST(181) }, // 189
    { I24SPR_PHED, 9,                      -1, NULL,                 I24ST(190) }, // 190
    { I24SPR_PPOS, 21,                     -1, NULL,                 I24ST(191) }, // 191
    { I24SPR_PPOS, 22,                      5, NULL,                 I24ST(193) }, // 192
    { I24SPR_PPOS, 23,                     -1, NULL,                 S_NULL     }, // 193

    // Vassago
    { I24SPR_VASS, 0,                      10, &A_Look,              I24ST(195) }, // 194
    { I24SPR_VASS, 1,                      10, &A_Look,              I24ST(194) }, // 195
    { I24SPR_VASS, 0,                       3, &A_Chase,             I24ST(197) }, // 196
    { I24SPR_VASS, 0,                       3, &A_Chase,             I24ST(198) }, // 197
    { I24SPR_VASS, 1,                       3, &A_Chase,             I24ST(199) }, // 198
    { I24SPR_VASS, 1,                       3, &A_Chase,             I24ST(200) }, // 199
    { I24SPR_VASS, 2,                       3, &A_Chase,             I24ST(201) }, // 200
    { I24SPR_VASS, 2,                       3, &A_Chase,             I24ST(202) }, // 201
    { I24SPR_VASS, 3,                       3, &A_Chase,             I24ST(203) }, // 202
    { I24SPR_VASS, 3,                       3, &A_Chase,             I24ST(196) }, // 203
    { I24SPR_VASS, (4 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(205), .misc1 = I24SFX(33) }, // 204
    { I24SPR_VASS, (4 | FF_FULLBRIGHT),     8, &A_FaceTarget,        I24ST(206) }, // 205
    { I24SPR_VASS, (5 | FF_FULLBRIGHT),     4, &A_FaceTarget,        I24ST(207) }, // 206
    { I24SPR_VASS, (6 | FF_FULLBRIGHT),     4, &A_FaceTarget,        I24ST(208) }, // 207
    { I24SPR_VASS, (7 | FF_FULLBRIGHT),     8, &A_MonsterProjectile, I24ST(196), .args = { I24MOARG(15) } }, // 208
    { I24SPR_VASS, 8,                       2, NULL,                 I24ST(210) }, // 209
    { I24SPR_VASS, 8,                       2, &A_Pain,              I24ST(196) }, // 210
    { I24SPR_VASS, (9 | FF_FULLBRIGHT),     8, NULL,                 I24ST(212) }, // 211
    { I24SPR_VASS, (10 | FF_FULLBRIGHT),    8, &A_Scream,            I24ST(213) }, // 212
    { I24SPR_VASS, (11 | FF_FULLBRIGHT),    7, NULL,                 I24ST(214) }, // 213
    { I24SPR_VASS, (12 | FF_FULLBRIGHT),    6, &A_Fall,              I24ST(215) }, // 214
    { I24SPR_VASS, (13 | FF_FULLBRIGHT),    6, NULL,                 I24ST(216) }, // 215
    { I24SPR_VASS, (14 | FF_FULLBRIGHT),    6, NULL,                 I24ST(217) }, // 216
    { I24SPR_VASS, (15 | FF_FULLBRIGHT),    7, NULL,                 I24ST(218) }, // 217
    { I24SPR_VASS, 16,                     -1, &A_BossDeath,         S_NULL     }, // 218
    { I24SPR_VASS, 15,                      8, NULL,                 I24ST(220) }, // 219
    { I24SPR_VASS, 14,                      8, NULL,                 I24ST(221) }, // 220
    { I24SPR_VASS, 13,                      8, NULL,                 I24ST(222) }, // 221
    { I24SPR_VASS, 12,                      8, NULL,                 I24ST(223) }, // 222
    { I24SPR_VASS, 11,                      8, NULL,                 I24ST(224) }, // 223
    { I24SPR_VASS, 10,                      8, NULL,                 I24ST(225) }, // 224
    { I24SPR_VASS, 9,                       8, NULL,                 I24ST(196) }, // 225
    { I24SPR_VFLM, FF_FULLBRIGHT,           4, NULL,                 I24ST(227) }, // 226
    { I24SPR_VFLM, (1 | FF_FULLBRIGHT),     4, NULL,                 I24ST(226) }, // 227
    { I24SPR_VFLM, (2 | FF_FULLBRIGHT),     0, &A_RemoveFlags,       I24ST(229), .args = { 512 } }, // 228
    { I24SPR_VFLM, (2 | FF_FULLBRIGHT),     0, &A_RemoveFlags,       I24ST(230), .args = { 16 } }, // 229
    { I24SPR_VFLM, (2 | FF_FULLBRIGHT),     0, &A_RandomJump,        I24ST(231), .misc1 = I24ST(232), .misc2 = 128 }, // 230
    { I24SPR_VFLM, (2 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(233), .misc1 = I24SFX(21) }, // 231
    { I24SPR_VFLM, (2 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(233), .misc1 = I24SFX(22) }, // 232
    { I24SPR_VFLM, (2 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(234), .args = { 10, 128 } }, // 233
    { I24SPR_VFLM, (3 | FF_FULLBRIGHT),     4, NULL,                 I24ST(235) }, // 234
    { I24SPR_VFLM, (4 | FF_FULLBRIGHT),     4, NULL,                 I24ST(236) }, // 235
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(237), .misc1 = I24SFX(23) }, // 236
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(238), .args = { 10, 128 } }, // 237
    { I24SPR_VFLM, (6 | FF_FULLBRIGHT),     4, NULL,                 I24ST(239) }, // 238
    { I24SPR_VFLM, (7 | FF_FULLBRIGHT),     4, NULL,                 I24ST(240) }, // 239
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(241), .misc1 = I24SFX(22) }, // 240
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(242), .args = { 10, 128 } }, // 241
    { I24SPR_VFLM, (6 | FF_FULLBRIGHT),     4, NULL,                 I24ST(243) }, // 242
    { I24SPR_VFLM, (7 | FF_FULLBRIGHT),     4, NULL,                 I24ST(244) }, // 243
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(245), .misc1 = I24SFX(23) }, // 244
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(246), .args = { 10, 128 } }, // 245
    { I24SPR_VFLM, (6 | FF_FULLBRIGHT),     4, NULL,                 I24ST(247) }, // 246
    { I24SPR_VFLM, (7 | FF_FULLBRIGHT),     4, NULL,                 I24ST(248) }, // 247
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(249), .misc1 = I24SFX(21) }, // 248
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(250), .args = { 10, 128 } }, // 249
    { I24SPR_VFLM, (6 | FF_FULLBRIGHT),     4, NULL,                 I24ST(251) }, // 250
    { I24SPR_VFLM, (7 | FF_FULLBRIGHT),     4, NULL,                 I24ST(252) }, // 251
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(253), .misc1 = I24SFX(22) }, // 252
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(254), .args = { 10, 128 } }, // 253
    { I24SPR_VFLM, (6 | FF_FULLBRIGHT),     4, NULL,                 I24ST(255) }, // 254
    { I24SPR_VFLM, (7 | FF_FULLBRIGHT),     4, NULL,                 I24ST(256) }, // 255
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(257), .misc1 = I24SFX(21) }, // 256
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(258), .args = { 10, 128 } }, // 257
    { I24SPR_VFLM, (6 | FF_FULLBRIGHT),     4, NULL,                 I24ST(259) }, // 258
    { I24SPR_VFLM, (7 | FF_FULLBRIGHT),     4, NULL,                 I24ST(260) }, // 259
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(261), .misc1 = I24SFX(22) }, // 260
    { I24SPR_VFLM, (5 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(262), .args = { 10, 128 } }, // 261
    { I24SPR_VFLM, (6 | FF_FULLBRIGHT),     4, NULL,                 I24ST(263) }, // 262
    { I24SPR_VFLM, (7 | FF_FULLBRIGHT),     4, NULL,                 I24ST(264) }, // 263
    { I24SPR_VFLM, (8 | FF_FULLBRIGHT),     0, &A_PlaySound,         I24ST(265), .misc1 = I24SFX(23) }, // 264
    { I24SPR_VFLM, (8 | FF_FULLBRIGHT),     4, &A_RadiusDamage,      I24ST(266), .args = { 10, 128 } }, // 265
    { I24SPR_VFLM, (9 | FF_FULLBRIGHT),     4, NULL,                 I24ST(267) }, // 266
    { I24SPR_VFLM, (10 | FF_FULLBRIGHT),    4, NULL,                 I24ST(268) }, // 267
    { I24SPR_VFLM, (11 | FF_FULLBRIGHT),    4, &A_RadiusDamage,      I24ST(269), .args = { 10, 128 } }, // 268
    { I24SPR_VFLM, (12 | FF_FULLBRIGHT),    4, NULL,                 I24ST(270) }, // 269
    { I24SPR_VFLM, (13 | FF_FULLBRIGHT),    4, NULL,                 I24ST(271) }, // 270
    { I24SPR_VFLM, (14 | FF_FULLBRIGHT),    4, NULL,                 I24ST(272) }, // 271
    { I24SPR_VFLM, (15 | FF_FULLBRIGHT),    4, NULL,                 I24ST(273) }, // 272
    { I24SPR_VFLM, (16 | FF_FULLBRIGHT),    4, NULL,                 S_NULL     }, // 273

    // Tyrant
    { I24SPR_CYB2, 0,                      10, &A_Look,              I24ST(275) }, // 274
    { I24SPR_CYB2, 1,                      10, &A_Look,              I24ST(274) }, // 275
    { I24SPR_CYB2, 0,                       3, &A_Hoof,              I24ST(277) }, // 276
    { I24SPR_CYB2, 0,                       3, &A_Chase,             I24ST(278) }, // 277
    { I24SPR_CYB2, 1,                       3, &A_Chase,             I24ST(279) }, // 278
    { I24SPR_CYB2, 1,                       3, &A_Chase,             I24ST(280) }, // 279
    { I24SPR_CYB2, 2,                       3, &A_Chase,             I24ST(281) }, // 280
    { I24SPR_CYB2, 2,                       3, &A_Chase,             I24ST(282) }, // 281
    { I24SPR_CYB2, 3,                       0, &A_PlaySound,         I24ST(283), .misc1 = I24SFX(31) }, // 282
    { I24SPR_CYB2, 3,                       3, &A_Chase,             I24ST(284) }, // 283
    { I24SPR_CYB2, 3,                       3, &A_Chase,             I24ST(276) }, // 284
    { I24SPR_CYB2, 4,                       6, &A_FaceTarget,        I24ST(286) }, // 285
    { I24SPR_CYB2, (5 | FF_FULLBRIGHT),    12, &A_CyberAttack,       I24ST(287) }, // 286
    { I24SPR_CYB2, 4,                      12, &A_FaceTarget,        I24ST(288) }, // 287
    { I24SPR_CYB2, (5 | FF_FULLBRIGHT),    12, &A_CyberAttack,       I24ST(289) }, // 288
    { I24SPR_CYB2, 4,                      12, &A_FaceTarget,        I24ST(290) }, // 289
    { I24SPR_CYB2, (5 | FF_FULLBRIGHT),    12, &A_CyberAttack,       I24ST(276) }, // 290
    { I24SPR_CYB2, 6,                      10, &A_Pain,              I24ST(276) }, // 291
    { I24SPR_CYB2, 7,                      10, NULL,                 I24ST(293) }, // 292
    { I24SPR_CYB2, 8,                      10, &A_Scream,            I24ST(294) }, // 293
    { I24SPR_CYB2, 9,                      10, NULL,                 I24ST(295) }, // 294
    { I24SPR_CYB2, 10,                     10, NULL,                 I24ST(296) }, // 295
    { I24SPR_CYB2, 11,                     10, NULL,                 I24ST(297) }, // 296
    { I24SPR_CYB2, 12,                     10, &A_Fall,              I24ST(298) }, // 297
    { I24SPR_CYB2, 13,                     10, NULL,                 I24ST(299) }, // 298
    { I24SPR_CYB2, 14,                     10, NULL,                 I24ST(300) }, // 299
    { I24SPR_CYB2, 15,                     30, NULL,                 I24ST(301) }, // 300
    { I24SPR_CYB2, 15,                     -1, &A_BossDeath,         S_NULL     }, // 301
    { I24SPR_FLMG, 0,                       1, &A_WeaponReady,       I24ST(302) }, // 302
    { I24SPR_FLMG, 0,                       1, &A_Lower,             I24ST(303) }, // 303
    { I24SPR_FLMG, 0,                       1, &A_Raise,             I24ST(304) }, // 304
    { I24SPR_FLMF, FF_FULLBRIGHT,           0, &A_WeaponJump,        I24ST(306), .args = { I24ST(307), 128 } }, // 305
    { I24SPR_FLMF, FF_FULLBRIGHT,           0, &A_WeaponSound,       I24ST(308), .args = { I24SFX(19) } }, // 306
    { I24SPR_FLMF, FF_FULLBRIGHT,           0, &A_WeaponSound,       I24ST(308), .args = { I24SFX(20) } }, // 307
    { I24SPR_FLMF, FF_FULLBRIGHT,           0, &A_GunFlash,          I24ST(309) }, // 308
    { I24SPR_FLMF, FF_FULLBRIGHT,           0, &A_ConsumeAmmo,       I24ST(310), .args = { 1 } }, // 309
    { I24SPR_FLMF, FF_FULLBRIGHT,           1, &A_WeaponProjectile,  I24ST(311), .args = { I24MOARG(9) } }, // 310
    { I24SPR_FLMF, (1 | FF_FULLBRIGHT),     1, NULL,                 I24ST(312) }, // 311
    { I24SPR_FLMG, 0,                       1, NULL,                 I24ST(313) }, // 312
    { I24SPR_FLMG, 0,                       0, &A_ReFire,            I24ST(302) }, // 313
    { SPR_TNT1,    0,                       2, &A_Light2,            I24ST(315) }, // 314
    { SPR_TNT1,    0,                       1, &A_Light1,            1          }, // 315
    { SPR_TNT1,    FF_FULLBRIGHT,           1, NULL,                 I24ST(317) }, // 316
    { I24SPR_IFLM, FF_FULLBRIGHT,           2, NULL,                 I24ST(318) }, // 317
    { I24SPR_IFLM, (1 | FF_FULLBRIGHT),     2, &A_PlaySound,         I24ST(319), .misc1 = I24SFX(18) }, // 318
    { I24SPR_IFLM, (2 | FF_FULLBRIGHT),     2, NULL,                 I24ST(320) }, // 319
    { I24SPR_IFLM, (3 | FF_FULLBRIGHT),     2, NULL,                 I24ST(321) }, // 320
    { I24SPR_IFLM, (4 | FF_FULLBRIGHT),     2, NULL,                 I24ST(322) }, // 321
    { I24SPR_IFLM, (5 | FF_FULLBRIGHT),     2, NULL,                 I24ST(323) }, // 322
    { I24SPR_IFLM, (6 | FF_FULLBRIGHT),     2, NULL,                 I24ST(324) }, // 323
    { I24SPR_IFLM, (7 | FF_FULLBRIGHT),     2, NULL,                 S_NULL     }, // 324
    { I24SPR_IFLM, FF_FULLBRIGHT,           0, &A_RandomJump,        I24ST(326), .misc1 = I24ST(327), .misc2 = 128 }, // 325
    { I24SPR_IFLM, FF_FULLBRIGHT,           0, &A_PlaySound,         I24ST(328), .misc1 = I24SFX(21) }, // 326
    { I24SPR_IFLM, FF_FULLBRIGHT,           0, &A_PlaySound,         I24ST(328), .misc1 = I24SFX(22) }, // 327
    { I24SPR_IFLM, (8 | FF_FULLBRIGHT),     2, &A_RadiusDamage,      I24ST(329), .args = { 5, 64 } }, // 328
    { I24SPR_IFLM, (9 | FF_FULLBRIGHT),     2, NULL,                 I24ST(330) }, // 329
    { I24SPR_IFLM, (8 | FF_FULLBRIGHT),     2, NULL,                 I24ST(331) }, // 330
    { I24SPR_IFLM, (9 | FF_FULLBRIGHT),     2, &A_RadiusDamage,      I24ST(332), .args = { 5, 64 } }, // 331
    { I24SPR_IFLM, (10 | FF_FULLBRIGHT),    2, NULL,                 I24ST(333) }, // 332
    { I24SPR_IFLM, (9 | FF_FULLBRIGHT),     2, NULL,                 I24ST(334) }, // 333
    { I24SPR_IFLM, (10 | FF_FULLBRIGHT),    2, &A_RadiusDamage,      I24ST(335), .args = { 5, 64 } }, // 334
    { I24SPR_IFLM, (11 | FF_FULLBRIGHT),    2, NULL,                 I24ST(336) }, // 335
    { I24SPR_IFLM, (10 | FF_FULLBRIGHT),    2, &A_PlaySound,         I24ST(337), .misc1 = I24SFX(23) }, // 336
    { I24SPR_IFLM, (11 | FF_FULLBRIGHT),    2, &A_RadiusDamage,      I24ST(338), .args = { 5, 64 } }, // 337
    { I24SPR_IFLM, (12 | FF_FULLBRIGHT),    2, NULL,                 I24ST(339) }, // 338
    { I24SPR_IFLM, (11 | FF_FULLBRIGHT),    2, NULL,                 I24ST(340) }, // 339
    { I24SPR_IFLM, (12 | FF_FULLBRIGHT),    2, &A_RadiusDamage,      I24ST(341), .args = { 5, 64 } }, // 340
    { I24SPR_IFLM, (13 | FF_FULLBRIGHT),    2, NULL,                 I24ST(342) }, // 341
    { I24SPR_IFLM, (12 | FF_FULLBRIGHT),    2, NULL,                 I24ST(343) }, // 342
    { I24SPR_IFLM, (13 | FF_FULLBRIGHT),    2, &A_RadiusDamage,      I24ST(344), .args = { 5, 64 } }, // 343
    { I24SPR_IFLM, (14 | FF_FULLBRIGHT),    2, NULL,                 I24ST(345) }, // 344
    { I24SPR_IFLM, (13 | FF_FULLBRIGHT),    2, NULL,                 I24ST(346) }, // 345
    { I24SPR_IFLM, (14 | FF_FULLBRIGHT),    2, NULL,                 I24ST(347) }, // 346
    { I24SPR_IFLM, (15 | FF_FULLBRIGHT),    2, NULL,                 I24ST(348) }, // 347
    { I24SPR_IFLM, (14 | FF_FULLBRIGHT),    2, NULL,                 I24ST(349) }, // 348
    { I24SPR_IFLM, (15 | FF_FULLBRIGHT),    2, NULL,                 S_NULL     }, // 349
    { I24SPR_INCN, 0,                      -1, NULL,                 S_NULL     }, // 350
    { I24SPR_HETG, 0,                       1, &A_WeaponReady,       I24ST(351) }, // 351
    { I24SPR_HETG, 0,                       1, &A_Lower,             I24ST(352) }, // 352
    { I24SPR_HETG, 0,                       1, &A_Raise,             I24ST(353) }, // 353
    { I24SPR_HETG, 0,                       0, &A_ConsumeAmmo,       I24ST(355) }, // 354
    { I24SPR_HETG, 0,                       0, &A_GunFlashTo,        I24ST(356), .args = { I24ST(429), 1 } }, // 355
    { I24SPR_HETG, 0,                      20, &A_WeaponSound,       I24ST(357), .args = { I24SFX(15) } }, // 356
    { I24SPR_HETG, 0,                       0, &A_CheckAmmo,         I24ST(358), .args = { I24ST(419) } }, // 357
    { I24SPR_HETG, 0,                       0, &A_RefireTo,          I24ST(419), .args = { I24ST(359) } }, // 358
    { I24SPR_HETG, 0,                       0, &A_ConsumeAmmo,       I24ST(360) }, // 359
    { I24SPR_HETG, 0,                       0, &A_GunFlashTo,        I24ST(361), .args = { I24ST(433), 1 } }, // 360
    { I24SPR_HETG, 0,                      20, &A_WeaponSound,       I24ST(362), .args = { I24SFX(15) } }, // 361
    { I24SPR_HETG, 0,                       0, &A_CheckAmmo,         I24ST(363), .args = { I24ST(413) } }, // 362
    { I24SPR_HETG, 0,                       0, &A_RefireTo,          I24ST(413), .args = { I24ST(364) } }, // 363
    { I24SPR_HETG, 0,                       0, &A_ConsumeAmmo,       I24ST(365) }, // 364
    { I24SPR_HETG, 0,                       0, &A_GunFlashTo,        I24ST(366), .args = { I24ST(437), 1 } }, // 365
    { I24SPR_HETG, 0,                      20, &A_WeaponSound,       I24ST(367), .args = { I24SFX(15) } }, // 366
    { I24SPR_HETG, 0,                       0, &A_CheckAmmo,         I24ST(368), .args = { I24ST(404) } }, // 367
    { I24SPR_HETG, 0,                       0, &A_RefireTo,          I24ST(404), .args = { I24ST(369) } }, // 368
    { I24SPR_HETG, 0,                       0, &A_ConsumeAmmo,       I24ST(370) }, // 369
    { I24SPR_HETG, 0,                       0, &A_GunFlashTo,        I24ST(371), .args = { I24ST(441), 1 } }, // 370
    { I24SPR_HETG, 0,                      20, &A_WeaponSound,       I24ST(372), .args = { I24SFX(15) } }, // 371
    { I24SPR_HETG, 0,                       0, &A_CheckAmmo,         I24ST(373), .args = { I24ST(392) } }, // 372
    { I24SPR_HETG, 0,                       0, &A_RefireTo,          I24ST(392), .args = { I24ST(374) } }, // 373
    { I24SPR_HETG, 0,                       0, &A_ConsumeAmmo,       I24ST(375) }, // 374
    { I24SPR_HETG, 0,                       0, &A_GunFlashTo,        I24ST(376), .args = { I24ST(445), 1 } }, // 375
    { I24SPR_HETG, 0,                      20, &A_WeaponSound,       I24ST(377), .args = { I24SFX(15) } }, // 376
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(378), .args = { I24MOARG(10), -2293760 } }, // 377
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(379), .args = { I24MOARG(10), -1966080 } }, // 378
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(380), .args = { I24MOARG(10), -1638400 } }, // 379
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(381), .args = { I24MOARG(10), -1310720 } }, // 380
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(382), .args = { I24MOARG(10), -983040 } }, // 381
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(383), .args = { I24MOARG(10), -655360 } }, // 382
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(384), .args = { I24MOARG(10), -327680 } }, // 383
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(385), .args = { I24MOARG(10) } }, // 384
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(386), .args = { I24MOARG(10), 327680 } }, // 385
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(387), .args = { I24MOARG(10), 655360 } }, // 386
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(388), .args = { I24MOARG(10), 983040 } }, // 387
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(389), .args = { I24MOARG(10), 1310720 } }, // 388
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(390), .args = { I24MOARG(10), 1638400 } }, // 389
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(391), .args = { I24MOARG(10), 1966080 } }, // 390
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(422), .args = { I24MOARG(10), 2293760 } }, // 391
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(393), .args = { I24MOARG(10), -1802240 } }, // 392
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(394), .args = { I24MOARG(10), -1474560 } }, // 393
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(395), .args = { I24MOARG(10), -1146880 } }, // 394
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(396), .args = { I24MOARG(10), -819200 } }, // 395
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(397), .args = { I24MOARG(10), -491520 } }, // 396
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(398), .args = { I24MOARG(10), -163840 } }, // 397
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(399), .args = { I24MOARG(10), 163840 } }, // 398
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(400), .args = { I24MOARG(10), 491520 } }, // 399
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(401), .args = { I24MOARG(10), 819200 } }, // 400
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(402), .args = { I24MOARG(10), 1146880 } }, // 401
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(403), .args = { I24MOARG(10), 1474560 } }, // 402
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(422), .args = { I24MOARG(10), 1802240 } }, // 403
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(405), .args = { I24MOARG(10), -1310720 } }, // 404
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(406), .args = { I24MOARG(10), -983040 } }, // 405
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(407), .args = { I24MOARG(10), -655360 } }, // 406
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(408), .args = { I24MOARG(10), -327680 } }, // 407
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(409), .args = { I24MOARG(10) } }, // 408
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(410), .args = { I24MOARG(10), 327680 } }, // 409
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(411), .args = { I24MOARG(10), 655360 } }, // 410
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(412), .args = { I24MOARG(10), 983040 } }, // 411
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(422), .args = { I24MOARG(10), 1310720 } }, // 412
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(414), .args = { I24MOARG(10), -819200 } }, // 413
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(415), .args = { I24MOARG(10), -491520 } }, // 414
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(416), .args = { I24MOARG(10), -163840 } }, // 415
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(417), .args = { I24MOARG(10), 163840 } }, // 416
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(418), .args = { I24MOARG(10), 491520 } }, // 417
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(422), .args = { I24MOARG(10), 819200 } }, // 418
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(420), .args = { I24MOARG(10), -327680 } }, // 419
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(421), .args = { I24MOARG(10) } }, // 420
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponProjectile,  I24ST(422), .args = { I24MOARG(10), 327680 } }, // 421
    { I24SPR_HETF, FF_FULLBRIGHT,           0, &A_WeaponSound,       I24ST(423), .args = { I24SFX(16) } }, // 422
    { I24SPR_HETF, FF_FULLBRIGHT,           3, &A_GunFlash,          I24ST(424) }, // 423
    { I24SPR_HETF, (1 | FF_FULLBRIGHT),     5, NULL,                 I24ST(425) }, // 424
    { I24SPR_HETG, 3,                       4, NULL,                 I24ST(426) }, // 425
    { I24SPR_HETG, 2,                       4, NULL,                 I24ST(427) }, // 426
    { I24SPR_HETG, 1,                       4, NULL,                 I24ST(428) }, // 427
    { I24SPR_HETG, 0,                       0, &A_ReFire,            I24ST(351) }, // 428
    { I24SPR_HETC, FF_FULLBRIGHT,           6, NULL,                 I24ST(430) }, // 429
    { I24SPR_HETC, (1 | FF_FULLBRIGHT),     5, NULL,                 I24ST(431) }, // 430
    { I24SPR_HETC, (2 | FF_FULLBRIGHT),     5, NULL,                 I24ST(432) }, // 431
    { I24SPR_HETC, (3 | FF_FULLBRIGHT),     5, NULL,                 1          }, // 432
    { I24SPR_HETC, (4 | FF_FULLBRIGHT),     6, NULL,                 I24ST(434) }, // 433
    { I24SPR_HETC, (5 | FF_FULLBRIGHT),     5, NULL,                 I24ST(435) }, // 434
    { I24SPR_HETC, (6 | FF_FULLBRIGHT),     5, NULL,                 I24ST(436) }, // 435
    { I24SPR_HETC, (7 | FF_FULLBRIGHT),     5, NULL,                 1          }, // 436
    { I24SPR_HETC, (8 | FF_FULLBRIGHT),     6, NULL,                 I24ST(438) }, // 437
    { I24SPR_HETC, (9 | FF_FULLBRIGHT),     5, NULL,                 I24ST(439) }, // 438
    { I24SPR_HETC, (10 | FF_FULLBRIGHT),    5, NULL,                 I24ST(440) }, // 439
    { I24SPR_HETC, (11 | FF_FULLBRIGHT),    5, NULL,                 1          }, // 440
    { I24SPR_HETC, (12 | FF_FULLBRIGHT),    6, NULL,                 I24ST(442) }, // 441
    { I24SPR_HETC, (13 | FF_FULLBRIGHT),    5, NULL,                 I24ST(443) }, // 442
    { I24SPR_HETC, (14 | FF_FULLBRIGHT),    5, NULL,                 I24ST(444) }, // 443
    { I24SPR_HETC, (15 | FF_FULLBRIGHT),    5, NULL,                 1          }, // 444
    { I24SPR_HETC, (16 | FF_FULLBRIGHT),    6, NULL,                 I24ST(446) }, // 445
    { I24SPR_HETC, (17 | FF_FULLBRIGHT),    5, NULL,                 I24ST(447) }, // 446
    { I24SPR_HETC, (18 | FF_FULLBRIGHT),    5, NULL,                 I24ST(448) }, // 447
    { I24SPR_HETC, (19 | FF_FULLBRIGHT),    5, NULL,                 1          }, // 448
    { SPR_TNT1,    0,                       3, &A_Light1,            I24ST(450) }, // 449
    { SPR_TNT1,    0,                       5, &A_Light2,            1          }, // 450
    { SPR_TNT1,    0,                       0, NULL,                 I24ST(452) }, // 451
    { SPR_TNT1,    0,                       1, &A_SpawnObject,       S_NULL, .args = { I24MOARG(11), 0, 0, 0, 0, 1310720 } }, // 452
    { I24SPR_HETB, FF_FULLBRIGHT,           3, NULL,                 I24ST(454) }, // 453
    { I24SPR_HETB, (1 | FF_FULLBRIGHT),     3, NULL,                 I24ST(455) }, // 454
    { I24SPR_HETB, (2 | FF_FULLBRIGHT),     3, NULL,                 I24ST(453) }, // 455
    { I24SPR_HETB, (3 | FF_FULLBRIGHT),     3, NULL,                 I24ST(457) }, // 456
    { I24SPR_HETB, (4 | FF_FULLBRIGHT),     3, NULL,                 I24ST(458) }, // 457
    { I24SPR_HETB, (5 | FF_FULLBRIGHT),     3, NULL,                 I24ST(459) }, // 458
    { I24SPR_HETB, (6 | FF_FULLBRIGHT),     3, NULL,                 I24ST(460) }, // 459
    { I24SPR_HETB, (7 | FF_FULLBRIGHT),     3, NULL,                 I24ST(461) }, // 460
    { I24SPR_HETB, (8 | FF_FULLBRIGHT),     3, NULL,                 S_NULL     }, // 461
    { I24SPR_CBLD, 0,                      -1, NULL,                 S_NULL     }, // 462
    { I24SPR_FCPU, 0,                      -1, NULL,                 S_NULL     }, // 463
    { I24SPR_FTNK, 0,                      -1, NULL,                 S_NULL     }, // 464
};

static const mobjinfo_t id24_mobjs[NUMID24MOBJS] =
{
    // Ghoul (ID24 thing 0)
    {
        /* doomednum            */ -28672,
        /* spawnstate           */ I24ST(42),
        /* spawnhealth          */ 50,
        /* gibhealth            */ -50,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(44),
        /* seesound             */ I24SFX(14),
        /* reactiontime         */ 8,
        /* attacksound          */ sfx_none,
        /* painstate            */ I24ST(56),
        /* painchance           */ 128,
        /* painsound            */ I24SFX(13),
        /* meleestate           */ S_NULL,
        /* missilestate         */ I24ST(52),
        /* deathstate           */ I24ST(58),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(12),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 12,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 40 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 50,
        /* damage               */ 0,
        /* activesound          */ I24SFX(11),
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_NOGRAVITY | MF_FLOAT | MF_COUNTKILL),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "ghoul",
        /* plural1              */ "ghouls",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Banshee (ID24 thing 1)
    {
        /* doomednum            */ -28671,
        /* spawnstate           */ I24ST(73),
        /* spawnhealth          */ 100,
        /* gibhealth            */ -100,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(75),
        /* seesound             */ I24SFX(0),
        /* reactiontime         */ 8,
        /* attacksound          */ sfx_none,
        /* painstate            */ I24ST(95),
        /* painchance           */ 64,
        /* painsound            */ I24SFX(2),
        /* meleestate           */ I24ST(94),
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(97),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(1),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 8,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 56 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 500,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_NOGRAVITY | MF_FLOAT | MF_COUNTKILL),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "banshee",
        /* plural1              */ "banshees",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Mindweaver (ID24 thing 2)
    {
        /* doomednum            */ -28670,
        /* spawnstate           */ I24ST(103),
        /* spawnhealth          */ 500,
        /* gibhealth            */ -500,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(105),
        /* seesound             */ I24SFX(6),
        /* reactiontime         */ 8,
        /* attacksound          */ sfx_none,
        /* painstate            */ I24ST(124),
        /* painchance           */ 40,
        /* painsound            */ sfx_dmpain,
        /* meleestate           */ S_NULL,
        /* missilestate         */ I24ST(120),
        /* deathstate           */ I24ST(126),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(5),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 12,
        /* radius               */ 64 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 64 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 600,
        /* damage               */ 0,
        /* activesound          */ I24SFX(4),
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ I24ST(133),
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "mindweaver",
        /* plural1              */ "mindweavers",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Shocktrooper (ID24 thing 3)
    {
        /* doomednum            */ -28669,
        /* spawnstate           */ I24ST(140),
        /* spawnhealth          */ 100,
        /* gibhealth            */ -100,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(142),
        /* seesound             */ sfx_none,
        /* reactiontime         */ 8,
        /* attacksound          */ sfx_none,
        /* painstate            */ I24ST(157),
        /* painchance           */ 30,
        /* painsound            */ I24SFX(28),
        /* meleestate           */ S_NULL,
        /* missilestate         */ I24ST(150),
        /* deathstate           */ I24ST(159),
        /* xdeathstate          */ I24ST(166),
        /* deathsound           */ I24SFX(26),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 10,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 56 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ I24SFX(25),
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ I24ST(175),
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "shocktrooper",
        /* plural1              */ "shocktroopers",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Vassago (ID24 thing 4)
    {
        /* doomednum            */ -28668,
        /* spawnstate           */ I24ST(194),
        /* spawnhealth          */ 1000,
        /* gibhealth            */ -1000,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(196),
        /* seesound             */ I24SFX(36),
        /* reactiontime         */ 8,
        /* attacksound          */ sfx_none,
        /* painstate            */ I24ST(209),
        /* painchance           */ 100,
        /* painsound            */ I24SFX(35),
        /* meleestate           */ I24ST(204),
        /* missilestate         */ I24ST(204),
        /* deathstate           */ I24ST(211),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(34),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 8,
        /* radius               */ 24 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 64 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 1000,
        /* damage               */ 0,
        /* activesound          */ I24SFX(32),
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ I24ST(219),
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ 2,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "vassago",
        /* plural1              */ "vassagos",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Tyrant (ID24 thing 5)
    {
        /* doomednum            */ -28667,
        /* spawnstate           */ I24ST(274),
        /* spawnhealth          */ 1000,
        /* gibhealth            */ -1000,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(276),
        /* seesound             */ I24SFX(30),
        /* reactiontime         */ 8,
        /* attacksound          */ sfx_none,
        /* painstate            */ I24ST(291),
        /* painchance           */ 20,
        /* painsound            */ sfx_dmpain,
        /* meleestate           */ S_NULL,
        /* missilestate         */ I24ST(285),
        /* deathstate           */ I24ST(292),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(29),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 16,
        /* radius               */ 40 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 110 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 1000,
        /* damage               */ 0,
        /* activesound          */ sfx_dmact,
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ (MF_MBF21_NORADIUSDMG | MF_MBF21_HIGHERMPROB | MF_MBF21_RANGEHALF | MF_MBF21_BOSS | MF_MBF21_E2M8BOSS | MF_MBF21_E4M6BOSS | MF_MBF21_FULLVOLSOUNDS),
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ 3,
        /* splashgroup          */ 3,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "tyrant",
        /* plural1              */ "tyrants",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Tyrant (Boss 1) (ID24 thing 6)
    {
        /* doomednum            */ -28666,
        /* spawnstate           */ I24ST(274),
        /* spawnhealth          */ 1000,
        /* gibhealth            */ -1000,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(276),
        /* seesound             */ I24SFX(30),
        /* reactiontime         */ 8,
        /* attacksound          */ sfx_none,
        /* painstate            */ I24ST(291),
        /* painchance           */ 20,
        /* painsound            */ sfx_dmpain,
        /* meleestate           */ S_NULL,
        /* missilestate         */ I24ST(285),
        /* deathstate           */ I24ST(292),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(29),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 16,
        /* radius               */ 40 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 110 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 1000,
        /* damage               */ 0,
        /* activesound          */ sfx_dmact,
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ (MF_MBF21_NORADIUSDMG | MF_MBF21_HIGHERMPROB | MF_MBF21_RANGEHALF | MF_MBF21_BOSS | MF_MBF21_E2M8BOSS | MF_MBF21_E4M6BOSS | MF_MBF21_FULLVOLSOUNDS),
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ 3,
        /* splashgroup          */ 3,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 2100,
        /* respawndice          */ 64,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "tyrant boss 1",
        /* plural1              */ "tyrant boss 1s",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Tyrant (Boss 2) (ID24 thing 7)
    {
        /* doomednum            */ -28665,
        /* spawnstate           */ I24ST(274),
        /* spawnhealth          */ 1000,
        /* gibhealth            */ -1000,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(276),
        /* seesound             */ I24SFX(30),
        /* reactiontime         */ 8,
        /* attacksound          */ sfx_none,
        /* painstate            */ I24ST(291),
        /* painchance           */ 20,
        /* painsound            */ sfx_dmpain,
        /* meleestate           */ S_NULL,
        /* missilestate         */ I24ST(285),
        /* deathstate           */ I24ST(292),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(29),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 16,
        /* radius               */ 40 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 110 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 1000,
        /* damage               */ 0,
        /* activesound          */ sfx_dmact,
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ (MF_MBF21_NORADIUSDMG | MF_MBF21_HIGHERMPROB | MF_MBF21_RANGEHALF | MF_MBF21_BOSS | MF_MBF21_E2M8BOSS | MF_MBF21_E4M6BOSS | MF_MBF21_FULLVOLSOUNDS),
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ 3,
        /* splashgroup          */ 3,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 2100,
        /* respawndice          */ 64,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "tyrant boss 2",
        /* plural1              */ "tyrant boss 2s",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Incinerator Flame (ID24 thing 8)
    {
        /* doomednum            */ -1,
        /* spawnstate           */ I24ST(316),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(325),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 40 * FRACUNIT,
        /* radius               */ 13 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 5,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_NOBLOCKMAP | MF_NOGRAVITY | MF_DROPOFF | MF_MISSILE | MF_TRANSLUCENT),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ MF_MBF21_FORCERADIUSDMG,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Heatwave Spawner (ID24 thing 9)
    {
        /* doomednum            */ -1,
        /* spawnstate           */ I24ST(451),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(452),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(17),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_NOBLOCKMAP | MF_NOGRAVITY | MF_DROPOFF | MF_MISSILE),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ MF_MBF21_RIP,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Heatwave Ripper (ID24 thing 10)
    {
        /* doomednum            */ -1,
        /* spawnstate           */ I24ST(453),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(456),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(17),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 20 * FRACUNIT,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 10,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_NOBLOCKMAP | MF_NOGRAVITY | MF_DROPOFF | MF_MISSILE | MF_TRANSLUCENT),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ MF_MBF21_RIP,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Ghoul Ball (ID24 thing 11)
    {
        /* doomednum            */ -1,
        /* spawnstate           */ I24ST(66),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_firsht,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(68),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_firxpl,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 15 * FRACUNIT,
        /* radius               */ 6 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 3,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_NOBLOCKMAP | MF_NOGRAVITY | MF_DROPOFF | MF_MISSILE | MF_TRANSLUCENT),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ 1310720,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Shocktrooper Head (ID24 thing 12)
    {
        /* doomednum            */ -1,
        /* spawnstate           */ I24ST(181),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(190),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(27),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 8 * FRACUNIT,
        /* radius               */ 6 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_DROPOFF | MF_MISSILE),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ MF_MBF21_LOGRAV,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Shocktrooper Torso (ID24 thing 13)
    {
        /* doomednum            */ -1,
        /* spawnstate           */ I24ST(191),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(192),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 8 * FRACUNIT,
        /* radius               */ 6 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_NOBLOCKMAP | MF_DROPOFF | MF_MISSILE),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Vassago Flame (ID24 thing 14)
    {
        /* doomednum            */ -1,
        /* spawnstate           */ I24ST(226),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ I24SFX(18),
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(228),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_flame,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 15 * FRACUNIT,
        /* radius               */ 6 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 5,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_NOBLOCKMAP | MF_NOGRAVITY | MF_DROPOFF | MF_MISSILE | MF_TRANSLUCENT),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ 2,
        /* ripsound             */ sfx_none,
        /* altspeed             */ 1310720,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Stalagmite (gray) (ID24 thing 15)
    {
        /* doomednum            */ -28664,
        /* spawnstate           */ I24ST(15),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "stalagmite (gray)",
        /* plural1              */ "stalagmite (gray)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Large corpse pile (ID24 thing 16)
    {
        /* doomednum            */ -28663,
        /* spawnstate           */ I24ST(0),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 40 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "large corpse pile",
        /* plural1              */ "large corpse pile",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Human BBQ 1 (ID24 thing 17)
    {
        /* doomednum            */ -28662,
        /* spawnstate           */ I24ST(1),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "human bbq 1",
        /* plural1              */ "human bbq 1",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Human BBQ 2 (ID24 thing 18)
    {
        /* doomednum            */ -28661,
        /* spawnstate           */ I24ST(4),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "human bbq 2",
        /* plural1              */ "human bbq 2",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging victim, both legs (ID24 thing 19)
    {
        /* doomednum            */ -28660,
        /* spawnstate           */ I24ST(7),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 80 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging victim, both legs",
        /* plural1              */ "hanging victim, both legs",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging victim, both legs (blocking) (ID24 thing 20)
    {
        /* doomednum            */ -28659,
        /* spawnstate           */ I24ST(7),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 80 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging victim, both legs (blocking)",
        /* plural1              */ "hanging victim, both legs (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging victim, crucified (ID24 thing 21)
    {
        /* doomednum            */ -28658,
        /* spawnstate           */ I24ST(8),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 64 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging victim, crucified",
        /* plural1              */ "hanging victim, crucified",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging victim, crucified (blocking) (ID24 thing 22)
    {
        /* doomednum            */ -28657,
        /* spawnstate           */ I24ST(8),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 64 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging victim, crucified (blocking)",
        /* plural1              */ "hanging victim, crucified (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging victim, arms bound (ID24 thing 23)
    {
        /* doomednum            */ -28656,
        /* spawnstate           */ I24ST(9),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 72 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging victim, arms bound",
        /* plural1              */ "hanging victim, arms bound",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging victim, arms bound (blocking) (ID24 thing 24)
    {
        /* doomednum            */ -28655,
        /* spawnstate           */ I24ST(9),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 72 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging victim, arms bound (blocking)",
        /* plural1              */ "hanging victim, arms bound (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging baron of hell (ID24 thing 25)
    {
        /* doomednum            */ -28654,
        /* spawnstate           */ I24ST(10),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 80 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging baron of hell",
        /* plural1              */ "hanging baron of hell",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging baron of hell (blocking) (ID24 thing 26)
    {
        /* doomednum            */ -28653,
        /* spawnstate           */ I24ST(10),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 80 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging baron of hell (blocking)",
        /* plural1              */ "hanging baron of hell (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging victim, chained (ID24 thing 27)
    {
        /* doomednum            */ -28652,
        /* spawnstate           */ I24ST(11),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 84 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging victim, chained",
        /* plural1              */ "hanging victim, chained",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging victim, chained (blocking) (ID24 thing 28)
    {
        /* doomednum            */ -28651,
        /* spawnstate           */ I24ST(11),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 84 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging victim, chained (blocking)",
        /* plural1              */ "hanging victim, chained (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging torso, chained (ID24 thing 29)
    {
        /* doomednum            */ -28650,
        /* spawnstate           */ I24ST(12),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 52 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging torso, chained",
        /* plural1              */ "hanging torso, chained",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Hanging torso, chained (blocking) (ID24 thing 30)
    {
        /* doomednum            */ -28649,
        /* spawnstate           */ I24ST(12),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 52 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "hanging torso, chained (blocking)",
        /* plural1              */ "hanging torso, chained (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Skull pole trio (ID24 thing 31)
    {
        /* doomednum            */ -28648,
        /* spawnstate           */ I24ST(13),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "skull pole trio",
        /* plural1              */ "skull pole trio",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Skull gibs (ID24 thing 32)
    {
        /* doomednum            */ -28647,
        /* spawnstate           */ I24ST(14),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "skull gibs",
        /* plural1              */ "skull gibs",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Bush, short (ID24 thing 33)
    {
        /* doomednum            */ -28646,
        /* spawnstate           */ I24ST(16),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "bush, short",
        /* plural1              */ "bush, short",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Bush, short burned 1 (ID24 thing 34)
    {
        /* doomednum            */ -28645,
        /* spawnstate           */ I24ST(17),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "bush, short burned 1",
        /* plural1              */ "bush, short burned 1",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Bush, short burned 2 (ID24 thing 35)
    {
        /* doomednum            */ -28644,
        /* spawnstate           */ I24ST(18),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "bush, short burned 2",
        /* plural1              */ "bush, short burned 2",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Bush, tall (ID24 thing 36)
    {
        /* doomednum            */ -28643,
        /* spawnstate           */ I24ST(19),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "bush, tall",
        /* plural1              */ "bush, tall",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Bush, tall burned 1 (ID24 thing 37)
    {
        /* doomednum            */ -28642,
        /* spawnstate           */ I24ST(20),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "bush, tall burned 1",
        /* plural1              */ "bush, tall burned 1",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Bush, tall burned 2 (ID24 thing 38)
    {
        /* doomednum            */ -28641,
        /* spawnstate           */ I24ST(21),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "bush, tall burned 2",
        /* plural1              */ "bush, tall burned 2",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave rock column (ID24 thing 39)
    {
        /* doomednum            */ -28640,
        /* spawnstate           */ I24ST(22),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 24 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave rock column",
        /* plural1              */ "cave rock column",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalagmite, large (ID24 thing 40)
    {
        /* doomednum            */ -28639,
        /* spawnstate           */ I24ST(23),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalagmite, large",
        /* plural1              */ "cave stalagmite, large",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalagmite, medium (ID24 thing 41)
    {
        /* doomednum            */ -28638,
        /* spawnstate           */ I24ST(24),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalagmite, medium",
        /* plural1              */ "cave stalagmite, medium",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalagmite, small (ID24 thing 42)
    {
        /* doomednum            */ -28637,
        /* spawnstate           */ I24ST(25),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalagmite, small",
        /* plural1              */ "cave stalagmite, small",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalactite, large (ID24 thing 43)
    {
        /* doomednum            */ -28636,
        /* spawnstate           */ I24ST(26),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 112 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalactite, large",
        /* plural1              */ "cave stalactite, large",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalactite, large (blocking) (ID24 thing 44)
    {
        /* doomednum            */ -28635,
        /* spawnstate           */ I24ST(26),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 112 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalactite, large (blocking)",
        /* plural1              */ "cave stalactite, large (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalactite, medium (ID24 thing 45)
    {
        /* doomednum            */ -28634,
        /* spawnstate           */ I24ST(27),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 64 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalactite, medium",
        /* plural1              */ "cave stalactite, medium",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalactite, medium (blocking) (ID24 thing 46)
    {
        /* doomednum            */ -28633,
        /* spawnstate           */ I24ST(27),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 64 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalactite, medium (blocking)",
        /* plural1              */ "cave stalactite, medium (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalactite, small (ID24 thing 47)
    {
        /* doomednum            */ -28632,
        /* spawnstate           */ I24ST(28),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 32 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalactite, small",
        /* plural1              */ "cave stalactite, small",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Cave stalactite, small (blocking) (ID24 thing 48)
    {
        /* doomednum            */ -28631,
        /* spawnstate           */ I24ST(28),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 32 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "cave stalactite, small (blocking)",
        /* plural1              */ "cave stalactite, small (blocking)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Office chair (ID24 thing 49)
    {
        /* doomednum            */ -28630,
        /* spawnstate           */ I24ST(29),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "office chair",
        /* plural1              */ "office chair",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Office lamp (breakable) (ID24 thing 50)
    {
        /* doomednum            */ -28629,
        /* spawnstate           */ I24ST(30),
        /* spawnhealth          */ 20,
        /* gibhealth            */ -20,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ I24ST(31),
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ I24SFX(3),
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 52 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 65536,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SOLID | MF_SHOOTABLE | MF_NOBLOOD),
        /* flags2               */ (MF2_FOOTCLIP | MF2_CASTSHADOW | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "office lamp (breakable)",
        /* plural1              */ "office lamp (breakable)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Ceiling lamp (ID24 thing 51)
    {
        /* doomednum            */ -28628,
        /* spawnstate           */ I24ST(35),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 32 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ (MF_SPAWNCEILING | MF_NOGRAVITY),
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "ceiling lamp",
        /* plural1              */ "ceiling lamp",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Candelabra (short) (ID24 thing 52)
    {
        /* doomednum            */ -28627,
        /* spawnstate           */ I24ST(36),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ S_NULL,
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ MF_SOLID,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "candelabra (short)",
        /* plural1              */ "candelabra (short)",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Ambient Klaxon (ID24 thing 53)
    {
        /* doomednum            */ -28626,
        /* spawnstate           */ I24ST(37),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(38),
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "ambient klaxon",
        /* plural1              */ "ambient klaxon",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Ambient Portal Open (ID24 thing 54)
    {
        /* doomednum            */ -28625,
        /* spawnstate           */ I24ST(37),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(39),
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "ambient portal open",
        /* plural1              */ "ambient portal open",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Ambient Portal Loop (ID24 thing 55)
    {
        /* doomednum            */ -28624,
        /* spawnstate           */ I24ST(37),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(40),
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "ambient portal loop",
        /* plural1              */ "ambient portal loop",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    },

    // Ambient Portal Close (ID24 thing 56)
    {
        /* doomednum            */ -28623,
        /* spawnstate           */ I24ST(37),
        /* spawnhealth          */ 0,
        /* gibhealth            */ 0,
        /* giblevel             */ 0,
        /* seestate             */ I24ST(41),
        /* seesound             */ sfx_none,
        /* reactiontime         */ 0,
        /* attacksound          */ sfx_none,
        /* painstate            */ S_NULL,
        /* painchance           */ 0,
        /* painsound            */ sfx_none,
        /* meleestate           */ S_NULL,
        /* missilestate         */ S_NULL,
        /* deathstate           */ S_NULL,
        /* xdeathstate          */ S_NULL,
        /* deathsound           */ sfx_none,
        /* droppeditem          */ MT_NULL,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 0,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 0,
        /* damage               */ 0,
        /* activesound          */ sfx_none,
        /* flags                */ 0,
        /* flags2               */ (MF2_FOOTCLIP | MF2_NOLIQUIDBOB),
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* bloodcolor           */ REDBLOOD,
        /* shadowoffset         */ 0,
        /* mbf21flags           */ 0,
        /* infightinggroup      */ IG_DEFAULT,
        /* projectilegroup      */ PG_DEFAULT,
        /* splashgroup          */ SG_DEFAULT,
        /* ripsound             */ sfx_none,
        /* altspeed             */ NO_ALTSPEED,
        /* meleerange           */ MELEERANGE,
        /* id24flags            */ 0,
        /* minrespawntics       */ 420,
        /* respawndice          */ 4,
        /* selfdamage           */ 1 * FRACUNIT,
        /* pickupammotype       */ am_noammo,
        /* pickupammocategory   */ PI_NOITEM,
        /* pickupweapontype     */ 0,
        /* pickupitemtype       */ PI_NOITEM,
        /* pickupbonuscount     */ 0,
        /* pickupsound          */ sfx_none,
        /* pickupstringmnemonic */ "",
        /* name1                */ "ambient portal close",
        /* plural1              */ "ambient portal close",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ "",
        /* angleoffset          */ 0
    }
};

bool    id24res;
int     id24firstmobj;
int     id24nummobjs;

static int  sprbase;
static int  statebase;
static int  sfxbase;
static int  mobjbase;

static int ID24_Remap(const int value)
{
    const int   offset = (value & 0x0FFFFFFF);

    if (value < 0x10000000 || value >= 0x60000000)
        return value;

    switch (value >> 28)
    {
        case 1:
            return sprbase + offset;

        case 2:
            return statebase + offset;

        case 3:
            return sfxbase + offset;

        case 5:
            return mobjbase + offset;
    }

    return value;
}

bool ID24_IsPlaceable(const int type)
{
    return (id24nummobjs && type >= id24firstmobj && type < id24firstmobj + id24nummobjs
        && *mobjinfo[type].name1);
}

void ID24_Init(void)
{
    const int   oldnumsprites = numsprites;
    const int   oldnumstates = numstates;
    const int   oldnumsfx = numsfx;
    const int   oldnummobjtypes = nummobjtypes;

    if (!id24res || id24nummobjs)
        return;

    sprbase = oldnumsprites;
    statebase = oldnumstates;
    sfxbase = oldnumsfx;
    mobjbase = oldnummobjtypes;

    dsdh_EnsureSpritesCapacity(oldnumsprites + NUMID24SPRITES);

    for (int i = 0; i < NUMID24SPRITES; i++)
        sprnames[sprbase + i] = M_StringDuplicate(id24_sprnames[i]);

    numsprites = oldnumsprites + NUMID24SPRITES;

    dsdh_EnsureSFXCapacity(oldnumsfx + NUMID24SFX);

    for (int i = 0; i < NUMID24SFX; i++)
    {
        sfxinfo_t   *sfx = &s_sfx[sfxbase + i];

        M_StringCopy(sfx->name1, id24_sfxnames[i], sizeof(sfx->name1));
        M_StringCopy(sfx->name2, id24_sfxnames[i], sizeof(sfx->name2));
        sfx->singularity = sg_none;
        sfx->priority = 127;
        sfx->lumpnum = -1;
    }

    numsfx = oldnumsfx + NUMID24SFX;

    dsdh_EnsureStatesCapacity(oldnumstates + NUMID24STATES);

    for (int i = 0; i < NUMID24STATES; i++)
    {
        state_t *state = &states[statebase + i];

        *state = id24_states[i];
        state->sprite = ID24_Remap(state->sprite);
        state->nextstate = ID24_Remap(state->nextstate);
        state->misc1 = ID24_Remap(state->misc1);
        state->misc2 = ID24_Remap(state->misc2);

        for (int j = 0; j < MAXSTATEARGS; j++)
            state->args[j] = ID24_Remap(state->args[j]);

        deh_codeptr[statebase + i] = state->action;

        // stop D_PostProcessDeh() replacing the arguments with defaults
        defined_codeptr_args[statebase + i] = 0xFF;
    }

    numstates = oldnumstates + NUMID24STATES;

    dsdh_EnsureMobjInfoCapacity(oldnummobjtypes + NUMID24MOBJS);

    for (int i = 0; i < NUMID24MOBJS; i++)
    {
        mobjinfo_t  *info = &mobjinfo[mobjbase + i];

        *info = id24_mobjs[i];
        info->spawnstate = ID24_Remap(info->spawnstate);
        info->seestate = ID24_Remap(info->seestate);
        info->seesound = ID24_Remap(info->seesound);
        info->attacksound = ID24_Remap(info->attacksound);
        info->painstate = ID24_Remap(info->painstate);
        info->painsound = ID24_Remap(info->painsound);
        info->meleestate = ID24_Remap(info->meleestate);
        info->missilestate = ID24_Remap(info->missilestate);
        info->deathstate = ID24_Remap(info->deathstate);
        info->xdeathstate = ID24_Remap(info->xdeathstate);
        info->deathsound = ID24_Remap(info->deathsound);
        info->activesound = ID24_Remap(info->activesound);
        info->raisestate = ID24_Remap(info->raisestate);
        info->ripsound = ID24_Remap(info->ripsound);
    }

    nummobjtypes = oldnummobjtypes + NUMID24MOBJS;
    id24firstmobj = oldnummobjtypes;
    id24nummobjs = NUMID24MOBJS;
}
