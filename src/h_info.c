/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2018 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
  <https://github.com/bradharding/doomretro/wiki/CREDITS>.

  This file is part of DOOM Retro.

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
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#include "d_player.h"
#include "r_defs.h"

char *hereticsprnames[] =
{
    "IMPX", "ACLO", "PTN1", "SHLD", "SHD2", "BAGH", "SPMP", "INVS", "PTN2", "SOAR",
    "INVU", "PWBK", "EGGC", "EGGM", "FX01", "SPHL", "TRCH", "FBMB", "XPL1", "ATLP",
    "PPOD", "AMG1", "SPSH", "LVAS", "SLDG", "SKH1", "SKH2", "SKH3", "SKH4", "CHDL",
    "SRTC", "SMPL", "STGS", "STGL", "STCS", "STCL", "KFR1", "BARL", "BRPL", "MOS1",
    "MOS2", "WTRH", "HCOR", "KGZ1", "KGZB", "KGZG", "KGZY", "VLCO", "VFBL", "VTFB",
    "SFFI", "TGLT", "TELE", "STFF", "PUF3", "PUF4", "BEAK", "WGNT", "GAUN", "PUF1",
    "WBLS", "BLSR", "FX18", "FX17", "WMCE", "MACE", "FX02", "WSKL", "HROD", "FX00",
    "FX20", "FX21", "FX22", "FX23", "GWND", "PUF2", "WPHX", "PHNX", "FX04", "FX08",
    "FX09", "WBOW", "CRBW", "FX03", "BLOD", "PLAY", "FDTH", "BSKL", "CHKN", "MUMM",
    "FX15", "BEAS", "FRB1", "SNKE", "SNFX", "HEAD", "FX05", "FX06", "FX07", "CLNK",
    "WZRD", "FX11", "FX10", "KNIG", "SPAX", "RAXE", "SRCR", "FX14", "SOR2", "SDTH",
    "FX16", "MNTR", "FX12", "FX13", "AKYY", "BKYY", "CKYY", "AMG2", "AMM1", "AMM2",
    "AMC1", "AMC2", "AMS1", "AMS2", "AMP1", "AMP2", "AMB1", "AMB2",

    "BLD2",     // [BH] blood splats

    "SP00", "SP01", "SP02", "SP03", "SP04", "SP05", "SP06", "SP07", "SP08", "SP09",
    "SP10", "SP11", "SP12", "SP13", "SP14", "SP15", "SP16", "SP17", "SP18", "SP19",
    "SP20", "SP21", "SP22", "SP23", "SP24", "SP25", "SP26", "SP27", "SP28", "SP29",
    "SP30", "SP31", "SP32", "SP33", "SP34", "SP35", "SP36", "SP37", "SP38", "SP39",
    "SP40", "SP41", "SP42", "SP43", "SP44", "SP45", "SP46", "SP47", "SP48", "SP49",
    "SP50", "SP51", "SP52", "SP53", "SP54", "SP55", "SP56", "SP57", "SP58", "SP59",
    "SP60", "SP61", "SP62", "SP63", "SP64", "SP65", "SP66", "SP67", "SP68", "SP69",
    "SP70", "SP71", "SP72", "SP73", "SP74", "SP75", "SP76", "SP77", "SP78", "SP79",
    "SP80", "SP81", "SP82", "SP83", "SP84", "SP85", "SP86", "SP87", "SP88", "SP89",
    "SP90", "SP91", "SP92", "SP93", "SP94", "SP95", "SP96", "SP97", "SP98", "SP99",

    NULL
};

//void A_FreeTargMobj(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_RestoreSpecialThing1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_RestoreSpecialThing2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_HideThing(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_UnHideThing(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_RestoreArtifact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Explode(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_PodPain(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_RemovePod(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MakePod(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_InitKeyGizmo(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_VolcanoSet(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_VolcanoBlast(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BeastPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_VolcBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SpawnTeleGlitter(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SpawnTeleGlitter2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_AccTeleGlitter(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light0(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_StaffAttackPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_StaffAttackPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BeakReady(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BeakRaise(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BeakAttackPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BeakAttackPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_GauntletAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireBlasterPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireBlasterPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SpawnRippers(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireMacePL1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireMacePL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MacePL1Check(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MaceBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MaceBallImpact2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_DeathBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireSkullRodPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireSkullRodPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SkullRodPL2Seek(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_AddPlayerRain(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_HideInCeiling(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SkullRodStorm(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_RainImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireGoldWandPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireGoldWandPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FirePhoenixPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_InitPhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FirePhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ShutdownPhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_PhoenixPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_RemovedPhoenixFunc(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FlameEnd(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FloatPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireCrossbowPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FireCrossbowPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BoltSpark(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_NoBlocking(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_AddPlayerCorpse(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullPop(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_FlameSnd(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_CheckBurnGone(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_CheckSkullFloor(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_CheckSkullDone(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_Feathers(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ChicLook(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ChicChase(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ChicPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ChicAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MummyAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MummyAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MummySoul(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ContMobjSound(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MummyFX1Seek(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BeastAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SnakeAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SnakeAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_HeadAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BossDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_HeadIceImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_HeadFireGrow(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_WhirlwindSeek(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ClinkAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_WizAtk1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_WizAtk2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_WizAtk3(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_GhostOff(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ImpMeAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ImpMsAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ImpMsAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ImpDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ImpXDeath1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ImpXDeath2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ImpExplode(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_KnightAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_DripBlood(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_Sor1Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_Sor1Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_Srcr1Attack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SorZap(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SorcererRise(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SorRise(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SorSightSnd(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_Srcr2Decide(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_Srcr2Attack(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_Sor2DthInit(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SorDSph(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_Sor2DthLoop(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SorDExp(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_SorDBon(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_BlueSpark(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_GenWizard(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MinotaurAtk1(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MinotaurDecide(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MinotaurAtk2(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MinotaurAtk3(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MinotaurCharge(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_MntrFloorFire(mobj_t *actor, player_t *player, pspdef_t *psp);
//void A_ESound(mobj_t *actor, player_t *player, pspdef_t *psp);

state_t hereticstates[NUMHSTATES] =
{
  //  sprite     frame               tics  action                          nextstate                 state
    { HSPR_IMPX, 0,                    -1, NULL,                           HS_NULL             }, // HS_NULL
    { HSPR_ACLO, 4,                  1050, NULL/*A_FreeTargMobj*/,         HS_NULL             }, // HS_FREETARGMOBJ
    { HSPR_PTN1, 0,                     3, NULL,                           HS_ITEM_PTN1_2      }, // HS_ITEM_PTN1_1
    { HSPR_PTN1, 1,                     3, NULL,                           HS_ITEM_PTN1_3      }, // HS_ITEM_PTN1_2
    { HSPR_PTN1, 2,                     3, NULL,                           HS_ITEM_PTN1_1      }, // HS_ITEM_PTN1_3
    { HSPR_SHLD, 0,                    -1, NULL,                           HS_NULL             }, // HS_ITEM_SHLD1
    { HSPR_SHD2, 0,                    -1, NULL,                           HS_NULL             }, // HS_ITEM_SHD2_1
    { HSPR_BAGH, 0,                    -1, NULL,                           HS_NULL             }, // HS_ITEM_BAGH1
    { HSPR_SPMP, 0,                    -1, NULL,                           HS_NULL             }, // HS_ITEM_SPMP1
    { HSPR_ACLO, 4,                  1400, NULL,                           HS_HIDESPECIAL2     }, // HS_HIDESPECIAL1
    { HSPR_ACLO, 0,                     4, NULL/*A_RestoreSpecialThing1*/, HS_HIDESPECIAL3     }, // HS_HIDESPECIAL2
    { HSPR_ACLO, 1,                     4, NULL,                           HS_HIDESPECIAL4     }, // HS_HIDESPECIAL3
    { HSPR_ACLO, 0,                     4, NULL,                           HS_HIDESPECIAL5     }, // HS_HIDESPECIAL4
    { HSPR_ACLO, 1,                     4, NULL,                           HS_HIDESPECIAL6     }, // HS_HIDESPECIAL5
    { HSPR_ACLO, 2,                     4, NULL,                           HS_HIDESPECIAL7     }, // HS_HIDESPECIAL6
    { HSPR_ACLO, 1,                     4, NULL,                           HS_HIDESPECIAL8     }, // HS_HIDESPECIAL7
    { HSPR_ACLO, 2,                     4, NULL,                           HS_HIDESPECIAL9     }, // HS_HIDESPECIAL8
    { HSPR_ACLO, 3,                     4, NULL,                           HS_HIDESPECIAL10    }, // HS_HIDESPECIAL9
    { HSPR_ACLO, 2,                     4, NULL,                           HS_HIDESPECIAL11    }, // HS_HIDESPECIAL10
    { HSPR_ACLO, 3,                     4, NULL/*A_RestoreSpecialThing2*/, HS_NULL             }, // HS_HIDESPECIAL11
    { HSPR_ACLO, 3,                     3, NULL,                           HS_DORMANTARTI2     }, // HS_DORMANTARTI1
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DORMANTARTI3     }, // HS_DORMANTARTI2
    { HSPR_ACLO, 3,                     3, NULL,                           HS_DORMANTARTI4     }, // HS_DORMANTARTI3
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DORMANTARTI5     }, // HS_DORMANTARTI4
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DORMANTARTI6     }, // HS_DORMANTARTI5
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DORMANTARTI7     }, // HS_DORMANTARTI6
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DORMANTARTI8     }, // HS_DORMANTARTI7
    { HSPR_ACLO, 0,                     3, NULL,                           HS_DORMANTARTI9     }, // HS_DORMANTARTI8
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DORMANTARTI10    }, // HS_DORMANTARTI9
    { HSPR_ACLO, 0,                     3, NULL,                           HS_DORMANTARTI11    }, // HS_DORMANTARTI10
    { HSPR_ACLO, 0,                  1400, NULL/*A_HideThing*/,            HS_DORMANTARTI12    }, // HS_DORMANTARTI11
    { HSPR_ACLO, 0,                     3, NULL/*A_UnHideThing*/,          HS_DORMANTARTI13    }, // HS_DORMANTARTI12
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DORMANTARTI14    }, // HS_DORMANTARTI13
    { HSPR_ACLO, 0,                     3, NULL,                           HS_DORMANTARTI15    }, // HS_DORMANTARTI14
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DORMANTARTI16    }, // HS_DORMANTARTI15
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DORMANTARTI17    }, // HS_DORMANTARTI16
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DORMANTARTI18    }, // HS_DORMANTARTI17
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DORMANTARTI19    }, // HS_DORMANTARTI18
    { HSPR_ACLO, 3,                     3, NULL,                           HS_DORMANTARTI20    }, // HS_DORMANTARTI19
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DORMANTARTI21    }, // HS_DORMANTARTI20
    { HSPR_ACLO, 3,                     3, NULL/*A_RestoreArtifact*/,      HS_NULL             }, // HS_DORMANTARTI21
    { HSPR_ACLO, 3,                     3, NULL,                           HS_DEADARTI2        }, // HS_DEADARTI1
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DEADARTI3        }, // HS_DEADARTI2
    { HSPR_ACLO, 3,                     3, NULL,                           HS_DEADARTI4        }, // HS_DEADARTI3
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DEADARTI5        }, // HS_DEADARTI4
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DEADARTI6        }, // HS_DEADARTI5
    { HSPR_ACLO, 2,                     3, NULL,                           HS_DEADARTI7        }, // HS_DEADARTI6
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DEADARTI8        }, // HS_DEADARTI7
    { HSPR_ACLO, 0,                     3, NULL,                           HS_DEADARTI9        }, // HS_DEADARTI8
    { HSPR_ACLO, 1,                     3, NULL,                           HS_DEADARTI10       }, // HS_DEADARTI9
    { HSPR_ACLO, 0,                     3, NULL,                           HS_NULL             }, // HS_DEADARTI10
    { HSPR_INVS, 0 | FF_FULLBRIGHT,   350, NULL,                           HS_ARTI_INVS1       }, // HS_ARTI_INVS1
    { HSPR_PTN2, 0,                     4, NULL,                           HS_ARTI_PTN2_2      }, // HS_ARTI_PTN2_1
    { HSPR_PTN2, 1,                     4, NULL,                           HS_ARTI_PTN2_3      }, // HS_ARTI_PTN2_2
    { HSPR_PTN2, 2,                     4, NULL,                           HS_ARTI_PTN2_1      }, // HS_ARTI_PTN2_3
    { HSPR_SOAR, 0,                     5, NULL,                           HS_ARTI_SOAR2       }, // HS_ARTI_SOAR1
    { HSPR_SOAR, 1,                     5, NULL,                           HS_ARTI_SOAR3       }, // HS_ARTI_SOAR2
    { HSPR_SOAR, 2,                     5, NULL,                           HS_ARTI_SOAR4       }, // HS_ARTI_SOAR3
    { HSPR_SOAR, 1,                     5, NULL,                           HS_ARTI_SOAR1       }, // HS_ARTI_SOAR4
    { HSPR_INVU, 0,                     3, NULL,                           HS_ARTI_INVU2       }, // HS_ARTI_INVU1
    { HSPR_INVU, 1,                     3, NULL,                           HS_ARTI_INVU3       }, // HS_ARTI_INVU2
    { HSPR_INVU, 2,                     3, NULL,                           HS_ARTI_INVU4       }, // HS_ARTI_INVU3
    { HSPR_INVU, 3,                     3, NULL,                           HS_ARTI_INVU1       }, // HS_ARTI_INVU4
    { HSPR_PWBK, 0,                   350, NULL,                           HS_ARTI_PWBK1       }, // HS_ARTI_PWBK1
    { HSPR_EGGC, 0,                     6, NULL,                           HS_ARTI_EGGC2       }, // HS_ARTI_EGGC1
    { HSPR_EGGC, 1,                     6, NULL,                           HS_ARTI_EGGC3       }, // HS_ARTI_EGGC2
    { HSPR_EGGC, 2,                     6, NULL,                           HS_ARTI_EGGC4       }, // HS_ARTI_EGGC3
    { HSPR_EGGC, 1,                     6, NULL,                           HS_ARTI_EGGC1       }, // HS_ARTI_EGGC4
    { HSPR_EGGM, 0,                     4, NULL,                           HS_EGGFX2           }, // HS_EGGFX1
    { HSPR_EGGM, 1,                     4, NULL,                           HS_EGGFX3           }, // HS_EGGFX2
    { HSPR_EGGM, 2,                     4, NULL,                           HS_EGGFX4           }, // HS_EGGFX3
    { HSPR_EGGM, 3,                     4, NULL,                           HS_EGGFX5           }, // HS_EGGFX4
    { HSPR_EGGM, 4,                     4, NULL,                           HS_EGGFX1           }, // HS_EGGFX5
    { HSPR_FX01, 4 | FF_FULLBRIGHT,     3, NULL,                           HS_EGGFXI1_2        }, // HS_EGGFXI1_1
    { HSPR_FX01, 5 | FF_FULLBRIGHT,     3, NULL,                           HS_EGGFXI1_3        }, // HS_EGGFXI1_2
    { HSPR_FX01, 6 | FF_FULLBRIGHT,     3, NULL,                           HS_EGGFXI1_4        }, // HS_EGGFXI1_3
    { HSPR_FX01, 7 | FF_FULLBRIGHT,     3, NULL,                           HS_NULL             }, // HS_EGGFXI1_4
    { HSPR_SPHL, 0,                   350, NULL,                           HS_ARTI_SPHL1       }, // HS_ARTI_SPHL1
    { HSPR_TRCH, 0 | FF_FULLBRIGHT,     3, NULL,                           HS_ARTI_TRCH2       }, // HS_ARTI_TRCH1
    { HSPR_TRCH, 1 | FF_FULLBRIGHT,     3, NULL,                           HS_ARTI_TRCH3       }, // HS_ARTI_TRCH2
    { HSPR_TRCH, 2 | FF_FULLBRIGHT,     3, NULL,                           HS_ARTI_TRCH1       }, // HS_ARTI_TRCH3
    { HSPR_FBMB, 4,                   350, NULL,                           HS_ARTI_FBMB1       }, // HS_ARTI_FBMB1
    { HSPR_FBMB, 0,                    10, NULL,                           HS_FIREBOMB2        }, // HS_FIREBOMB1
    { HSPR_FBMB, 1,                    10, NULL,                           HS_FIREBOMB3        }, // HS_FIREBOMB2
    { HSPR_FBMB, 2,                    10, NULL,                           HS_FIREBOMB4        }, // HS_FIREBOMB3
    { HSPR_FBMB, 3,                    10, NULL,                           HS_FIREBOMB5        }, // HS_FIREBOMB4
    { HSPR_FBMB, 4,                     6, A_Scream,                       HS_FIREBOMB6        }, // HS_FIREBOMB5
    { HSPR_XPL1, 0 | FF_FULLBRIGHT,     4, A_Explode,                      HS_FIREBOMB7        }, // HS_FIREBOMB6
    { HSPR_XPL1, 1 | FF_FULLBRIGHT,     4, NULL,                           HS_FIREBOMB8        }, // HS_FIREBOMB7
    { HSPR_XPL1, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_FIREBOMB9        }, // HS_FIREBOMB8
    { HSPR_XPL1, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_FIREBOMB10       }, // HS_FIREBOMB9
    { HSPR_XPL1, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_FIREBOMB11       }, // HS_FIREBOMB10
    { HSPR_XPL1, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_FIREBOMB11
    { HSPR_ATLP, 0,                     4, NULL,                           HS_ARTI_ATLP2       }, // HS_ARTI_ATLP1
    { HSPR_ATLP, 1,                     4, NULL,                           HS_ARTI_ATLP3       }, // HS_ARTI_ATLP2
    { HSPR_ATLP, 2,                     4, NULL,                           HS_ARTI_ATLP4       }, // HS_ARTI_ATLP3
    { HSPR_ATLP, 1,                     4, NULL,                           HS_ARTI_ATLP1       }, // HS_ARTI_ATLP4
    { HSPR_PPOD, 0,                    10, NULL,                           HS_POD_WAIT1        }, // HS_POD_WAIT1
    { HSPR_PPOD, 1,                    14, NULL/*A_PodPain*/,              HS_POD_WAIT1        }, // HS_POD_PAIN1
    { HSPR_PPOD, 2 | FF_FULLBRIGHT,     5, NULL/*A_RemovePod*/,            HS_POD_DIE2         }, // HS_POD_DIE1
    { HSPR_PPOD, 3 | FF_FULLBRIGHT,     5, A_Scream,                       HS_POD_DIE3         }, // HS_POD_DIE2
    { HSPR_PPOD, 4 | FF_FULLBRIGHT,     5, A_Explode,                      HS_POD_DIE4         }, // HS_POD_DIE3
    { HSPR_PPOD, 5 | FF_FULLBRIGHT,    10, NULL,                           HS_FREETARGMOBJ     }, // HS_POD_DIE4
    { HSPR_PPOD, 8,                     3, NULL,                           HS_POD_GROW2        }, // HS_POD_GROW1
    { HSPR_PPOD, 9,                     3, NULL,                           HS_POD_GROW3        }, // HS_POD_GROW2
    { HSPR_PPOD, 10,                    3, NULL,                           HS_POD_GROW4        }, // HS_POD_GROW3
    { HSPR_PPOD, 11,                    3, NULL,                           HS_POD_GROW5        }, // HS_POD_GROW4
    { HSPR_PPOD, 12,                    3, NULL,                           HS_POD_GROW6        }, // HS_POD_GROW5
    { HSPR_PPOD, 13,                    3, NULL,                           HS_POD_GROW7        }, // HS_POD_GROW6
    { HSPR_PPOD, 14,                    3, NULL,                           HS_POD_GROW8        }, // HS_POD_GROW7
    { HSPR_PPOD, 15,                    3, NULL,                           HS_POD_WAIT1        }, // HS_POD_GROW8
    { HSPR_PPOD, 6,                     8, NULL,                           HS_PODGOO2          }, // HS_PODGOO1
    { HSPR_PPOD, 7,                     8, NULL,                           HS_PODGOO1          }, // HS_PODGOO2
    { HSPR_PPOD, 6,                    10, NULL,                           HS_NULL             }, // HS_PODGOOX
    { HSPR_AMG1, 0,                    35, NULL/*A_MakePod*/,              HS_PODGENERATOR     }, // HS_PODGENERATOR
    { HSPR_SPSH, 0,                     8, NULL,                           HS_SPLASH2          }, // HS_SPLASH1
    { HSPR_SPSH, 1,                     8, NULL,                           HS_SPLASH3          }, // HS_SPLASH2
    { HSPR_SPSH, 2,                     8, NULL,                           HS_SPLASH4          }, // HS_SPLASH3
    { HSPR_SPSH, 3,                    16, NULL,                           HS_NULL             }, // HS_SPLASH4
    { HSPR_SPSH, 3,                    10, NULL,                           HS_NULL             }, // HS_SPLASHX
    { HSPR_SPSH, 4,                     5, NULL,                           HS_SPLASHBASE2      }, // HS_SPLASHBASE1
    { HSPR_SPSH, 5,                     5, NULL,                           HS_SPLASHBASE3      }, // HS_SPLASHBASE2
    { HSPR_SPSH, 6,                     5, NULL,                           HS_SPLASHBASE4      }, // HS_SPLASHBASE3
    { HSPR_SPSH, 7,                     5, NULL,                           HS_SPLASHBASE5      }, // HS_SPLASHBASE4
    { HSPR_SPSH, 8,                     5, NULL,                           HS_SPLASHBASE6      }, // HS_SPLASHBASE5
    { HSPR_SPSH, 9,                     5, NULL,                           HS_SPLASHBASE7      }, // HS_SPLASHBASE6
    { HSPR_SPSH, 10,                    5, NULL,                           HS_NULL             }, // HS_SPLASHBASE7
    { HSPR_LVAS, 0 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASPLASH2      }, // HS_LAVASPLASH1
    { HSPR_LVAS, 1 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASPLASH3      }, // HS_LAVASPLASH2
    { HSPR_LVAS, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASPLASH4      }, // HS_LAVASPLASH3
    { HSPR_LVAS, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASPLASH5      }, // HS_LAVASPLASH4
    { HSPR_LVAS, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASPLASH6      }, // HS_LAVASPLASH5
    { HSPR_LVAS, 5 | FF_FULLBRIGHT,     5, NULL,                           HS_NULL             }, // HS_LAVASPLASH6
    { HSPR_LVAS, 6 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASMOKE2       }, // HS_LAVASMOKE1
    { HSPR_LVAS, 7 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASMOKE3       }, // HS_LAVASMOKE2
    { HSPR_LVAS, 8 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASMOKE4       }, // HS_LAVASMOKE3
    { HSPR_LVAS, 9 | FF_FULLBRIGHT,     5, NULL,                           HS_LAVASMOKE5       }, // HS_LAVASMOKE4
    { HSPR_LVAS, 10 | FF_FULLBRIGHT,    5, NULL,                           HS_NULL             }, // HS_LAVASMOKE5
    { HSPR_SLDG, 0,                     8, NULL,                           HS_SLUDGECHUNK2     }, // HS_SLUDGECHUNK1
    { HSPR_SLDG, 1,                     8, NULL,                           HS_SLUDGECHUNK3     }, // HS_SLUDGECHUNK2
    { HSPR_SLDG, 2,                     8, NULL,                           HS_SLUDGECHUNK4     }, // HS_SLUDGECHUNK3
    { HSPR_SLDG, 3,                     8, NULL,                           HS_NULL             }, // HS_SLUDGECHUNK4
    { HSPR_SLDG, 3,                     6, NULL,                           HS_NULL             }, // HS_SLUDGECHUNKX
    { HSPR_SLDG, 4,                     5, NULL,                           HS_SLUDGESPLASH2    }, // HS_SLUDGESPLASH1
    { HSPR_SLDG, 5,                     5, NULL,                           HS_SLUDGESPLASH3    }, // HS_SLUDGESPLASH2
    { HSPR_SLDG, 6,                     5, NULL,                           HS_SLUDGESPLASH4    }, // HS_SLUDGESPLASH3
    { HSPR_SLDG, 7,                     5, NULL,                           HS_NULL             }, // HS_SLUDGESPLASH4
    { HSPR_SKH1, 0,                    -1, NULL,                           HS_NULL             }, // HS_SKULLHANG70_1
    { HSPR_SKH2, 0,                    -1, NULL,                           HS_NULL             }, // HS_SKULLHANG60_1
    { HSPR_SKH3, 0,                    -1, NULL,                           HS_NULL             }, // HS_SKULLHANG45_1
    { HSPR_SKH4, 0,                    -1, NULL,                           HS_NULL             }, // HS_SKULLHANG35_1
    { HSPR_CHDL, 0,                     4, NULL,                           HS_CHANDELIER2      }, // HS_CHANDELIER1
    { HSPR_CHDL, 1,                     4, NULL,                           HS_CHANDELIER3      }, // HS_CHANDELIER2
    { HSPR_CHDL, 2,                     4, NULL,                           HS_CHANDELIER1      }, // HS_CHANDELIER3
    { HSPR_SRTC, 0,                     4, NULL,                           HS_SERPTORCH2       }, // HS_SERPTORCH1
    { HSPR_SRTC, 1,                     4, NULL,                           HS_SERPTORCH3       }, // HS_SERPTORCH2
    { HSPR_SRTC, 2,                     4, NULL,                           HS_SERPTORCH1       }, // HS_SERPTORCH3
    { HSPR_SMPL, 0,                    -1, NULL,                           HS_NULL             }, // HS_SMALLPILLAR
    { HSPR_STGS, 0,                    -1, NULL,                           HS_NULL             }, // HS_STALAGMITESMALL
    { HSPR_STGL, 0,                    -1, NULL,                           HS_NULL             }, // HS_STALAGMITELARGE
    { HSPR_STCS, 0,                    -1, NULL,                           HS_NULL             }, // HS_STALACTITESMALL
    { HSPR_STCL, 0,                    -1, NULL,                           HS_NULL             }, // HS_STALACTITELARGE
    { HSPR_KFR1, 0 | FF_FULLBRIGHT,     3, NULL,                           HS_FIREBRAZIER2     }, // HS_FIREBRAZIER1
    { HSPR_KFR1, 1 | FF_FULLBRIGHT,     3, NULL,                           HS_FIREBRAZIER3     }, // HS_FIREBRAZIER2
    { HSPR_KFR1, 2 | FF_FULLBRIGHT,     3, NULL,                           HS_FIREBRAZIER4     }, // HS_FIREBRAZIER3
    { HSPR_KFR1, 3 | FF_FULLBRIGHT,     3, NULL,                           HS_FIREBRAZIER5     }, // HS_FIREBRAZIER4
    { HSPR_KFR1, 4 | FF_FULLBRIGHT,     3, NULL,                           HS_FIREBRAZIER6     }, // HS_FIREBRAZIER5
    { HSPR_KFR1, 5 | FF_FULLBRIGHT,     3, NULL,                           HS_FIREBRAZIER7     }, // HS_FIREBRAZIER6
    { HSPR_KFR1, 6 | FF_FULLBRIGHT,     3, NULL,                           HS_FIREBRAZIER8     }, // HS_FIREBRAZIER7
    { HSPR_KFR1, 7 | FF_FULLBRIGHT,     3, NULL,                           HS_FIREBRAZIER1     }, // HS_FIREBRAZIER8
    { HSPR_BARL, 0,                    -1, NULL,                           HS_NULL             }, // HS_BARREL
    { HSPR_BRPL, 0,                    -1, NULL,                           HS_NULL             }, // HS_BRPILLAR
    { HSPR_MOS1, 0,                    -1, NULL,                           HS_NULL             }, // HS_MOSS1
    { HSPR_MOS2, 0,                    -1, NULL,                           HS_NULL             }, // HS_MOSS2
    { HSPR_WTRH, 0 | FF_FULLBRIGHT,     6, NULL,                           HS_WALLTORCH2       }, // HS_WALLTORCH1
    { HSPR_WTRH, 1 | FF_FULLBRIGHT,     6, NULL,                           HS_WALLTORCH3       }, // HS_WALLTORCH2
    { HSPR_WTRH, 2 | FF_FULLBRIGHT,     6, NULL,                           HS_WALLTORCH1       }, // HS_WALLTORCH3
    { HSPR_HCOR, 0,                    -1, NULL,                           HS_NULL             }, // HS_HANGINGCORPSE
    { HSPR_KGZ1, 0,                     1, NULL,                           HS_KEYGIZMO2        }, // HS_KEYGIZMO1
    { HSPR_KGZ1, 0,                     1, NULL/*A_InitKeyGizmo*/,         HS_KEYGIZMO3        }, // HS_KEYGIZMO2
    { HSPR_KGZ1, 0,                    -1, NULL,                           HS_NULL             }, // HS_KEYGIZMO3
    { HSPR_KGZB, 0,                     1, NULL,                           HS_KGZ_START        }, // HS_KGZ_START
    { HSPR_KGZB, 0 | FF_FULLBRIGHT,    -1, NULL,                           HS_NULL             }, // HS_KGZ_BLUEFLOAT1
    { HSPR_KGZG, 0 | FF_FULLBRIGHT,    -1, NULL,                           HS_NULL             }, // HS_KGZ_GREENFLOAT1
    { HSPR_KGZY, 0 | FF_FULLBRIGHT,    -1, NULL,                           HS_NULL             }, // HS_KGZ_YELLOWFLOAT1
    { HSPR_VLCO, 0,                   350, NULL,                           HS_VOLCANO2         }, // HS_VOLCANO1
    { HSPR_VLCO, 0,                    35, NULL/*A_VolcanoSet*/,           HS_VOLCANO3         }, // HS_VOLCANO2
    { HSPR_VLCO, 1,                     3, NULL,                           HS_VOLCANO4         }, // HS_VOLCANO3
    { HSPR_VLCO, 2,                     3, NULL,                           HS_VOLCANO5         }, // HS_VOLCANO4
    { HSPR_VLCO, 3,                     3, NULL,                           HS_VOLCANO6         }, // HS_VOLCANO5
    { HSPR_VLCO, 1,                     3, NULL,                           HS_VOLCANO7         }, // HS_VOLCANO6
    { HSPR_VLCO, 2,                     3, NULL,                           HS_VOLCANO8         }, // HS_VOLCANO7
    { HSPR_VLCO, 3,                     3, NULL,                           HS_VOLCANO9         }, // HS_VOLCANO8
    { HSPR_VLCO, 4,                    10, NULL/*A_VolcanoBlast*/,         HS_VOLCANO2         }, // HS_VOLCANO9
    { HSPR_VFBL, 0,                     4, NULL/*A_BeastPuff*/,            HS_VOLCANOBALL2     }, // HS_VOLCANOBALL1
    { HSPR_VFBL, 1,                     4, NULL/*A_BeastPuff*/,            HS_VOLCANOBALL1     }, // HS_VOLCANOBALL2
    { HSPR_XPL1, 0,                     4, NULL/*A_VolcBallImpact*/,       HS_VOLCANOBALLX2    }, // HS_VOLCANOBALLX1
    { HSPR_XPL1, 1,                     4, NULL,                           HS_VOLCANOBALLX3    }, // HS_VOLCANOBALLX2
    { HSPR_XPL1, 2,                     4, NULL,                           HS_VOLCANOBALLX4    }, // HS_VOLCANOBALLX3
    { HSPR_XPL1, 3,                     4, NULL,                           HS_VOLCANOBALLX5    }, // HS_VOLCANOBALLX4
    { HSPR_XPL1, 4,                     4, NULL,                           HS_VOLCANOBALLX6    }, // HS_VOLCANOBALLX5
    { HSPR_XPL1, 5,                     4, NULL,                           HS_NULL             }, // HS_VOLCANOBALLX6
    { HSPR_VTFB, 0,                     4, NULL,                           HS_VOLCANOTBALL2    }, // HS_VOLCANOTBALL1
    { HSPR_VTFB, 1,                     4, NULL,                           HS_VOLCANOTBALL1    }, // HS_VOLCANOTBALL2
    { HSPR_SFFI, 2,                     4, NULL,                           HS_VOLCANOTBALLX2   }, // HS_VOLCANOTBALLX1
    { HSPR_SFFI, 1,                     4, NULL,                           HS_VOLCANOTBALLX3   }, // HS_VOLCANOTBALLX2
    { HSPR_SFFI, 0,                     4, NULL,                           HS_VOLCANOTBALLX4   }, // HS_VOLCANOTBALLX3
    { HSPR_SFFI, 1,                     4, NULL,                           HS_VOLCANOTBALLX5   }, // HS_VOLCANOTBALLX4
    { HSPR_SFFI, 2,                     4, NULL,                           HS_VOLCANOTBALLX6   }, // HS_VOLCANOTBALLX5
    { HSPR_SFFI, 3,                     4, NULL,                           HS_VOLCANOTBALLX7   }, // HS_VOLCANOTBALLX6
    { HSPR_SFFI, 4,                     4, NULL,                           HS_NULL             }, // HS_VOLCANOTBALLX7
    { HSPR_TGLT, 0,                     8, NULL/*A_SpawnTeleGlitter*/,     HS_TELEGLITGEN1     }, // HS_TELEGLITGEN1
    { HSPR_TGLT, 5,                     8, NULL/*A_SpawnTeleGlitter2*/,    HS_TELEGLITGEN2     }, // HS_TELEGLITGEN2
    { HSPR_TGLT, 0 | FF_FULLBRIGHT,     2, NULL,                           HS_TELEGLITTER1_2   }, // HS_TELEGLITTER1_1
    { HSPR_TGLT, 1 | FF_FULLBRIGHT,     2, NULL/*A_AccTeleGlitter*/,       HS_TELEGLITTER1_3   }, // HS_TELEGLITTER1_2
    { HSPR_TGLT, 2 | FF_FULLBRIGHT,     2, NULL,                           HS_TELEGLITTER1_4   }, // HS_TELEGLITTER1_3
    { HSPR_TGLT, 3 | FF_FULLBRIGHT,     2, NULL/*A_AccTeleGlitter*/,       HS_TELEGLITTER1_5   }, // HS_TELEGLITTER1_4
    { HSPR_TGLT, 4 | FF_FULLBRIGHT,     2, NULL,                           HS_TELEGLITTER1_1   }, // HS_TELEGLITTER1_5
    { HSPR_TGLT, 5 | FF_FULLBRIGHT,     2, NULL,                           HS_TELEGLITTER2_2   }, // HS_TELEGLITTER2_1
    { HSPR_TGLT, 6 | FF_FULLBRIGHT,     2, NULL/*A_AccTeleGlitter*/,       HS_TELEGLITTER2_3   }, // HS_TELEGLITTER2_2
    { HSPR_TGLT, 7 | FF_FULLBRIGHT,     2, NULL,                           HS_TELEGLITTER2_4   }, // HS_TELEGLITTER2_3
    { HSPR_TGLT, 8 | FF_FULLBRIGHT,     2, NULL/*A_AccTeleGlitter*/,       HS_TELEGLITTER2_5   }, // HS_TELEGLITTER2_4
    { HSPR_TGLT, 9 | FF_FULLBRIGHT,     2, NULL,                           HS_TELEGLITTER2_1   }, // HS_TELEGLITTER2_5
    { HSPR_TELE, 0 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG2            }, // HS_TFOG1
    { HSPR_TELE, 1 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG3            }, // HS_TFOG2
    { HSPR_TELE, 2 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG4            }, // HS_TFOG3
    { HSPR_TELE, 3 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG5            }, // HS_TFOG4
    { HSPR_TELE, 4 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG6            }, // HS_TFOG5
    { HSPR_TELE, 5 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG7            }, // HS_TFOG6
    { HSPR_TELE, 6 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG8            }, // HS_TFOG7
    { HSPR_TELE, 7 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG9            }, // HS_TFOG8
    { HSPR_TELE, 6 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG10           }, // HS_TFOG9
    { HSPR_TELE, 5 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG11           }, // HS_TFOG10
    { HSPR_TELE, 4 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG12           }, // HS_TFOG11
    { HSPR_TELE, 3 | FF_FULLBRIGHT,     6, NULL,                           HS_TFOG13           }, // HS_TFOG12
    { HSPR_TELE, 2 | FF_FULLBRIGHT,     6, NULL,                           HS_NULL             }, // HS_TFOG13
    { HSPR_STFF, 0,                     0, A_Light0,                       HS_NULL             }, // HS_LIGHTDONE
    { HSPR_STFF, 0,                     1, A_WeaponReady,                  HS_STAFFREADY       }, // HS_STAFFREADY
    { HSPR_STFF, 0,                     1, A_Lower,                        HS_STAFFDOWN        }, // HS_STAFFDOWN
    { HSPR_STFF, 0,                     1, A_Raise,                        HS_STAFFUP          }, // HS_STAFFUP
    { HSPR_STFF, 3,                     4, A_WeaponReady,                  HS_STAFFREADY2_2    }, // HS_STAFFREADY2_1
    { HSPR_STFF, 4,                     4, A_WeaponReady,                  HS_STAFFREADY2_3    }, // HS_STAFFREADY2_2
    { HSPR_STFF, 5,                     4, A_WeaponReady,                  HS_STAFFREADY2_1    }, // HS_STAFFREADY2_3
    { HSPR_STFF, 3,                     1, A_Lower,                        HS_STAFFDOWN2       }, // HS_STAFFDOWN2
    { HSPR_STFF, 3,                     1, A_Raise,                        HS_STAFFUP2         }, // HS_STAFFUP2
    { HSPR_STFF, 1,                     6, NULL,                           HS_STAFFATK1_2      }, // HS_STAFFATK1_1
    { HSPR_STFF, 2,                     8, NULL/*A_StaffAttackPL1*/,       HS_STAFFATK1_3      }, // HS_STAFFATK1_2
    { HSPR_STFF, 1,                     8, A_ReFire,                       HS_STAFFREADY       }, // HS_STAFFATK1_3
    { HSPR_STFF, 6,                     6, NULL,                           HS_STAFFATK2_2      }, // HS_STAFFATK2_1
    { HSPR_STFF, 7,                     8, NULL/*A_StaffAttackPL2*/,       HS_STAFFATK2_3      }, // HS_STAFFATK2_2
    { HSPR_STFF, 6,                     8, A_ReFire,                       HS_STAFFREADY2_1    }, // HS_STAFFATK2_3
    { HSPR_PUF3, 0 | FF_FULLBRIGHT,     4, NULL,                           HS_STAFFPUFF2       }, // HS_STAFFPUFF1
    { HSPR_PUF3, 1,                     4, NULL,                           HS_STAFFPUFF3       }, // HS_STAFFPUFF2
    { HSPR_PUF3, 2,                     4, NULL,                           HS_STAFFPUFF4       }, // HS_STAFFPUFF3
    { HSPR_PUF3, 3,                     4, NULL,                           HS_NULL             }, // HS_STAFFPUFF4
    { HSPR_PUF4, 0 | FF_FULLBRIGHT,     4, NULL,                           HS_STAFFPUFF2_2     }, // HS_STAFFPUFF2_1
    { HSPR_PUF4, 1 | FF_FULLBRIGHT,     4, NULL,                           HS_STAFFPUFF2_3     }, // HS_STAFFPUFF2_2
    { HSPR_PUF4, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_STAFFPUFF2_4     }, // HS_STAFFPUFF2_3
    { HSPR_PUF4, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_STAFFPUFF2_5     }, // HS_STAFFPUFF2_4
    { HSPR_PUF4, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_STAFFPUFF2_6     }, // HS_STAFFPUFF2_5
    { HSPR_PUF4, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_STAFFPUFF2_6
    { HSPR_BEAK, 0,                     1, NULL/*A_BeakReady*/,            HS_BEAKREADY        }, // HS_BEAKREADY
    { HSPR_BEAK, 0,                     1, A_Lower,                        HS_BEAKDOWN         }, // HS_BEAKDOWN
    { HSPR_BEAK, 0,                     1, NULL/*A_BeakRaise*/,            HS_BEAKUP           }, // HS_BEAKUP
    { HSPR_BEAK, 0,                    18, NULL/*A_BeakAttackPL1*/,        HS_BEAKREADY        }, // HS_BEAKATK1_1
    { HSPR_BEAK, 0,                    12, NULL/*A_BeakAttackPL2*/,        HS_BEAKREADY        }, // HS_BEAKATK2_1
    { HSPR_WGNT, 0,                    -1, NULL,                           HS_NULL             }, // HS_WGNT
    { HSPR_GAUN, 0,                     1, A_WeaponReady,                  HS_GAUNTLETREADY    }, // HS_GAUNTLETREADY
    { HSPR_GAUN, 0,                     1, A_Lower,                        HS_GAUNTLETDOWN     }, // HS_GAUNTLETDOWN
    { HSPR_GAUN, 0,                     1, A_Raise,                        HS_GAUNTLETUP       }, // HS_GAUNTLETUP
    { HSPR_GAUN, 6,                     4, A_WeaponReady,                  HS_GAUNTLETREADY2_2 }, // HS_GAUNTLETREADY2_1
    { HSPR_GAUN, 7,                     4, A_WeaponReady,                  HS_GAUNTLETREADY2_3 }, // HS_GAUNTLETREADY2_2
    { HSPR_GAUN, 8,                     4, A_WeaponReady,                  HS_GAUNTLETREADY2_1 }, // HS_GAUNTLETREADY2_3
    { HSPR_GAUN, 6,                     1, A_Lower,                        HS_GAUNTLETDOWN2    }, // HS_GAUNTLETDOWN2
    { HSPR_GAUN, 6,                     1, A_Raise,                        HS_GAUNTLETUP2      }, // HS_GAUNTLETUP2
    { HSPR_GAUN, 1,                     4, NULL,                           HS_GAUNTLETATK1_2   }, // HS_GAUNTLETATK1_1
    { HSPR_GAUN, 2,                     4, NULL,                           HS_GAUNTLETATK1_3   }, // HS_GAUNTLETATK1_2
    { HSPR_GAUN, 3 | FF_FULLBRIGHT,     4, NULL/*A_GauntletAttack*/,       HS_GAUNTLETATK1_4   }, // HS_GAUNTLETATK1_3
    { HSPR_GAUN, 4 | FF_FULLBRIGHT,     4, NULL/*A_GauntletAttack*/,       HS_GAUNTLETATK1_5   }, // HS_GAUNTLETATK1_4
    { HSPR_GAUN, 5 | FF_FULLBRIGHT,     4, NULL/*A_GauntletAttack*/,       HS_GAUNTLETATK1_6   }, // HS_GAUNTLETATK1_5
    { HSPR_GAUN, 2,                     4, A_ReFire,                       HS_GAUNTLETATK1_7   }, // HS_GAUNTLETATK1_6
    { HSPR_GAUN, 1,                     4, A_Light0,                       HS_GAUNTLETREADY    }, // HS_GAUNTLETATK1_7
    { HSPR_GAUN, 9,                     4, NULL,                           HS_GAUNTLETATK2_2   }, // HS_GAUNTLETATK2_1
    { HSPR_GAUN, 10,                    4, NULL,                           HS_GAUNTLETATK2_3   }, // HS_GAUNTLETATK2_2
    { HSPR_GAUN, 11 | FF_FULLBRIGHT,    4, NULL/*A_GauntletAttack*/,       HS_GAUNTLETATK2_4   }, // HS_GAUNTLETATK2_3
    { HSPR_GAUN, 12 | FF_FULLBRIGHT,    4, NULL/*A_GauntletAttack*/,       HS_GAUNTLETATK2_5   }, // HS_GAUNTLETATK2_4
    { HSPR_GAUN, 13 | FF_FULLBRIGHT,    4, NULL/*A_GauntletAttack*/,       HS_GAUNTLETATK2_6   }, // HS_GAUNTLETATK2_5
    { HSPR_GAUN, 10,                    4, A_ReFire,                       HS_GAUNTLETATK2_7   }, // HS_GAUNTLETATK2_6
    { HSPR_GAUN, 9,                     4, A_Light0,                       HS_GAUNTLETREADY2_1 }, // HS_GAUNTLETATK2_7
    { HSPR_PUF1, 0 | FF_FULLBRIGHT,     4, NULL,                           HS_GAUNTLETPUFF1_2  }, // HS_GAUNTLETPUFF1_1
    { HSPR_PUF1, 1 | FF_FULLBRIGHT,     4, NULL,                           HS_GAUNTLETPUFF1_3  }, // HS_GAUNTLETPUFF1_2
    { HSPR_PUF1, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_GAUNTLETPUFF1_4  }, // HS_GAUNTLETPUFF1_3
    { HSPR_PUF1, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_GAUNTLETPUFF1_4
    { HSPR_PUF1, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_GAUNTLETPUFF2_2  }, // HS_GAUNTLETPUFF2_1
    { HSPR_PUF1, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_GAUNTLETPUFF2_3  }, // HS_GAUNTLETPUFF2_2
    { HSPR_PUF1, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_GAUNTLETPUFF2_4  }, // HS_GAUNTLETPUFF2_3
    { HSPR_PUF1, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_GAUNTLETPUFF2_4
    { HSPR_WBLS, 0,                    -1, NULL,                           HS_NULL             }, // HS_BLSR
    { HSPR_BLSR, 0,                     1, A_WeaponReady,                  HS_BLASTERREADY     }, // HS_BLASTERREADY
    { HSPR_BLSR, 0,                     1, A_Lower,                        HS_BLASTERDOWN      }, // HS_BLASTERDOWN
    { HSPR_BLSR, 0,                     1, A_Raise,                        HS_BLASTERUP        }, // HS_BLASTERUP
    { HSPR_BLSR, 1,                     3, NULL,                           HS_BLASTERATK1_2    }, // HS_BLASTERATK1_1
    { HSPR_BLSR, 2,                     3, NULL,                           HS_BLASTERATK1_3    }, // HS_BLASTERATK1_2
    { HSPR_BLSR, 3,                     2, NULL/*A_FireBlasterPL1*/,       HS_BLASTERATK1_4    }, // HS_BLASTERATK1_3
    { HSPR_BLSR, 2,                     2, NULL,                           HS_BLASTERATK1_5    }, // HS_BLASTERATK1_4
    { HSPR_BLSR, 1,                     2, NULL,                           HS_BLASTERATK1_6    }, // HS_BLASTERATK1_5
    { HSPR_BLSR, 0,                     0, A_ReFire,                       HS_BLASTERREADY     }, // HS_BLASTERATK1_6
    { HSPR_BLSR, 1,                     0, NULL,                           HS_BLASTERATK2_2    }, // HS_BLASTERATK2_1
    { HSPR_BLSR, 2,                     0, NULL,                           HS_BLASTERATK2_3    }, // HS_BLASTERATK2_2
    { HSPR_BLSR, 3,                     3, NULL/*A_FireBlasterPL2*/,       HS_BLASTERATK2_4    }, // HS_BLASTERATK2_3
    { HSPR_BLSR, 2,                     4, NULL,                           HS_BLASTERATK2_5    }, // HS_BLASTERATK2_4
    { HSPR_BLSR, 1,                     4, NULL,                           HS_BLASTERATK2_6    }, // HS_BLASTERATK2_5
    { HSPR_BLSR, 0,                     0, A_ReFire,                       HS_BLASTERREADY     }, // HS_BLASTERATK2_6
    { HSPR_ACLO, 4,                   200, NULL,                           HS_BLASTERFX1_1     }, // HS_BLASTERFX1_1
    { HSPR_FX18, 0 | FF_FULLBRIGHT,     3, NULL/*A_SpawnRippers*/,         HS_BLASTERFXI1_2    }, // HS_BLASTERFXI1_1
    { HSPR_FX18, 1 | FF_FULLBRIGHT,     3, NULL,                           HS_BLASTERFXI1_3    }, // HS_BLASTERFXI1_2
    { HSPR_FX18, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERFXI1_4    }, // HS_BLASTERFXI1_3
    { HSPR_FX18, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERFXI1_5    }, // HS_BLASTERFXI1_4
    { HSPR_FX18, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERFXI1_6    }, // HS_BLASTERFXI1_5
    { HSPR_FX18, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERFXI1_7    }, // HS_BLASTERFXI1_6
    { HSPR_FX18, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_BLASTERFXI1_7
    { HSPR_FX18, 7,                     4, NULL,                           HS_BLASTERSMOKE2    }, // HS_BLASTERSMOKE1
    { HSPR_FX18, 8,                     4, NULL,                           HS_BLASTERSMOKE3    }, // HS_BLASTERSMOKE2
    { HSPR_FX18, 9,                     4, NULL,                           HS_BLASTERSMOKE4    }, // HS_BLASTERSMOKE3
    { HSPR_FX18, 10,                    4, NULL,                           HS_BLASTERSMOKE5    }, // HS_BLASTERSMOKE4
    { HSPR_FX18, 11,                    4, NULL,                           HS_NULL             }, // HS_BLASTERSMOKE5
    { HSPR_FX18, 12,                    4, NULL,                           HS_RIPPER2          }, // HS_RIPPER1
    { HSPR_FX18, 13,                    5, NULL,                           HS_RIPPER1          }, // HS_RIPPER2
    { HSPR_FX18, 14 | FF_FULLBRIGHT,    4, NULL,                           HS_RIPPERX2         }, // HS_RIPPERX1
    { HSPR_FX18, 15 | FF_FULLBRIGHT,    4, NULL,                           HS_RIPPERX3         }, // HS_RIPPERX2
    { HSPR_FX18, 16 | FF_FULLBRIGHT,    4, NULL,                           HS_RIPPERX4         }, // HS_RIPPERX3
    { HSPR_FX18, 17 | FF_FULLBRIGHT,    4, NULL,                           HS_RIPPERX5         }, // HS_RIPPERX4
    { HSPR_FX18, 18 | FF_FULLBRIGHT,    4, NULL,                           HS_NULL             }, // HS_RIPPERX5
    { HSPR_FX17, 0 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERPUFF1_2   }, // HS_BLASTERPUFF1_1
    { HSPR_FX17, 1 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERPUFF1_3   }, // HS_BLASTERPUFF1_2
    { HSPR_FX17, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERPUFF1_4   }, // HS_BLASTERPUFF1_3
    { HSPR_FX17, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERPUFF1_5   }, // HS_BLASTERPUFF1_4
    { HSPR_FX17, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_BLASTERPUFF1_5
    { HSPR_FX17, 5 | FF_FULLBRIGHT,     3, NULL,                           HS_BLASTERPUFF2_2   }, // HS_BLASTERPUFF2_1
    { HSPR_FX17, 6 | FF_FULLBRIGHT,     3, NULL,                           HS_BLASTERPUFF2_3   }, // HS_BLASTERPUFF2_2
    { HSPR_FX17, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERPUFF2_4   }, // HS_BLASTERPUFF2_3
    { HSPR_FX17, 8 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERPUFF2_5   }, // HS_BLASTERPUFF2_4
    { HSPR_FX17, 9 | FF_FULLBRIGHT,     4, NULL,                           HS_BLASTERPUFF2_6   }, // HS_BLASTERPUFF2_5
    { HSPR_FX17, 10 | FF_FULLBRIGHT,    4, NULL,                           HS_BLASTERPUFF2_7   }, // HS_BLASTERPUFF2_6
    { HSPR_FX17, 11 | FF_FULLBRIGHT,    4, NULL,                           HS_NULL             }, // HS_BLASTERPUFF2_7
    { HSPR_WMCE, 0,                    -1, NULL,                           HS_NULL             }, // HS_WMCE
    { HSPR_MACE, 0,                     1, A_WeaponReady,                  HS_MACEREADY        }, // HS_MACEREADY
    { HSPR_MACE, 0,                     1, A_Lower,                        HS_MACEDOWN         }, // HS_MACEDOWN
    { HSPR_MACE, 0,                     1, A_Raise,                        HS_MACEUP           }, // HS_MACEUP
    { HSPR_MACE, 1,                     4, NULL,                           HS_MACEATK1_2       }, // HS_MACEATK1_1
    { HSPR_MACE, 2,                     3, NULL/*A_FireMacePL1*/,          HS_MACEATK1_3       }, // HS_MACEATK1_2
    { HSPR_MACE, 3,                     3, NULL/*A_FireMacePL1*/,          HS_MACEATK1_4       }, // HS_MACEATK1_3
    { HSPR_MACE, 4,                     3, NULL/*A_FireMacePL1*/,          HS_MACEATK1_5       }, // HS_MACEATK1_4
    { HSPR_MACE, 5,                     3, NULL/*A_FireMacePL1*/,          HS_MACEATK1_6       }, // HS_MACEATK1_5
    { HSPR_MACE, 2,                     4, A_ReFire,                       HS_MACEATK1_7       }, // HS_MACEATK1_6
    { HSPR_MACE, 3,                     4, NULL,                           HS_MACEATK1_8       }, // HS_MACEATK1_7
    { HSPR_MACE, 4,                     4, NULL,                           HS_MACEATK1_9       }, // HS_MACEATK1_8
    { HSPR_MACE, 5,                     4, NULL,                           HS_MACEATK1_10      }, // HS_MACEATK1_9
    { HSPR_MACE, 1,                     4, NULL,                           HS_MACEREADY        }, // HS_MACEATK1_10
    { HSPR_MACE, 1,                     4, NULL,                           HS_MACEATK2_2       }, // HS_MACEATK2_1
    { HSPR_MACE, 3,                     4, NULL/*A_FireMacePL2*/,          HS_MACEATK2_3       }, // HS_MACEATK2_2
    { HSPR_MACE, 1,                     4, NULL,                           HS_MACEATK2_4       }, // HS_MACEATK2_3
    { HSPR_MACE, 0,                     8, A_ReFire,                       HS_MACEREADY        }, // HS_MACEATK2_4
    { HSPR_FX02, 0,                     4, NULL/*A_MacePL1Check*/,         HS_MACEFX1_2        }, // HS_MACEFX1_1
    { HSPR_FX02, 1,                     4, NULL/*A_MacePL1Check*/,         HS_MACEFX1_1        }, // HS_MACEFX1_2
    { HSPR_FX02, 5 | FF_FULLBRIGHT,     4, NULL/*A_MaceBallImpact*/,       HS_MACEFXI1_2       }, // HS_MACEFXI1_1
    { HSPR_FX02, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_MACEFXI1_3       }, // HS_MACEFXI1_2
    { HSPR_FX02, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_MACEFXI1_4       }, // HS_MACEFXI1_3
    { HSPR_FX02, 8 | FF_FULLBRIGHT,     4, NULL,                           HS_MACEFXI1_5       }, // HS_MACEFXI1_4
    { HSPR_FX02, 9 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_MACEFXI1_5
    { HSPR_FX02, 2,                     4, NULL,                           HS_MACEFX2_2        }, // HS_MACEFX2_1
    { HSPR_FX02, 3,                     4, NULL,                           HS_MACEFX2_1        }, // HS_MACEFX2_2
    { HSPR_FX02, 5 | FF_FULLBRIGHT,     4, NULL/*A_MaceBallImpact2*/,      HS_MACEFXI1_2       }, // HS_MACEFXI2_1
    { HSPR_FX02, 0,                     4, NULL,                           HS_MACEFX3_2        }, // HS_MACEFX3_1
    { HSPR_FX02, 1,                     4, NULL,                           HS_MACEFX3_1        }, // HS_MACEFX3_2
    { HSPR_FX02, 4,                    99, NULL,                           HS_MACEFX4_1        }, // HS_MACEFX4_1
    { HSPR_FX02, 2 | FF_FULLBRIGHT,     4, NULL/*A_DeathBallImpact*/,      HS_MACEFXI1_2       }, // HS_MACEFXI4_1
    { HSPR_WSKL, 0,                    -1, NULL,                           HS_NULL             }, // HS_WSKL
    { HSPR_HROD, 0,                     1, A_WeaponReady,                  HS_HORNRODREADY     }, // HS_HORNRODREADY
    { HSPR_HROD, 0,                     1, A_Lower,                        HS_HORNRODDOWN      }, // HS_HORNRODDOWN
    { HSPR_HROD, 0,                     1, A_Raise,                        HS_HORNRODUP        }, // HS_HORNRODUP
    { HSPR_HROD, 0,                     4, NULL/*A_FireSkullRodPL1*/,      HS_HORNRODATK1_2    }, // HS_HORNRODATK1_1
    { HSPR_HROD, 1,                     4, NULL/*A_FireSkullRodPL1*/,      HS_HORNRODATK1_3    }, // HS_HORNRODATK1_2
    { HSPR_HROD, 1,                     0, A_ReFire,                       HS_HORNRODREADY     }, // HS_HORNRODATK1_3
    { HSPR_HROD, 2,                     2, NULL,                           HS_HORNRODATK2_2    }, // HS_HORNRODATK2_1
    { HSPR_HROD, 3,                     3, NULL,                           HS_HORNRODATK2_3    }, // HS_HORNRODATK2_2
    { HSPR_HROD, 4,                     2, NULL,                           HS_HORNRODATK2_4    }, // HS_HORNRODATK2_3
    { HSPR_HROD, 5,                     3, NULL,                           HS_HORNRODATK2_5    }, // HS_HORNRODATK2_4
    { HSPR_HROD, 6,                     4, NULL/*A_FireSkullRodPL2*/,      HS_HORNRODATK2_6    }, // HS_HORNRODATK2_5
    { HSPR_HROD, 5,                     2, NULL,                           HS_HORNRODATK2_7    }, // HS_HORNRODATK2_6
    { HSPR_HROD, 4,                     3, NULL,                           HS_HORNRODATK2_8    }, // HS_HORNRODATK2_7
    { HSPR_HROD, 3,                     2, NULL,                           HS_HORNRODATK2_9    }, // HS_HORNRODATK2_8
    { HSPR_HROD, 2,                     2, A_ReFire,                       HS_HORNRODREADY     }, // HS_HORNRODATK2_9
    { HSPR_FX00, 0 | FF_FULLBRIGHT,     6, NULL,                           HS_HRODFX1_2        }, // HS_HRODFX1_1
    { HSPR_FX00, 1 | FF_FULLBRIGHT,     6, NULL,                           HS_HRODFX1_1        }, // HS_HRODFX1_2
    { HSPR_FX00, 7 | FF_FULLBRIGHT,     5, NULL,                           HS_HRODFXI1_2       }, // HS_HRODFXI1_1
    { HSPR_FX00, 8 | FF_FULLBRIGHT,     5, NULL,                           HS_HRODFXI1_3       }, // HS_HRODFXI1_2
    { HSPR_FX00, 9 | FF_FULLBRIGHT,     4, NULL,                           HS_HRODFXI1_4       }, // HS_HRODFXI1_3
    { HSPR_FX00, 10 | FF_FULLBRIGHT,    4, NULL,                           HS_HRODFXI1_5       }, // HS_HRODFXI1_4
    { HSPR_FX00, 11 | FF_FULLBRIGHT,    3, NULL,                           HS_HRODFXI1_6       }, // HS_HRODFXI1_5
    { HSPR_FX00, 12 | FF_FULLBRIGHT,    3, NULL,                           HS_NULL             }, // HS_HRODFXI1_6
    { HSPR_FX00, 2 | FF_FULLBRIGHT,     3, NULL,                           HS_HRODFX2_2        }, // HS_HRODFX2_1
    { HSPR_FX00, 3 | FF_FULLBRIGHT,     3, NULL/*A_SkullRodPL2Seek*/,      HS_HRODFX2_3        }, // HS_HRODFX2_2
    { HSPR_FX00, 4 | FF_FULLBRIGHT,     3, NULL,                           HS_HRODFX2_4        }, // HS_HRODFX2_3
    { HSPR_FX00, 5 | FF_FULLBRIGHT,     3, NULL/*A_SkullRodPL2Seek*/,      HS_HRODFX2_1        }, // HS_HRODFX2_4
    { HSPR_FX00, 7 | FF_FULLBRIGHT,     5, NULL/*A_AddPlayerRain*/,        HS_HRODFXI2_2       }, // HS_HRODFXI2_1
    { HSPR_FX00, 8 | FF_FULLBRIGHT,     5, NULL,                           HS_HRODFXI2_3       }, // HS_HRODFXI2_2
    { HSPR_FX00, 9 | FF_FULLBRIGHT,     4, NULL,                           HS_HRODFXI2_4       }, // HS_HRODFXI2_3
    { HSPR_FX00, 10 | FF_FULLBRIGHT,    3, NULL,                           HS_HRODFXI2_5       }, // HS_HRODFXI2_4
    { HSPR_FX00, 11 | FF_FULLBRIGHT,    3, NULL,                           HS_HRODFXI2_6       }, // HS_HRODFXI2_5
    { HSPR_FX00, 12 | FF_FULLBRIGHT,    3, NULL,                           HS_HRODFXI2_7       }, // HS_HRODFXI2_6
    { HSPR_FX00, 6,                     1, NULL/*A_HideInCeiling*/,        HS_HRODFXI2_8       }, // HS_HRODFXI2_7
    { HSPR_FX00, 6,                     1, NULL/*A_SkullRodStorm*/,        HS_HRODFXI2_8       }, // HS_HRODFXI2_8
    { HSPR_FX20, 0 | FF_FULLBRIGHT,    -1, NULL,                           HS_NULL             }, // HS_RAINPLR1_1
    { HSPR_FX21, 0 | FF_FULLBRIGHT,    -1, NULL,                           HS_NULL             }, // HS_RAINPLR2_1
    { HSPR_FX22, 0 | FF_FULLBRIGHT,    -1, NULL,                           HS_NULL             }, // HS_RAINPLR3_1
    { HSPR_FX23, 0 | FF_FULLBRIGHT,    -1, NULL,                           HS_NULL             }, // HS_RAINPLR4_1
    { HSPR_FX20, 1 | FF_FULLBRIGHT,     4, NULL/*A_RainImpact*/,           HS_RAINPLR1X_2      }, // HS_RAINPLR1X_1
    { HSPR_FX20, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR1X_3      }, // HS_RAINPLR1X_2
    { HSPR_FX20, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR1X_4      }, // HS_RAINPLR1X_3
    { HSPR_FX20, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR1X_5      }, // HS_RAINPLR1X_4
    { HSPR_FX20, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_RAINPLR1X_5
    { HSPR_FX21, 1 | FF_FULLBRIGHT,     4, NULL/*A_RainImpact*/,           HS_RAINPLR2X_2      }, // HS_RAINPLR2X_1
    { HSPR_FX21, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR2X_3      }, // HS_RAINPLR2X_2
    { HSPR_FX21, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR2X_4      }, // HS_RAINPLR2X_3
    { HSPR_FX21, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR2X_5      }, // HS_RAINPLR2X_4
    { HSPR_FX21, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_RAINPLR2X_5
    { HSPR_FX22, 1 | FF_FULLBRIGHT,     4, NULL/*A_RainImpact*/,           HS_RAINPLR3X_2      }, // HS_RAINPLR3X_1
    { HSPR_FX22, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR3X_3      }, // HS_RAINPLR3X_2
    { HSPR_FX22, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR3X_4      }, // HS_RAINPLR3X_3
    { HSPR_FX22, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR3X_5      }, // HS_RAINPLR3X_4
    { HSPR_FX22, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_RAINPLR3X_5
    { HSPR_FX23, 1 | FF_FULLBRIGHT,     4, NULL/*A_RainImpact*/,           HS_RAINPLR4X_2      }, // HS_RAINPLR4X_1
    { HSPR_FX23, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR4X_3      }, // HS_RAINPLR4X_2
    { HSPR_FX23, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR4X_4      }, // HS_RAINPLR4X_3
    { HSPR_FX23, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINPLR4X_5      }, // HS_RAINPLR4X_4
    { HSPR_FX23, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_RAINPLR4X_5
    { HSPR_FX20, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINAIRXPLR1_2   }, // HS_RAINAIRXPLR1_1
    { HSPR_FX21, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINAIRXPLR2_2   }, // HS_RAINAIRXPLR2_1
    { HSPR_FX22, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINAIRXPLR3_2   }, // HS_RAINAIRXPLR3_1
    { HSPR_FX23, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINAIRXPLR4_2   }, // HS_RAINAIRXPLR4_1
    { HSPR_FX20, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINAIRXPLR1_3   }, // HS_RAINAIRXPLR1_2
    { HSPR_FX21, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINAIRXPLR2_3   }, // HS_RAINAIRXPLR2_2
    { HSPR_FX22, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINAIRXPLR3_3   }, // HS_RAINAIRXPLR3_2
    { HSPR_FX23, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_RAINAIRXPLR4_3   }, // HS_RAINAIRXPLR4_2
    { HSPR_FX20, 8 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_RAINAIRXPLR1_3
    { HSPR_FX21, 8 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_RAINAIRXPLR2_3
    { HSPR_FX22, 8 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_RAINAIRXPLR3_3
    { HSPR_FX23, 8 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_RAINAIRXPLR4_3
    { HSPR_GWND, 0,                     1, A_WeaponReady,                  HS_GOLDWANDREADY    }, // HS_GOLDWANDREADY
    { HSPR_GWND, 0,                     1, A_Lower,                        HS_GOLDWANDDOWN     }, // HS_GOLDWANDDOWN
    { HSPR_GWND, 0,                     1, A_Raise,                        HS_GOLDWANDUP       }, // HS_GOLDWANDUP
    { HSPR_GWND, 1,                     3, NULL,                           HS_GOLDWANDATK1_2   }, // HS_GOLDWANDATK1_1
    { HSPR_GWND, 2,                     5, NULL/*A_FireGoldWandPL1*/,      HS_GOLDWANDATK1_3   }, // HS_GOLDWANDATK1_2
    { HSPR_GWND, 3,                     3, NULL,                           HS_GOLDWANDATK1_4   }, // HS_GOLDWANDATK1_3
    { HSPR_GWND, 3,                     0, A_ReFire,                       HS_GOLDWANDREADY    }, // HS_GOLDWANDATK1_4
    { HSPR_GWND, 1,                     3, NULL,                           HS_GOLDWANDATK2_2   }, // HS_GOLDWANDATK2_1
    { HSPR_GWND, 2,                     4, NULL/*A_FireGoldWandPL2*/,      HS_GOLDWANDATK2_3   }, // HS_GOLDWANDATK2_2
    { HSPR_GWND, 3,                     3, NULL,                           HS_GOLDWANDATK2_4   }, // HS_GOLDWANDATK2_3
    { HSPR_GWND, 3,                     0, A_ReFire,                       HS_GOLDWANDREADY    }, // HS_GOLDWANDATK2_4
    { HSPR_FX01, 0 | FF_FULLBRIGHT,     6, NULL,                           HS_GWANDFX1_2       }, // HS_GWANDFX1_1
    { HSPR_FX01, 1 | FF_FULLBRIGHT,     6, NULL,                           HS_GWANDFX1_1       }, // HS_GWANDFX1_2
    { HSPR_FX01, 4 | FF_FULLBRIGHT,     3, NULL,                           HS_GWANDFXI1_2      }, // HS_GWANDFXI1_1
    { HSPR_FX01, 5 | FF_FULLBRIGHT,     3, NULL,                           HS_GWANDFXI1_3      }, // HS_GWANDFXI1_2
    { HSPR_FX01, 6 | FF_FULLBRIGHT,     3, NULL,                           HS_GWANDFXI1_4      }, // HS_GWANDFXI1_3
    { HSPR_FX01, 7 | FF_FULLBRIGHT,     3, NULL,                           HS_NULL             }, // HS_GWANDFXI1_4
    { HSPR_FX01, 2 | FF_FULLBRIGHT,     6, NULL,                           HS_GWANDFX2_2       }, // HS_GWANDFX2_1
    { HSPR_FX01, 3 | FF_FULLBRIGHT,     6, NULL,                           HS_GWANDFX2_1       }, // HS_GWANDFX2_2
    { HSPR_PUF2, 0 | FF_FULLBRIGHT,     3, NULL,                           HS_GWANDPUFF1_2     }, // HS_GWANDPUFF1_1
    { HSPR_PUF2, 1 | FF_FULLBRIGHT,     3, NULL,                           HS_GWANDPUFF1_3     }, // HS_GWANDPUFF1_2
    { HSPR_PUF2, 2 | FF_FULLBRIGHT,     3, NULL,                           HS_GWANDPUFF1_4     }, // HS_GWANDPUFF1_3
    { HSPR_PUF2, 3 | FF_FULLBRIGHT,     3, NULL,                           HS_GWANDPUFF1_5     }, // HS_GWANDPUFF1_4
    { HSPR_PUF2, 4 | FF_FULLBRIGHT,     3, NULL,                           HS_NULL             }, // HS_GWANDPUFF1_5
    { HSPR_WPHX, 0,                    -1, NULL,                           HS_NULL             }, // HS_WPHX
    { HSPR_PHNX, 0,                     1, A_WeaponReady,                  HS_PHOENIXREADY     }, // HS_PHOENIXREADY
    { HSPR_PHNX, 0,                     1, A_Lower,                        HS_PHOENIXDOWN      }, // HS_PHOENIXDOWN
    { HSPR_PHNX, 0,                     1, A_Raise,                        HS_PHOENIXUP        }, // HS_PHOENIXUP
    { HSPR_PHNX, 1,                     5, NULL,                           HS_PHOENIXATK1_2    }, // HS_PHOENIXATK1_1
    { HSPR_PHNX, 2,                     7, NULL/*A_FirePhoenixPL1*/,       HS_PHOENIXATK1_3    }, // HS_PHOENIXATK1_2
    { HSPR_PHNX, 3,                     4, NULL,                           HS_PHOENIXATK1_4    }, // HS_PHOENIXATK1_3
    { HSPR_PHNX, 1,                     4, NULL,                           HS_PHOENIXATK1_5    }, // HS_PHOENIXATK1_4
    { HSPR_PHNX, 1,                     0, A_ReFire,                       HS_PHOENIXREADY     }, // HS_PHOENIXATK1_5
    { HSPR_PHNX, 1,                     3, NULL/*A_InitPhoenixPL2*/,       HS_PHOENIXATK2_2    }, // HS_PHOENIXATK2_1
    { HSPR_PHNX, 2 | FF_FULLBRIGHT,     1, NULL/*A_FirePhoenixPL2*/,       HS_PHOENIXATK2_3    }, // HS_PHOENIXATK2_2
    { HSPR_PHNX, 1,                     4, A_ReFire,                       HS_PHOENIXATK2_4    }, // HS_PHOENIXATK2_3
    { HSPR_PHNX, 1,                     4, NULL/*A_ShutdownPhoenixPL2*/,   HS_PHOENIXREADY     }, // HS_PHOENIXATK2_4
    { HSPR_FX04, 0 | FF_FULLBRIGHT,     4, NULL/*A_PhoenixPuff*/,          HS_PHOENIXFX1_1     }, // HS_PHOENIXFX1_1
    { HSPR_FX08, 0 | FF_FULLBRIGHT,     6, A_Explode,                      HS_PHOENIXFXI1_2    }, // HS_PHOENIXFXI1_1
    { HSPR_FX08, 1 | FF_FULLBRIGHT,     5, NULL,                           HS_PHOENIXFXI1_3    }, // HS_PHOENIXFXI1_2
    { HSPR_FX08, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_PHOENIXFXI1_4    }, // HS_PHOENIXFXI1_3
    { HSPR_FX08, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_PHOENIXFXI1_5    }, // HS_PHOENIXFXI1_4
    { HSPR_FX08, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_PHOENIXFXI1_6    }, // HS_PHOENIXFXI1_5
    { HSPR_FX08, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_PHOENIXFXI1_7    }, // HS_PHOENIXFXI1_6
    { HSPR_FX08, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_PHOENIXFXI1_8    }, // HS_PHOENIXFXI1_7
    { HSPR_FX08, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_PHOENIXFXI1_8
    { HSPR_FX08, 8 | FF_FULLBRIGHT,     8, NULL,                           HS_PHOENIXFXIX_1    }, // HS_PHOENIXFXIX_1
    { HSPR_FX08, 9 | FF_FULLBRIGHT,     8, NULL/*A_RemovedPhoenixFunc*/,   HS_PHOENIXFXIX_2    }, // HS_PHOENIXFXIX_2
    { HSPR_FX08, 10 | FF_FULLBRIGHT,    8, NULL,                           HS_NULL             }, // HS_PHOENIXFXIX_3
    { HSPR_FX04, 1,                     4, NULL,                           HS_PHOENIXPUFF2     }, // HS_PHOENIXPUFF1
    { HSPR_FX04, 2,                     4, NULL,                           HS_PHOENIXPUFF3     }, // HS_PHOENIXPUFF2
    { HSPR_FX04, 3,                     4, NULL,                           HS_PHOENIXPUFF4     }, // HS_PHOENIXPUFF3
    { HSPR_FX04, 4,                     4, NULL,                           HS_PHOENIXPUFF5     }, // HS_PHOENIXPUFF4
    { HSPR_FX04, 5,                     4, NULL,                           HS_NULL             }, // HS_PHOENIXPUFF5
    { HSPR_FX09, 0 | FF_FULLBRIGHT,     2, NULL,                           HS_PHOENIXFX2_2     }, // HS_PHOENIXFX2_1
    { HSPR_FX09, 1 | FF_FULLBRIGHT,     2, NULL,                           HS_PHOENIXFX2_3     }, // HS_PHOENIXFX2_2
    { HSPR_FX09, 0 | FF_FULLBRIGHT,     2, NULL,                           HS_PHOENIXFX2_4     }, // HS_PHOENIXFX2_3
    { HSPR_FX09, 1 | FF_FULLBRIGHT,     2, NULL,                           HS_PHOENIXFX2_5     }, // HS_PHOENIXFX2_4
    { HSPR_FX09, 0 | FF_FULLBRIGHT,     2, NULL,                           HS_PHOENIXFX2_6     }, // HS_PHOENIXFX2_5
    { HSPR_FX09, 1 | FF_FULLBRIGHT,     2, NULL/*A_FlameEnd*/,             HS_PHOENIXFX2_7     }, // HS_PHOENIXFX2_6
    { HSPR_FX09, 2 | FF_FULLBRIGHT,     2, NULL,                           HS_PHOENIXFX2_8     }, // HS_PHOENIXFX2_7
    { HSPR_FX09, 3 | FF_FULLBRIGHT,     2, NULL,                           HS_PHOENIXFX2_9     }, // HS_PHOENIXFX2_8
    { HSPR_FX09, 4 | FF_FULLBRIGHT,     2, NULL,                           HS_PHOENIXFX2_10    }, // HS_PHOENIXFX2_9
    { HSPR_FX09, 5 | FF_FULLBRIGHT,     2, NULL,                           HS_NULL             }, // HS_PHOENIXFX2_10
    { HSPR_FX09, 6 | FF_FULLBRIGHT,     3, NULL,                           HS_PHOENIXFXI2_2    }, // HS_PHOENIXFXI2_1
    { HSPR_FX09, 7 | FF_FULLBRIGHT,     3, NULL/*A_FloatPuff*/,            HS_PHOENIXFXI2_3    }, // HS_PHOENIXFXI2_2
    { HSPR_FX09, 8 | FF_FULLBRIGHT,     4, NULL,                           HS_PHOENIXFXI2_4    }, // HS_PHOENIXFXI2_3
    { HSPR_FX09, 9 | FF_FULLBRIGHT,     5, NULL,                           HS_PHOENIXFXI2_5    }, // HS_PHOENIXFXI2_4
    { HSPR_FX09, 10 | FF_FULLBRIGHT,    5, NULL,                           HS_NULL             }, // HS_PHOENIXFXI2_5
    { HSPR_WBOW, 0,                    -1, NULL,                           HS_NULL             }, // HS_WBOW
    { HSPR_CRBW, 0,                     1, A_WeaponReady,                  HS_CRBOW2           }, // HS_CRBOW1
    { HSPR_CRBW, 0,                     1, A_WeaponReady,                  HS_CRBOW3           }, // HS_CRBOW2
    { HSPR_CRBW, 0,                     1, A_WeaponReady,                  HS_CRBOW4           }, // HS_CRBOW3
    { HSPR_CRBW, 0,                     1, A_WeaponReady,                  HS_CRBOW5           }, // HS_CRBOW4
    { HSPR_CRBW, 0,                     1, A_WeaponReady,                  HS_CRBOW6           }, // HS_CRBOW5
    { HSPR_CRBW, 0,                     1, A_WeaponReady,                  HS_CRBOW7           }, // HS_CRBOW6
    { HSPR_CRBW, 1,                     1, A_WeaponReady,                  HS_CRBOW8           }, // HS_CRBOW7
    { HSPR_CRBW, 1,                     1, A_WeaponReady,                  HS_CRBOW9           }, // HS_CRBOW8
    { HSPR_CRBW, 1,                     1, A_WeaponReady,                  HS_CRBOW10          }, // HS_CRBOW9
    { HSPR_CRBW, 1,                     1, A_WeaponReady,                  HS_CRBOW11          }, // HS_CRBOW10
    { HSPR_CRBW, 1,                     1, A_WeaponReady,                  HS_CRBOW12          }, // HS_CRBOW11
    { HSPR_CRBW, 1,                     1, A_WeaponReady,                  HS_CRBOW13          }, // HS_CRBOW12
    { HSPR_CRBW, 2,                     1, A_WeaponReady,                  HS_CRBOW14          }, // HS_CRBOW13
    { HSPR_CRBW, 2,                     1, A_WeaponReady,                  HS_CRBOW15          }, // HS_CRBOW14
    { HSPR_CRBW, 2,                     1, A_WeaponReady,                  HS_CRBOW16          }, // HS_CRBOW15
    { HSPR_CRBW, 2,                     1, A_WeaponReady,                  HS_CRBOW17          }, // HS_CRBOW16
    { HSPR_CRBW, 2,                     1, A_WeaponReady,                  HS_CRBOW18          }, // HS_CRBOW17
    { HSPR_CRBW, 2,                     1, A_WeaponReady,                  HS_CRBOW1           }, // HS_CRBOW18
    { HSPR_CRBW, 0,                     1, A_Lower,                        HS_CRBOWDOWN        }, // HS_CRBOWDOWN
    { HSPR_CRBW, 0,                     1, A_Raise,                        HS_CRBOWUP          }, // HS_CRBOWUP
    { HSPR_CRBW, 3,                     6, NULL/*A_FireCrossbowPL1*/,      HS_CRBOWATK1_2      }, // HS_CRBOWATK1_1
    { HSPR_CRBW, 4,                     3, NULL,                           HS_CRBOWATK1_3      }, // HS_CRBOWATK1_2
    { HSPR_CRBW, 5,                     3, NULL,                           HS_CRBOWATK1_4      }, // HS_CRBOWATK1_3
    { HSPR_CRBW, 6,                     3, NULL,                           HS_CRBOWATK1_5      }, // HS_CRBOWATK1_4
    { HSPR_CRBW, 7,                     3, NULL,                           HS_CRBOWATK1_6      }, // HS_CRBOWATK1_5
    { HSPR_CRBW, 0,                     4, NULL,                           HS_CRBOWATK1_7      }, // HS_CRBOWATK1_6
    { HSPR_CRBW, 1,                     4, NULL,                           HS_CRBOWATK1_8      }, // HS_CRBOWATK1_7
    { HSPR_CRBW, 2,                     5, A_ReFire,                       HS_CRBOW1           }, // HS_CRBOWATK1_8
    { HSPR_CRBW, 3,                     5, NULL/*A_FireCrossbowPL2*/,      HS_CRBOWATK2_2      }, // HS_CRBOWATK2_1
    { HSPR_CRBW, 4,                     3, NULL,                           HS_CRBOWATK2_3      }, // HS_CRBOWATK2_2
    { HSPR_CRBW, 5,                     2, NULL,                           HS_CRBOWATK2_4      }, // HS_CRBOWATK2_3
    { HSPR_CRBW, 6,                     3, NULL,                           HS_CRBOWATK2_5      }, // HS_CRBOWATK2_4
    { HSPR_CRBW, 7,                     2, NULL,                           HS_CRBOWATK2_6      }, // HS_CRBOWATK2_5
    { HSPR_CRBW, 0,                     3, NULL,                           HS_CRBOWATK2_7      }, // HS_CRBOWATK2_6
    { HSPR_CRBW, 1,                     3, NULL,                           HS_CRBOWATK2_8      }, // HS_CRBOWATK2_7
    { HSPR_CRBW, 2,                     4, A_ReFire,                       HS_CRBOW1           }, // HS_CRBOWATK2_8
    { HSPR_FX03, 1 | FF_FULLBRIGHT,     1, NULL,                           HS_CRBOWFX1         }, // HS_CRBOWFX1
    { HSPR_FX03, 7 | FF_FULLBRIGHT,     8, NULL,                           HS_CRBOWFXI1_2      }, // HS_CRBOWFXI1_1
    { HSPR_FX03, 8 | FF_FULLBRIGHT,     8, NULL,                           HS_CRBOWFXI1_3      }, // HS_CRBOWFXI1_2
    { HSPR_FX03, 9 | FF_FULLBRIGHT,     8, NULL,                           HS_NULL             }, // HS_CRBOWFXI1_3
    { HSPR_FX03, 1 | FF_FULLBRIGHT,     1, NULL/*A_BoltSpark*/,            HS_CRBOWFX2         }, // HS_CRBOWFX2
    { HSPR_FX03, 0 | FF_FULLBRIGHT,     1, NULL,                           HS_CRBOWFX3         }, // HS_CRBOWFX3
    { HSPR_FX03, 2 | FF_FULLBRIGHT,     8, NULL,                           HS_CRBOWFXI3_2      }, // HS_CRBOWFXI3_1
    { HSPR_FX03, 3 | FF_FULLBRIGHT,     8, NULL,                           HS_CRBOWFXI3_3      }, // HS_CRBOWFXI3_2
    { HSPR_FX03, 4 | FF_FULLBRIGHT,     8, NULL,                           HS_NULL             }, // HS_CRBOWFXI3_3
    { HSPR_FX03, 5 | FF_FULLBRIGHT,     8, NULL,                           HS_CRBOWFX4_2       }, // HS_CRBOWFX4_1
    { HSPR_FX03, 6 | FF_FULLBRIGHT,     8, NULL,                           HS_NULL             }, // HS_CRBOWFX4_2
    { HSPR_BLOD, 2,                     8, NULL,                           HS_BLOOD2           }, // HS_BLOOD1
    { HSPR_BLOD, 1,                     8, NULL,                           HS_BLOOD3           }, // HS_BLOOD2
    { HSPR_BLOD, 0,                     8, NULL,                           HS_NULL             }, // HS_BLOOD3
    { HSPR_BLOD, 2,                     8, NULL,                           HS_BLOODSPLATTER2   }, // HS_BLOODSPLATTER1
    { HSPR_BLOD, 1,                     8, NULL,                           HS_BLOODSPLATTER3   }, // HS_BLOODSPLATTER2
    { HSPR_BLOD, 0,                     8, NULL,                           HS_NULL             }, // HS_BLOODSPLATTER3
    { HSPR_BLOD, 0,                     6, NULL,                           HS_NULL             }, // HS_BLOODSPLATTERX
    { HSPR_PLAY, 0,                    -1, NULL,                           HS_NULL             }, // HS_PLAY
    { HSPR_PLAY, 0,                     4, NULL,                           HS_PLAY_RUN2        }, // HS_PLAY_RUN1
    { HSPR_PLAY, 1,                     4, NULL,                           HS_PLAY_RUN3        }, // HS_PLAY_RUN2
    { HSPR_PLAY, 2,                     4, NULL,                           HS_PLAY_RUN4        }, // HS_PLAY_RUN3
    { HSPR_PLAY, 3,                     4, NULL,                           HS_PLAY_RUN1        }, // HS_PLAY_RUN4
    { HSPR_PLAY, 4,                    12, NULL,                           HS_PLAY             }, // HS_PLAY_ATK1
    { HSPR_PLAY, 5 | FF_FULLBRIGHT,     6, NULL,                           HS_PLAY_ATK1        }, // HS_PLAY_ATK2
    { HSPR_PLAY, 6,                     4, NULL,                           HS_PLAY_PAIN2       }, // HS_PLAY_PAIN
    { HSPR_PLAY, 6,                     4, A_Pain,                         HS_PLAY             }, // HS_PLAY_PAIN2
    { HSPR_PLAY, 7,                     6, NULL,                           HS_PLAY_DIE2        }, // HS_PLAY_DIE1
    { HSPR_PLAY, 8,                     6, A_Scream,                       HS_PLAY_DIE3        }, // HS_PLAY_DIE2
    { HSPR_PLAY, 9,                     6, NULL,                           HS_PLAY_DIE4        }, // HS_PLAY_DIE3
    { HSPR_PLAY, 10,                    6, NULL,                           HS_PLAY_DIE5        }, // HS_PLAY_DIE4
    { HSPR_PLAY, 11,                    6, NULL/*A_NoBlocking*/,           HS_PLAY_DIE6        }, // HS_PLAY_DIE5
    { HSPR_PLAY, 12,                    6, NULL,                           HS_PLAY_DIE7        }, // HS_PLAY_DIE6
    { HSPR_PLAY, 13,                    6, NULL,                           HS_PLAY_DIE8        }, // HS_PLAY_DIE7
    { HSPR_PLAY, 14,                    6, NULL,                           HS_PLAY_DIE9        }, // HS_PLAY_DIE8
    { HSPR_PLAY, 15,                   -1, NULL/*A_AddPlayerCorpse*/,      HS_NULL             }, // HS_PLAY_DIE9
    { HSPR_PLAY, 16,                    5, A_Scream,                       HS_PLAY_XDIE2       }, // HS_PLAY_XDIE1
    { HSPR_PLAY, 17,                    5, A_SkullPop,                     HS_PLAY_XDIE3       }, // HS_PLAY_XDIE2
    { HSPR_PLAY, 18,                    5, NULL/*A_NoBlocking*/,           HS_PLAY_XDIE4       }, // HS_PLAY_XDIE3
    { HSPR_PLAY, 19,                    5, NULL,                           HS_PLAY_XDIE5       }, // HS_PLAY_XDIE4
    { HSPR_PLAY, 20,                    5, NULL,                           HS_PLAY_XDIE6       }, // HS_PLAY_XDIE5
    { HSPR_PLAY, 21,                    5, NULL,                           HS_PLAY_XDIE7       }, // HS_PLAY_XDIE6
    { HSPR_PLAY, 22,                    5, NULL,                           HS_PLAY_XDIE8       }, // HS_PLAY_XDIE7
    { HSPR_PLAY, 23,                    5, NULL,                           HS_PLAY_XDIE9       }, // HS_PLAY_XDIE8
    { HSPR_PLAY, 24,                   -1, NULL/*A_AddPlayerCorpse*/,      HS_NULL             }, // HS_PLAY_XDIE9
    { HSPR_FDTH, 0 | FF_FULLBRIGHT,     5, NULL/*A_FlameSnd*/,             HS_PLAY_FDTH2       }, // HS_PLAY_FDTH1
    { HSPR_FDTH, 1 | FF_FULLBRIGHT,     4, NULL,                           HS_PLAY_FDTH3       }, // HS_PLAY_FDTH2
    { HSPR_FDTH, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_PLAY_FDTH4       }, // HS_PLAY_FDTH3
    { HSPR_FDTH, 3 | FF_FULLBRIGHT,     4, A_Scream,                       HS_PLAY_FDTH5       }, // HS_PLAY_FDTH4
    { HSPR_FDTH, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_PLAY_FDTH6       }, // HS_PLAY_FDTH5
    { HSPR_FDTH, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_PLAY_FDTH7       }, // HS_PLAY_FDTH6
    { HSPR_FDTH, 6 | FF_FULLBRIGHT,     5, NULL/*A_FlameSnd*/,             HS_PLAY_FDTH8       }, // HS_PLAY_FDTH7
    { HSPR_FDTH, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_PLAY_FDTH9       }, // HS_PLAY_FDTH8
    { HSPR_FDTH, 8 | FF_FULLBRIGHT,     5, NULL,                           HS_PLAY_FDTH10      }, // HS_PLAY_FDTH9
    { HSPR_FDTH, 9 | FF_FULLBRIGHT,     4, NULL,                           HS_PLAY_FDTH11      }, // HS_PLAY_FDTH10
    { HSPR_FDTH, 10 | FF_FULLBRIGHT,    5, NULL,                           HS_PLAY_FDTH12      }, // HS_PLAY_FDTH11
    { HSPR_FDTH, 11 | FF_FULLBRIGHT,    4, NULL,                           HS_PLAY_FDTH13      }, // HS_PLAY_FDTH12
    { HSPR_FDTH, 12 | FF_FULLBRIGHT,    5, NULL,                           HS_PLAY_FDTH14      }, // HS_PLAY_FDTH13
    { HSPR_FDTH, 13 | FF_FULLBRIGHT,    4, NULL,                           HS_PLAY_FDTH15      }, // HS_PLAY_FDTH14
    { HSPR_FDTH, 14 | FF_FULLBRIGHT,    5, NULL/*A_NoBlocking*/,           HS_PLAY_FDTH16      }, // HS_PLAY_FDTH15
    { HSPR_FDTH, 15 | FF_FULLBRIGHT,    4, NULL,                           HS_PLAY_FDTH17      }, // HS_PLAY_FDTH16
    { HSPR_FDTH, 16 | FF_FULLBRIGHT,    5, NULL,                           HS_PLAY_FDTH18      }, // HS_PLAY_FDTH17
    { HSPR_FDTH, 17 | FF_FULLBRIGHT,    4, NULL,                           HS_PLAY_FDTH19      }, // HS_PLAY_FDTH18
    { HSPR_ACLO, 4,                    35, NULL/*A_CheckBurnGone*/,        HS_PLAY_FDTH19      }, // HS_PLAY_FDTH19
    { HSPR_ACLO, 4,                     8, NULL,                           HS_NULL             }, // HS_PLAY_FDTH20
    { HSPR_BSKL, 0,                     5, NULL/*A_CheckSkullFloor*/,      HS_BLOODYSKULL2     }, // HS_BLOODYSKULL1
    { HSPR_BSKL, 1,                     5, NULL/*A_CheckSkullFloor*/,      HS_BLOODYSKULL3     }, // HS_BLOODYSKULL2
    { HSPR_BSKL, 2,                     5, NULL/*A_CheckSkullFloor*/,      HS_BLOODYSKULL4     }, // HS_BLOODYSKULL3
    { HSPR_BSKL, 3,                     5, NULL/*A_CheckSkullFloor*/,      HS_BLOODYSKULL5     }, // HS_BLOODYSKULL4
    { HSPR_BSKL, 4,                     5, NULL/*A_CheckSkullFloor*/,      HS_BLOODYSKULL1     }, // HS_BLOODYSKULL5
    { HSPR_BSKL, 5,                    16, NULL/*A_CheckSkullDone*/,       HS_BLOODYSKULLX1    }, // HS_BLOODYSKULLX1
    { HSPR_BSKL, 5,                  1050, NULL,                           HS_NULL             }, // HS_BLOODYSKULLX2
    { HSPR_CHKN, 0,                    -1, NULL,                           HS_NULL             }, // HS_CHICPLAY
    { HSPR_CHKN, 0,                     3, NULL,                           HS_CHICPLAY_RUN2    }, // HS_CHICPLAY_RUN1
    { HSPR_CHKN, 1,                     3, NULL,                           HS_CHICPLAY_RUN3    }, // HS_CHICPLAY_RUN2
    { HSPR_CHKN, 0,                     3, NULL,                           HS_CHICPLAY_RUN4    }, // HS_CHICPLAY_RUN3
    { HSPR_CHKN, 1,                     3, NULL,                           HS_CHICPLAY_RUN1    }, // HS_CHICPLAY_RUN4
    { HSPR_CHKN, 2,                    12, NULL,                           HS_CHICPLAY         }, // HS_CHICPLAY_ATK1
    { HSPR_CHKN, 3,                     4, NULL/*A_Feathers*/,             HS_CHICPLAY_PAIN2   }, // HS_CHICPLAY_PAIN
    { HSPR_CHKN, 2,                     4, A_Pain,                         HS_CHICPLAY         }, // HS_CHICPLAY_PAIN2
    { HSPR_CHKN, 0,                    10, NULL/*A_ChicLook*/,             HS_CHICKEN_LOOK2    }, // HS_CHICKEN_LOOK1
    { HSPR_CHKN, 1,                    10, NULL/*A_ChicLook*/,             HS_CHICKEN_LOOK1    }, // HS_CHICKEN_LOOK2
    { HSPR_CHKN, 0,                     3, NULL/*A_ChicChase*/,            HS_CHICKEN_WALK2    }, // HS_CHICKEN_WALK1
    { HSPR_CHKN, 1,                     3, NULL/*A_ChicChase*/,            HS_CHICKEN_WALK1    }, // HS_CHICKEN_WALK2
    { HSPR_CHKN, 3,                     5, NULL/*A_Feathers*/,             HS_CHICKEN_PAIN2    }, // HS_CHICKEN_PAIN1
    { HSPR_CHKN, 2,                     5, NULL/*A_ChicPain*/,             HS_CHICKEN_WALK1    }, // HS_CHICKEN_PAIN2
    { HSPR_CHKN, 0,                     8, A_FaceTarget,                   HS_CHICKEN_ATK2     }, // HS_CHICKEN_ATK1
    { HSPR_CHKN, 2,                    10, NULL/*A_ChicAttack*/,           HS_CHICKEN_WALK1    }, // HS_CHICKEN_ATK2
    { HSPR_CHKN, 4,                     6, A_Scream,                       HS_CHICKEN_DIE2     }, // HS_CHICKEN_DIE1
    { HSPR_CHKN, 5,                     6, NULL/*A_Feathers*/,             HS_CHICKEN_DIE3     }, // HS_CHICKEN_DIE2
    { HSPR_CHKN, 6,                     6, NULL,                           HS_CHICKEN_DIE4     }, // HS_CHICKEN_DIE3
    { HSPR_CHKN, 7,                     6, NULL/*A_NoBlocking*/,           HS_CHICKEN_DIE5     }, // HS_CHICKEN_DIE4
    { HSPR_CHKN, 8,                     6, NULL,                           HS_CHICKEN_DIE6     }, // HS_CHICKEN_DIE5
    { HSPR_CHKN, 9,                     6, NULL,                           HS_CHICKEN_DIE7     }, // HS_CHICKEN_DIE6
    { HSPR_CHKN, 10,                    6, NULL,                           HS_CHICKEN_DIE8     }, // HS_CHICKEN_DIE7
    { HSPR_CHKN, 11,                   -1, NULL,                           HS_NULL             }, // HS_CHICKEN_DIE8
    { HSPR_CHKN, 12,                    3, NULL,                           HS_FEATHER2         }, // HS_FEATHER1
    { HSPR_CHKN, 13,                    3, NULL,                           HS_FEATHER3         }, // HS_FEATHER2
    { HSPR_CHKN, 14,                    3, NULL,                           HS_FEATHER4         }, // HS_FEATHER3
    { HSPR_CHKN, 15,                    3, NULL,                           HS_FEATHER5         }, // HS_FEATHER4
    { HSPR_CHKN, 16,                    3, NULL,                           HS_FEATHER6         }, // HS_FEATHER5
    { HSPR_CHKN, 15,                    3, NULL,                           HS_FEATHER7         }, // HS_FEATHER6
    { HSPR_CHKN, 14,                    3, NULL,                           HS_FEATHER8         }, // HS_FEATHER7
    { HSPR_CHKN, 13,                    3, NULL,                           HS_FEATHER1         }, // HS_FEATHER8
    { HSPR_CHKN, 13,                    6, NULL,                           HS_NULL             }, // HS_FEATHERX
    { HSPR_MUMM, 0,                    10, A_Look,                         HS_MUMMY_LOOK2      }, // HS_MUMMY_LOOK1
    { HSPR_MUMM, 1,                    10, A_Look,                         HS_MUMMY_LOOK1      }, // HS_MUMMY_LOOK2
    { HSPR_MUMM, 0,                     4, A_Chase,                        HS_MUMMY_WALK2      }, // HS_MUMMY_WALK1
    { HSPR_MUMM, 1,                     4, A_Chase,                        HS_MUMMY_WALK3      }, // HS_MUMMY_WALK2
    { HSPR_MUMM, 2,                     4, A_Chase,                        HS_MUMMY_WALK4      }, // HS_MUMMY_WALK3
    { HSPR_MUMM, 3,                     4, A_Chase,                        HS_MUMMY_WALK1      }, // HS_MUMMY_WALK4
    { HSPR_MUMM, 4,                     6, A_FaceTarget,                   HS_MUMMY_ATK2       }, // HS_MUMMY_ATK1
    { HSPR_MUMM, 5,                     6, NULL/*A_MummyAttack*/,          HS_MUMMY_ATK3       }, // HS_MUMMY_ATK2
    { HSPR_MUMM, 6,                     6, A_FaceTarget,                   HS_MUMMY_WALK1      }, // HS_MUMMY_ATK3
    { HSPR_MUMM, 23,                    5, A_FaceTarget,                   HS_MUMMYL_ATK2      }, // HS_MUMMYL_ATK1
    { HSPR_MUMM, 24 | FF_FULLBRIGHT,    5, A_FaceTarget,                   HS_MUMMYL_ATK3      }, // HS_MUMMYL_ATK2
    { HSPR_MUMM, 23,                    5, A_FaceTarget,                   HS_MUMMYL_ATK4      }, // HS_MUMMYL_ATK3
    { HSPR_MUMM, 24 | FF_FULLBRIGHT,    5, A_FaceTarget,                   HS_MUMMYL_ATK5      }, // HS_MUMMYL_ATK4
    { HSPR_MUMM, 23,                    5, A_FaceTarget,                   HS_MUMMYL_ATK6      }, // HS_MUMMYL_ATK5
    { HSPR_MUMM, 24 | FF_FULLBRIGHT,   15, NULL/*A_MummyAttack2*/,         HS_MUMMY_WALK1      }, // HS_MUMMYL_ATK6
    { HSPR_MUMM, 7,                     4, NULL,                           HS_MUMMY_PAIN2      }, // HS_MUMMY_PAIN1
    { HSPR_MUMM, 7,                     4, A_Pain,                         HS_MUMMY_WALK1      }, // HS_MUMMY_PAIN2
    { HSPR_MUMM, 8,                     5, NULL,                           HS_MUMMY_DIE2       }, // HS_MUMMY_DIE1
    { HSPR_MUMM, 9,                     5, A_Scream,                       HS_MUMMY_DIE3       }, // HS_MUMMY_DIE2
    { HSPR_MUMM, 10,                    5, NULL/*A_MummySoul*/,            HS_MUMMY_DIE4       }, // HS_MUMMY_DIE3
    { HSPR_MUMM, 11,                    5, NULL,                           HS_MUMMY_DIE5       }, // HS_MUMMY_DIE4
    { HSPR_MUMM, 12,                    5, NULL/*A_NoBlocking*/,           HS_MUMMY_DIE6       }, // HS_MUMMY_DIE5
    { HSPR_MUMM, 13,                    5, NULL,                           HS_MUMMY_DIE7       }, // HS_MUMMY_DIE6
    { HSPR_MUMM, 14,                    5, NULL,                           HS_MUMMY_DIE8       }, // HS_MUMMY_DIE7
    { HSPR_MUMM, 15,                   -1, NULL,                           HS_NULL             }, // HS_MUMMY_DIE8
    { HSPR_MUMM, 16,                    5, NULL,                           HS_MUMMY_SOUL2      }, // HS_MUMMY_SOUL1
    { HSPR_MUMM, 17,                    5, NULL,                           HS_MUMMY_SOUL3      }, // HS_MUMMY_SOUL2
    { HSPR_MUMM, 18,                    5, NULL,                           HS_MUMMY_SOUL4      }, // HS_MUMMY_SOUL3
    { HSPR_MUMM, 19,                    9, NULL,                           HS_MUMMY_SOUL5      }, // HS_MUMMY_SOUL4
    { HSPR_MUMM, 20,                    5, NULL,                           HS_MUMMY_SOUL6      }, // HS_MUMMY_SOUL5
    { HSPR_MUMM, 21,                    5, NULL,                           HS_MUMMY_SOUL7      }, // HS_MUMMY_SOUL6
    { HSPR_MUMM, 22,                    5, NULL,                           HS_NULL             }, // HS_MUMMY_SOUL7
    { HSPR_FX15, 0 | FF_FULLBRIGHT,     5, NULL/*A_ContMobjSound*/,        HS_MUMMYFX1_2       }, // HS_MUMMYFX1_1
    { HSPR_FX15, 1 | FF_FULLBRIGHT,     5, NULL/*A_MummyFX1Seek*/,         HS_MUMMYFX1_3       }, // HS_MUMMYFX1_2
    { HSPR_FX15, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_MUMMYFX1_4       }, // HS_MUMMYFX1_3
    { HSPR_FX15, 1 | FF_FULLBRIGHT,     5, NULL/*A_MummyFX1Seek*/,         HS_MUMMYFX1_1       }, // HS_MUMMYFX1_4
    { HSPR_FX15, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_MUMMYFXI1_2      }, // HS_MUMMYFXI1_1
    { HSPR_FX15, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_MUMMYFXI1_3      }, // HS_MUMMYFXI1_2
    { HSPR_FX15, 5 | FF_FULLBRIGHT,     5, NULL,                           HS_MUMMYFXI1_4      }, // HS_MUMMYFXI1_3
    { HSPR_FX15, 6 | FF_FULLBRIGHT,     5, NULL,                           HS_NULL             }, // HS_MUMMYFXI1_4
    { HSPR_BEAS, 0,                    10, A_Look,                         HS_BEAST_LOOK2      }, // HS_BEAST_LOOK1
    { HSPR_BEAS, 1,                    10, A_Look,                         HS_BEAST_LOOK1      }, // HS_BEAST_LOOK2
    { HSPR_BEAS, 0,                     3, A_Chase,                        HS_BEAST_WALK2      }, // HS_BEAST_WALK1
    { HSPR_BEAS, 1,                     3, A_Chase,                        HS_BEAST_WALK3      }, // HS_BEAST_WALK2
    { HSPR_BEAS, 2,                     3, A_Chase,                        HS_BEAST_WALK4      }, // HS_BEAST_WALK3
    { HSPR_BEAS, 3,                     3, A_Chase,                        HS_BEAST_WALK5      }, // HS_BEAST_WALK4
    { HSPR_BEAS, 4,                     3, A_Chase,                        HS_BEAST_WALK6      }, // HS_BEAST_WALK5
    { HSPR_BEAS, 5,                     3, A_Chase,                        HS_BEAST_WALK1      }, // HS_BEAST_WALK6
    { HSPR_BEAS, 7,                    10, A_FaceTarget,                   HS_BEAST_ATK2       }, // HS_BEAST_ATK1
    { HSPR_BEAS, 8,                    10, NULL/*A_BeastAttack*/,          HS_BEAST_WALK1      }, // HS_BEAST_ATK2
    { HSPR_BEAS, 6,                     3, NULL,                           HS_BEAST_PAIN2      }, // HS_BEAST_PAIN1
    { HSPR_BEAS, 6,                     3, A_Pain,                         HS_BEAST_WALK1      }, // HS_BEAST_PAIN2
    { HSPR_BEAS, 17,                    6, NULL,                           HS_BEAST_DIE2       }, // HS_BEAST_DIE1
    { HSPR_BEAS, 18,                    6, A_Scream,                       HS_BEAST_DIE3       }, // HS_BEAST_DIE2
    { HSPR_BEAS, 19,                    6, NULL,                           HS_BEAST_DIE4       }, // HS_BEAST_DIE3
    { HSPR_BEAS, 20,                    6, NULL,                           HS_BEAST_DIE5       }, // HS_BEAST_DIE4
    { HSPR_BEAS, 21,                    6, NULL,                           HS_BEAST_DIE6       }, // HS_BEAST_DIE5
    { HSPR_BEAS, 22,                    6, NULL/*A_NoBlocking*/,           HS_BEAST_DIE7       }, // HS_BEAST_DIE6
    { HSPR_BEAS, 23,                    6, NULL,                           HS_BEAST_DIE8       }, // HS_BEAST_DIE7
    { HSPR_BEAS, 24,                    6, NULL,                           HS_BEAST_DIE9       }, // HS_BEAST_DIE8
    { HSPR_BEAS, 25,                   -1, NULL,                           HS_NULL             }, // HS_BEAST_DIE9
    { HSPR_BEAS, 9,                     5, NULL,                           HS_BEAST_XDIE2      }, // HS_BEAST_XDIE1
    { HSPR_BEAS, 10,                    6, A_Scream,                       HS_BEAST_XDIE3      }, // HS_BEAST_XDIE2
    { HSPR_BEAS, 11,                    5, NULL,                           HS_BEAST_XDIE4      }, // HS_BEAST_XDIE3
    { HSPR_BEAS, 12,                    6, NULL,                           HS_BEAST_XDIE5      }, // HS_BEAST_XDIE4
    { HSPR_BEAS, 13,                    5, NULL,                           HS_BEAST_XDIE6      }, // HS_BEAST_XDIE5
    { HSPR_BEAS, 14,                    6, NULL/*A_NoBlocking*/,           HS_BEAST_XDIE7      }, // HS_BEAST_XDIE6
    { HSPR_BEAS, 15,                    5, NULL,                           HS_BEAST_XDIE8      }, // HS_BEAST_XDIE7
    { HSPR_BEAS, 16,                   -1, NULL,                           HS_NULL             }, // HS_BEAST_XDIE8
    { HSPR_FRB1, 0,                     2, NULL/*A_BeastPuff*/,            HS_BEASTBALL2       }, // HS_BEASTBALL1
    { HSPR_FRB1, 0,                     2, NULL/*A_BeastPuff*/,            HS_BEASTBALL3       }, // HS_BEASTBALL2
    { HSPR_FRB1, 1,                     2, NULL/*A_BeastPuff*/,            HS_BEASTBALL4       }, // HS_BEASTBALL3
    { HSPR_FRB1, 1,                     2, NULL/*A_BeastPuff*/,            HS_BEASTBALL5       }, // HS_BEASTBALL4
    { HSPR_FRB1, 2,                     2, NULL/*A_BeastPuff*/,            HS_BEASTBALL6       }, // HS_BEASTBALL5
    { HSPR_FRB1, 2,                     2, NULL/*A_BeastPuff*/,            HS_BEASTBALL1       }, // HS_BEASTBALL6
    { HSPR_FRB1, 3,                     4, NULL,                           HS_BEASTBALLX2      }, // HS_BEASTBALLX1
    { HSPR_FRB1, 4,                     4, NULL,                           HS_BEASTBALLX3      }, // HS_BEASTBALLX2
    { HSPR_FRB1, 5,                     4, NULL,                           HS_BEASTBALLX4      }, // HS_BEASTBALLX3
    { HSPR_FRB1, 6,                     4, NULL,                           HS_BEASTBALLX5      }, // HS_BEASTBALLX4
    { HSPR_FRB1, 7,                     4, NULL,                           HS_NULL             }, // HS_BEASTBALLX5
    { HSPR_FRB1, 0,                     4, NULL,                           HS_BURNBALL2        }, // HS_BURNBALL1
    { HSPR_FRB1, 1,                     4, NULL,                           HS_BURNBALL3        }, // HS_BURNBALL2
    { HSPR_FRB1, 2,                     4, NULL,                           HS_BURNBALL4        }, // HS_BURNBALL3
    { HSPR_FRB1, 3,                     4, NULL,                           HS_BURNBALL5        }, // HS_BURNBALL4
    { HSPR_FRB1, 4,                     4, NULL,                           HS_BURNBALL6        }, // HS_BURNBALL5
    { HSPR_FRB1, 5,                     4, NULL,                           HS_BURNBALL7        }, // HS_BURNBALL6
    { HSPR_FRB1, 6,                     4, NULL,                           HS_BURNBALL8        }, // HS_BURNBALL7
    { HSPR_FRB1, 7,                     4, NULL,                           HS_NULL             }, // HS_BURNBALL8
    { HSPR_FRB1, 0 | FF_FULLBRIGHT,     4, NULL,                           HS_BURNBALLFB2      }, // HS_BURNBALLFB1
    { HSPR_FRB1, 1 | FF_FULLBRIGHT,     4, NULL,                           HS_BURNBALLFB3      }, // HS_BURNBALLFB2
    { HSPR_FRB1, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_BURNBALLFB4      }, // HS_BURNBALLFB3
    { HSPR_FRB1, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_BURNBALLFB5      }, // HS_BURNBALLFB4
    { HSPR_FRB1, 4 | FF_FULLBRIGHT,     4, NULL,                           HS_BURNBALLFB6      }, // HS_BURNBALLFB5
    { HSPR_FRB1, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_BURNBALLFB7      }, // HS_BURNBALLFB6
    { HSPR_FRB1, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_BURNBALLFB8      }, // HS_BURNBALLFB7
    { HSPR_FRB1, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_BURNBALLFB8
    { HSPR_FRB1, 3,                     4, NULL,                           HS_PUFFY2           }, // HS_PUFFY1
    { HSPR_FRB1, 4,                     4, NULL,                           HS_PUFFY3           }, // HS_PUFFY2
    { HSPR_FRB1, 5,                     4, NULL,                           HS_PUFFY4           }, // HS_PUFFY3
    { HSPR_FRB1, 6,                     4, NULL,                           HS_PUFFY5           }, // HS_PUFFY4
    { HSPR_FRB1, 7,                     4, NULL,                           HS_NULL             }, // HS_PUFFY5
    { HSPR_SNKE, 0,                    10, A_Look,                         HS_SNAKE_LOOK2      }, // HS_SNAKE_LOOK1
    { HSPR_SNKE, 1,                    10, A_Look,                         HS_SNAKE_LOOK1      }, // HS_SNAKE_LOOK2
    { HSPR_SNKE, 0,                     4, A_Chase,                        HS_SNAKE_WALK2      }, // HS_SNAKE_WALK1
    { HSPR_SNKE, 1,                     4, A_Chase,                        HS_SNAKE_WALK3      }, // HS_SNAKE_WALK2
    { HSPR_SNKE, 2,                     4, A_Chase,                        HS_SNAKE_WALK4      }, // HS_SNAKE_WALK3
    { HSPR_SNKE, 3,                     4, A_Chase,                        HS_SNAKE_WALK1      }, // HS_SNAKE_WALK4
    { HSPR_SNKE, 5,                     5, A_FaceTarget,                   HS_SNAKE_ATK2       }, // HS_SNAKE_ATK1
    { HSPR_SNKE, 5,                     5, A_FaceTarget,                   HS_SNAKE_ATK3       }, // HS_SNAKE_ATK2
    { HSPR_SNKE, 5,                     4, NULL/*A_SnakeAttack*/,          HS_SNAKE_ATK4       }, // HS_SNAKE_ATK3
    { HSPR_SNKE, 5,                     4, NULL/*A_SnakeAttack*/,          HS_SNAKE_ATK5       }, // HS_SNAKE_ATK4
    { HSPR_SNKE, 5,                     4, NULL/*A_SnakeAttack*/,          HS_SNAKE_ATK6       }, // HS_SNAKE_ATK5
    { HSPR_SNKE, 5,                     5, A_FaceTarget,                   HS_SNAKE_ATK7       }, // HS_SNAKE_ATK6
    { HSPR_SNKE, 5,                     5, A_FaceTarget,                   HS_SNAKE_ATK8       }, // HS_SNAKE_ATK7
    { HSPR_SNKE, 5,                     5, A_FaceTarget,                   HS_SNAKE_ATK9       }, // HS_SNAKE_ATK8
    { HSPR_SNKE, 5,                     4, NULL/*A_SnakeAttack2*/,         HS_SNAKE_WALK1      }, // HS_SNAKE_ATK9
    { HSPR_SNKE, 4,                     3, NULL,                           HS_SNAKE_PAIN2      }, // HS_SNAKE_PAIN1
    { HSPR_SNKE, 4,                     3, A_Pain,                         HS_SNAKE_WALK1      }, // HS_SNAKE_PAIN2
    { HSPR_SNKE, 6,                     5, NULL,                           HS_SNAKE_DIE2       }, // HS_SNAKE_DIE1
    { HSPR_SNKE, 7,                     5, A_Scream,                       HS_SNAKE_DIE3       }, // HS_SNAKE_DIE2
    { HSPR_SNKE, 8,                     5, NULL,                           HS_SNAKE_DIE4       }, // HS_SNAKE_DIE3
    { HSPR_SNKE, 9,                     5, NULL,                           HS_SNAKE_DIE5       }, // HS_SNAKE_DIE4
    { HSPR_SNKE, 10,                    5, NULL,                           HS_SNAKE_DIE6       }, // HS_SNAKE_DIE5
    { HSPR_SNKE, 11,                    5, NULL,                           HS_SNAKE_DIE7       }, // HS_SNAKE_DIE6
    { HSPR_SNKE, 12,                    5, NULL/*A_NoBlocking*/,           HS_SNAKE_DIE8       }, // HS_SNAKE_DIE7
    { HSPR_SNKE, 13,                    5, NULL,                           HS_SNAKE_DIE9       }, // HS_SNAKE_DIE8
    { HSPR_SNKE, 14,                    5, NULL,                           HS_SNAKE_DIE10      }, // HS_SNAKE_DIE9
    { HSPR_SNKE, 15,                   -1, NULL,                           HS_NULL             }, // HS_SNAKE_DIE10
    { HSPR_SNFX, 0 | FF_FULLBRIGHT,     5, NULL,                           HS_SNAKEPRO_A2      }, // HS_SNAKEPRO_A1
    { HSPR_SNFX, 1 | FF_FULLBRIGHT,     5, NULL,                           HS_SNAKEPRO_A3      }, // HS_SNAKEPRO_A2
    { HSPR_SNFX, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_SNAKEPRO_A4      }, // HS_SNAKEPRO_A3
    { HSPR_SNFX, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_SNAKEPRO_A1      }, // HS_SNAKEPRO_A4
    { HSPR_SNFX, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_SNAKEPRO_AX2     }, // HS_SNAKEPRO_AX1
    { HSPR_SNFX, 5 | FF_FULLBRIGHT,     5, NULL,                           HS_SNAKEPRO_AX3     }, // HS_SNAKEPRO_AX2
    { HSPR_SNFX, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_SNAKEPRO_AX4     }, // HS_SNAKEPRO_AX3
    { HSPR_SNFX, 7 | FF_FULLBRIGHT,     3, NULL,                           HS_SNAKEPRO_AX5     }, // HS_SNAKEPRO_AX4
    { HSPR_SNFX, 8 | FF_FULLBRIGHT,     3, NULL,                           HS_NULL             }, // HS_SNAKEPRO_AX5
    { HSPR_SNFX, 9 | FF_FULLBRIGHT,     6, NULL,                           HS_SNAKEPRO_B2      }, // HS_SNAKEPRO_B1
    { HSPR_SNFX, 10 | FF_FULLBRIGHT,    6, NULL,                           HS_SNAKEPRO_B1      }, // HS_SNAKEPRO_B2
    { HSPR_SNFX, 11 | FF_FULLBRIGHT,    5, NULL,                           HS_SNAKEPRO_BX2     }, // HS_SNAKEPRO_BX1
    { HSPR_SNFX, 12 | FF_FULLBRIGHT,    5, NULL,                           HS_SNAKEPRO_BX3     }, // HS_SNAKEPRO_BX2
    { HSPR_SNFX, 13 | FF_FULLBRIGHT,    4, NULL,                           HS_SNAKEPRO_BX4     }, // HS_SNAKEPRO_BX3
    { HSPR_SNFX, 14 | FF_FULLBRIGHT,    3, NULL,                           HS_NULL             }, // HS_SNAKEPRO_BX4
    { HSPR_HEAD, 0,                    10, A_Look,                         HS_HEAD_LOOK        }, // HS_HEAD_LOOK
    { HSPR_HEAD, 0,                     4, A_Chase,                        HS_HEAD_FLOAT       }, // HS_HEAD_FLOAT
    { HSPR_HEAD, 0,                     5, A_FaceTarget,                   HS_HEAD_ATK2        }, // HS_HEAD_ATK1
    { HSPR_HEAD, 1,                    20, NULL/*A_HeadAttack*/,           HS_HEAD_FLOAT       }, // HS_HEAD_ATK2
    { HSPR_HEAD, 0,                     4, NULL,                           HS_HEAD_PAIN2       }, // HS_HEAD_PAIN1
    { HSPR_HEAD, 0,                     4, A_Pain,                         HS_HEAD_FLOAT       }, // HS_HEAD_PAIN2
    { HSPR_HEAD, 2,                     7, NULL,                           HS_HEAD_DIE2        }, // HS_HEAD_DIE1
    { HSPR_HEAD, 3,                     7, A_Scream,                       HS_HEAD_DIE3        }, // HS_HEAD_DIE2
    { HSPR_HEAD, 4,                     7, NULL,                           HS_HEAD_DIE4        }, // HS_HEAD_DIE3
    { HSPR_HEAD, 5,                     7, NULL,                           HS_HEAD_DIE5        }, // HS_HEAD_DIE4
    { HSPR_HEAD, 6,                     7, NULL/*A_NoBlocking*/,           HS_HEAD_DIE6        }, // HS_HEAD_DIE5
    { HSPR_HEAD, 7,                     7, NULL,                           HS_HEAD_DIE7        }, // HS_HEAD_DIE6
    { HSPR_HEAD, 8,                    -1, NULL/*A_BossDeath*/,            HS_NULL             }, // HS_HEAD_DIE7
    { HSPR_FX05, 0,                     6, NULL,                           HS_HEADFX1_2        }, // HS_HEADFX1_1
    { HSPR_FX05, 1,                     6, NULL,                           HS_HEADFX1_3        }, // HS_HEADFX1_2
    { HSPR_FX05, 2,                     6, NULL,                           HS_HEADFX1_1        }, // HS_HEADFX1_3
    { HSPR_FX05, 3,                     5, NULL/*A_HeadIceImpact*/,        HS_HEADFXI1_2       }, // HS_HEADFXI1_1
    { HSPR_FX05, 4,                     5, NULL,                           HS_HEADFXI1_3       }, // HS_HEADFXI1_2
    { HSPR_FX05, 5,                     5, NULL,                           HS_HEADFXI1_4       }, // HS_HEADFXI1_3
    { HSPR_FX05, 6,                     5, NULL,                           HS_NULL             }, // HS_HEADFXI1_4
    { HSPR_FX05, 7,                     6, NULL,                           HS_HEADFX2_2        }, // HS_HEADFX2_1
    { HSPR_FX05, 8,                     6, NULL,                           HS_HEADFX2_3        }, // HS_HEADFX2_2
    { HSPR_FX05, 9,                     6, NULL,                           HS_HEADFX2_1        }, // HS_HEADFX2_3
    { HSPR_FX05, 3,                     5, NULL,                           HS_HEADFXI2_2       }, // HS_HEADFXI2_1
    { HSPR_FX05, 4,                     5, NULL,                           HS_HEADFXI2_3       }, // HS_HEADFXI2_2
    { HSPR_FX05, 5,                     5, NULL,                           HS_HEADFXI2_4       }, // HS_HEADFXI2_3
    { HSPR_FX05, 6,                     5, NULL,                           HS_NULL             }, // HS_HEADFXI2_4
    { HSPR_FX06, 0,                     4, NULL/*A_HeadFireGrow*/,         HS_HEADFX3_2        }, // HS_HEADFX3_1
    { HSPR_FX06, 1,                     4, NULL/*A_HeadFireGrow*/,         HS_HEADFX3_3        }, // HS_HEADFX3_2
    { HSPR_FX06, 2,                     4, NULL/*A_HeadFireGrow*/,         HS_HEADFX3_1        }, // HS_HEADFX3_3
    { HSPR_FX06, 0,                     5, NULL,                           HS_HEADFX3_5        }, // HS_HEADFX3_4
    { HSPR_FX06, 1,                     5, NULL,                           HS_HEADFX3_6        }, // HS_HEADFX3_5
    { HSPR_FX06, 2,                     5, NULL,                           HS_HEADFX3_4        }, // HS_HEADFX3_6
    { HSPR_FX06, 3,                     5, NULL,                           HS_HEADFXI3_2       }, // HS_HEADFXI3_1
    { HSPR_FX06, 4,                     5, NULL,                           HS_HEADFXI3_3       }, // HS_HEADFXI3_2
    { HSPR_FX06, 5,                     5, NULL,                           HS_HEADFXI3_4       }, // HS_HEADFXI3_3
    { HSPR_FX06, 6,                     5, NULL,                           HS_NULL             }, // HS_HEADFXI3_4
    { HSPR_FX07, 3,                     3, NULL,                           HS_HEADFX4_2        }, // HS_HEADFX4_1
    { HSPR_FX07, 4,                     3, NULL,                           HS_HEADFX4_3        }, // HS_HEADFX4_2
    { HSPR_FX07, 5,                     3, NULL,                           HS_HEADFX4_4        }, // HS_HEADFX4_3
    { HSPR_FX07, 6,                     3, NULL,                           HS_HEADFX4_5        }, // HS_HEADFX4_4
    { HSPR_FX07, 0,                     3, NULL/*A_WhirlwindSeek*/,        HS_HEADFX4_6        }, // HS_HEADFX4_5
    { HSPR_FX07, 1,                     3, NULL/*A_WhirlwindSeek*/,        HS_HEADFX4_7        }, // HS_HEADFX4_6
    { HSPR_FX07, 2,                     3, NULL/*A_WhirlwindSeek*/,        HS_HEADFX4_5        }, // HS_HEADFX4_7
    { HSPR_FX07, 6,                     4, NULL,                           HS_HEADFXI4_2       }, // HS_HEADFXI4_1
    { HSPR_FX07, 5,                     4, NULL,                           HS_HEADFXI4_3       }, // HS_HEADFXI4_2
    { HSPR_FX07, 4,                     4, NULL,                           HS_HEADFXI4_4       }, // HS_HEADFXI4_3
    { HSPR_FX07, 3,                     4, NULL,                           HS_NULL             }, // HS_HEADFXI4_4
    { HSPR_CLNK, 0,                    10, A_Look,                         HS_CLINK_LOOK2      }, // HS_CLINK_LOOK1
    { HSPR_CLNK, 1,                    10, A_Look,                         HS_CLINK_LOOK1      }, // HS_CLINK_LOOK2
    { HSPR_CLNK, 0,                     3, A_Chase,                        HS_CLINK_WALK2      }, // HS_CLINK_WALK1
    { HSPR_CLNK, 1,                     3, A_Chase,                        HS_CLINK_WALK3      }, // HS_CLINK_WALK2
    { HSPR_CLNK, 2,                     3, A_Chase,                        HS_CLINK_WALK4      }, // HS_CLINK_WALK3
    { HSPR_CLNK, 3,                     3, A_Chase,                        HS_CLINK_WALK1      }, // HS_CLINK_WALK4
    { HSPR_CLNK, 4,                     5, A_FaceTarget,                   HS_CLINK_ATK2       }, // HS_CLINK_ATK1
    { HSPR_CLNK, 5,                     4, A_FaceTarget,                   HS_CLINK_ATK3       }, // HS_CLINK_ATK2
    { HSPR_CLNK, 6,                     7, NULL/*A_ClinkAttack*/,          HS_CLINK_WALK1      }, // HS_CLINK_ATK3
    { HSPR_CLNK, 7,                     3, NULL,                           HS_CLINK_PAIN2      }, // HS_CLINK_PAIN1
    { HSPR_CLNK, 7,                     3, A_Pain,                         HS_CLINK_WALK1      }, // HS_CLINK_PAIN2
    { HSPR_CLNK, 8,                     6, NULL,                           HS_CLINK_DIE2       }, // HS_CLINK_DIE1
    { HSPR_CLNK, 9,                     6, NULL,                           HS_CLINK_DIE3       }, // HS_CLINK_DIE2
    { HSPR_CLNK, 10,                    5, A_Scream,                       HS_CLINK_DIE4       }, // HS_CLINK_DIE3
    { HSPR_CLNK, 11,                    5, NULL/*A_NoBlocking*/,           HS_CLINK_DIE5       }, // HS_CLINK_DIE4
    { HSPR_CLNK, 12,                    5, NULL,                           HS_CLINK_DIE6       }, // HS_CLINK_DIE5
    { HSPR_CLNK, 13,                    5, NULL,                           HS_CLINK_DIE7       }, // HS_CLINK_DIE6
    { HSPR_CLNK, 14,                   -1, NULL,                           HS_NULL             }, // HS_CLINK_DIE7
    { HSPR_WZRD, 0,                    10, A_Look,                         HS_WIZARD_LOOK2     }, // HS_WIZARD_LOOK1
    { HSPR_WZRD, 1,                    10, A_Look,                         HS_WIZARD_LOOK1     }, // HS_WIZARD_LOOK2
    { HSPR_WZRD, 0,                     3, A_Chase,                        HS_WIZARD_WALK2     }, // HS_WIZARD_WALK1
    { HSPR_WZRD, 0,                     4, A_Chase,                        HS_WIZARD_WALK3     }, // HS_WIZARD_WALK2
    { HSPR_WZRD, 0,                     3, A_Chase,                        HS_WIZARD_WALK4     }, // HS_WIZARD_WALK3
    { HSPR_WZRD, 0,                     4, A_Chase,                        HS_WIZARD_WALK5     }, // HS_WIZARD_WALK4
    { HSPR_WZRD, 1,                     3, A_Chase,                        HS_WIZARD_WALK6     }, // HS_WIZARD_WALK5
    { HSPR_WZRD, 1,                     4, A_Chase,                        HS_WIZARD_WALK7     }, // HS_WIZARD_WALK6
    { HSPR_WZRD, 1,                     3, A_Chase,                        HS_WIZARD_WALK8     }, // HS_WIZARD_WALK7
    { HSPR_WZRD, 1,                     4, A_Chase,                        HS_WIZARD_WALK1     }, // HS_WIZARD_WALK8
    { HSPR_WZRD, 2,                     4, NULL/*A_WizAtk1*/,              HS_WIZARD_ATK2      }, // HS_WIZARD_ATK1
    { HSPR_WZRD, 2,                     4, NULL/*A_WizAtk2*/,              HS_WIZARD_ATK3      }, // HS_WIZARD_ATK2
    { HSPR_WZRD, 2,                     4, NULL/*A_WizAtk1*/,              HS_WIZARD_ATK4      }, // HS_WIZARD_ATK3
    { HSPR_WZRD, 2,                     4, NULL/*A_WizAtk2*/,              HS_WIZARD_ATK5      }, // HS_WIZARD_ATK4
    { HSPR_WZRD, 2,                     4, NULL/*A_WizAtk1*/,              HS_WIZARD_ATK6      }, // HS_WIZARD_ATK5
    { HSPR_WZRD, 2,                     4, NULL/*A_WizAtk2*/,              HS_WIZARD_ATK7      }, // HS_WIZARD_ATK6
    { HSPR_WZRD, 2,                     4, NULL/*A_WizAtk1*/,              HS_WIZARD_ATK8      }, // HS_WIZARD_ATK7
    { HSPR_WZRD, 2,                     4, NULL/*A_WizAtk2*/,              HS_WIZARD_ATK9      }, // HS_WIZARD_ATK8
    { HSPR_WZRD, 3,                    12, NULL/*A_WizAtk3*/,              HS_WIZARD_WALK1     }, // HS_WIZARD_ATK9
    { HSPR_WZRD, 4,                     3, NULL/*A_GhostOff*/,             HS_WIZARD_PAIN2     }, // HS_WIZARD_PAIN1
    { HSPR_WZRD, 4,                     3, A_Pain,                         HS_WIZARD_WALK1     }, // HS_WIZARD_PAIN2
    { HSPR_WZRD, 5,                     6, NULL/*A_GhostOff*/,             HS_WIZARD_DIE2      }, // HS_WIZARD_DIE1
    { HSPR_WZRD, 6,                     6, A_Scream,                       HS_WIZARD_DIE3      }, // HS_WIZARD_DIE2
    { HSPR_WZRD, 7,                     6, NULL,                           HS_WIZARD_DIE4      }, // HS_WIZARD_DIE3
    { HSPR_WZRD, 8,                     6, NULL,                           HS_WIZARD_DIE5      }, // HS_WIZARD_DIE4
    { HSPR_WZRD, 9,                     6, NULL/*A_NoBlocking*/,           HS_WIZARD_DIE6      }, // HS_WIZARD_DIE5
    { HSPR_WZRD, 10,                    6, NULL,                           HS_WIZARD_DIE7      }, // HS_WIZARD_DIE6
    { HSPR_WZRD, 11,                    6, NULL,                           HS_WIZARD_DIE8      }, // HS_WIZARD_DIE7
    { HSPR_WZRD, 12,                   -1, NULL,                           HS_NULL             }, // HS_WIZARD_DIE8
    { HSPR_FX11, 0 | FF_FULLBRIGHT,     6, NULL,                           HS_WIZFX1_2         }, // HS_WIZFX1_1
    { HSPR_FX11, 1 | FF_FULLBRIGHT,     6, NULL,                           HS_WIZFX1_1         }, // HS_WIZFX1_2
    { HSPR_FX11, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_WIZFXI1_2        }, // HS_WIZFXI1_1
    { HSPR_FX11, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_WIZFXI1_3        }, // HS_WIZFXI1_2
    { HSPR_FX11, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_WIZFXI1_4        }, // HS_WIZFXI1_3
    { HSPR_FX11, 5 | FF_FULLBRIGHT,     5, NULL,                           HS_WIZFXI1_5        }, // HS_WIZFXI1_4
    { HSPR_FX11, 6 | FF_FULLBRIGHT,     5, NULL,                           HS_NULL             }, // HS_WIZFXI1_5
    { HSPR_IMPX, 0,                    10, A_Look,                         HS_IMP_LOOK2        }, // HS_IMP_LOOK1
    { HSPR_IMPX, 1,                    10, A_Look,                         HS_IMP_LOOK3        }, // HS_IMP_LOOK2
    { HSPR_IMPX, 2,                    10, A_Look,                         HS_IMP_LOOK4        }, // HS_IMP_LOOK3
    { HSPR_IMPX, 1,                    10, A_Look,                         HS_IMP_LOOK1        }, // HS_IMP_LOOK4
    { HSPR_IMPX, 0,                     3, A_Chase,                        HS_IMP_FLY2         }, // HS_IMP_FLY1
    { HSPR_IMPX, 0,                     3, A_Chase,                        HS_IMP_FLY3         }, // HS_IMP_FLY2
    { HSPR_IMPX, 1,                     3, A_Chase,                        HS_IMP_FLY4         }, // HS_IMP_FLY3
    { HSPR_IMPX, 1,                     3, A_Chase,                        HS_IMP_FLY5         }, // HS_IMP_FLY4
    { HSPR_IMPX, 2,                     3, A_Chase,                        HS_IMP_FLY6         }, // HS_IMP_FLY5
    { HSPR_IMPX, 2,                     3, A_Chase,                        HS_IMP_FLY7         }, // HS_IMP_FLY6
    { HSPR_IMPX, 1,                     3, A_Chase,                        HS_IMP_FLY8         }, // HS_IMP_FLY7
    { HSPR_IMPX, 1,                     3, A_Chase,                        HS_IMP_FLY1         }, // HS_IMP_FLY8
    { HSPR_IMPX, 3,                     6, A_FaceTarget,                   HS_IMP_MEATK2       }, // HS_IMP_MEATK1
    { HSPR_IMPX, 4,                     6, A_FaceTarget,                   HS_IMP_MEATK3       }, // HS_IMP_MEATK2
    { HSPR_IMPX, 5,                     6, NULL/*A_ImpMeAttack*/,          HS_IMP_FLY1         }, // HS_IMP_MEATK3
    { HSPR_IMPX, 0,                    10, A_FaceTarget,                   HS_IMP_MSATK1_2     }, // HS_IMP_MSATK1_1
    { HSPR_IMPX, 1,                     6, NULL/*A_ImpMsAttack*/,          HS_IMP_MSATK1_3     }, // HS_IMP_MSATK1_2
    { HSPR_IMPX, 2,                     6, NULL,                           HS_IMP_MSATK1_4     }, // HS_IMP_MSATK1_3
    { HSPR_IMPX, 1,                     6, NULL,                           HS_IMP_MSATK1_5     }, // HS_IMP_MSATK1_4
    { HSPR_IMPX, 0,                     6, NULL,                           HS_IMP_MSATK1_6     }, // HS_IMP_MSATK1_5
    { HSPR_IMPX, 1,                     6, NULL,                           HS_IMP_MSATK1_3     }, // HS_IMP_MSATK1_6
    { HSPR_IMPX, 3,                     6, A_FaceTarget,                   HS_IMP_MSATK2_2     }, // HS_IMP_MSATK2_1
    { HSPR_IMPX, 4,                     6, A_FaceTarget,                   HS_IMP_MSATK2_3     }, // HS_IMP_MSATK2_2
    { HSPR_IMPX, 5,                     6, NULL/*A_ImpMsAttack2*/,         HS_IMP_FLY1         }, // HS_IMP_MSATK2_3
    { HSPR_IMPX, 6,                     3, NULL,                           HS_IMP_PAIN2        }, // HS_IMP_PAIN1
    { HSPR_IMPX, 6,                     3, A_Pain,                         HS_IMP_FLY1         }, // HS_IMP_PAIN2
    { HSPR_IMPX, 6,                     4, NULL/*A_ImpDeath*/,             HS_IMP_DIE2         }, // HS_IMP_DIE1
    { HSPR_IMPX, 7,                     5, NULL,                           HS_IMP_DIE2         }, // HS_IMP_DIE2
    { HSPR_IMPX, 18,                    5, NULL/*A_ImpXDeath1*/,           HS_IMP_XDIE2        }, // HS_IMP_XDIE1
    { HSPR_IMPX, 19,                    5, NULL,                           HS_IMP_XDIE3        }, // HS_IMP_XDIE2
    { HSPR_IMPX, 20,                    5, NULL,                           HS_IMP_XDIE4        }, // HS_IMP_XDIE3
    { HSPR_IMPX, 21,                    5, NULL/*A_ImpXDeath2*/,           HS_IMP_XDIE5        }, // HS_IMP_XDIE4
    { HSPR_IMPX, 22,                    5, NULL,                           HS_IMP_XDIE5        }, // HS_IMP_XDIE5
    { HSPR_IMPX, 8,                     7, NULL/*A_ImpExplode*/,           HS_IMP_CRASH2       }, // HS_IMP_CRASH1
    { HSPR_IMPX, 9,                     7, A_Scream,                       HS_IMP_CRASH3       }, // HS_IMP_CRASH2
    { HSPR_IMPX, 10,                    7, NULL,                           HS_IMP_CRASH4       }, // HS_IMP_CRASH3
    { HSPR_IMPX, 11,                   -1, NULL,                           HS_NULL             }, // HS_IMP_CRASH4
    { HSPR_IMPX, 23,                    7, NULL,                           HS_IMP_XCRASH2      }, // HS_IMP_XCRASH1
    { HSPR_IMPX, 24,                    7, NULL,                           HS_IMP_XCRASH3      }, // HS_IMP_XCRASH2
    { HSPR_IMPX, 25,                   -1, NULL,                           HS_NULL             }, // HS_IMP_XCRASH3
    { HSPR_IMPX, 12,                    5, NULL,                           HS_IMP_CHUNKA2      }, // HS_IMP_CHUNKA1
    { HSPR_IMPX, 13,                  700, NULL,                           HS_IMP_CHUNKA3      }, // HS_IMP_CHUNKA2
    { HSPR_IMPX, 14,                  700, NULL,                           HS_NULL             }, // HS_IMP_CHUNKA3
    { HSPR_IMPX, 15,                    5, NULL,                           HS_IMP_CHUNKB2      }, // HS_IMP_CHUNKB1
    { HSPR_IMPX, 16,                  700, NULL,                           HS_IMP_CHUNKB3      }, // HS_IMP_CHUNKB2
    { HSPR_IMPX, 17,                  700, NULL,                           HS_NULL             }, // HS_IMP_CHUNKB3
    { HSPR_FX10, 0 | FF_FULLBRIGHT,     6, NULL,                           HS_IMPFX2           }, // HS_IMPFX1
    { HSPR_FX10, 1 | FF_FULLBRIGHT,     6, NULL,                           HS_IMPFX3           }, // HS_IMPFX2
    { HSPR_FX10, 2 | FF_FULLBRIGHT,     6, NULL,                           HS_IMPFX1           }, // HS_IMPFX3
    { HSPR_FX10, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_IMPFXI2          }, // HS_IMPFXI1
    { HSPR_FX10, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_IMPFXI3          }, // HS_IMPFXI2
    { HSPR_FX10, 5 | FF_FULLBRIGHT,     5, NULL,                           HS_IMPFXI4          }, // HS_IMPFXI3
    { HSPR_FX10, 6 | FF_FULLBRIGHT,     5, NULL,                           HS_NULL             }, // HS_IMPFXI4
    { HSPR_KNIG, 0,                    10, A_Look,                         HS_KNIGHT_STND2     }, // HS_KNIGHT_STND1
    { HSPR_KNIG, 1,                    10, A_Look,                         HS_KNIGHT_STND1     }, // HS_KNIGHT_STND2
    { HSPR_KNIG, 0,                     4, A_Chase,                        HS_KNIGHT_WALK2     }, // HS_KNIGHT_WALK1
    { HSPR_KNIG, 1,                     4, A_Chase,                        HS_KNIGHT_WALK3     }, // HS_KNIGHT_WALK2
    { HSPR_KNIG, 2,                     4, A_Chase,                        HS_KNIGHT_WALK4     }, // HS_KNIGHT_WALK3
    { HSPR_KNIG, 3,                     4, A_Chase,                        HS_KNIGHT_WALK1     }, // HS_KNIGHT_WALK4
    { HSPR_KNIG, 4,                    10, A_FaceTarget,                   HS_KNIGHT_ATK2      }, // HS_KNIGHT_ATK1
    { HSPR_KNIG, 5,                     8, A_FaceTarget,                   HS_KNIGHT_ATK3      }, // HS_KNIGHT_ATK2
    { HSPR_KNIG, 6,                     8, NULL/*A_KnightAttack*/,         HS_KNIGHT_ATK4      }, // HS_KNIGHT_ATK3
    { HSPR_KNIG, 4,                    10, A_FaceTarget,                   HS_KNIGHT_ATK5      }, // HS_KNIGHT_ATK4
    { HSPR_KNIG, 5,                     8, A_FaceTarget,                   HS_KNIGHT_ATK6      }, // HS_KNIGHT_ATK5
    { HSPR_KNIG, 6,                     8, NULL/*A_KnightAttack*/,         HS_KNIGHT_WALK1     }, // HS_KNIGHT_ATK6
    { HSPR_KNIG, 7,                     3, NULL,                           HS_KNIGHT_PAIN2     }, // HS_KNIGHT_PAIN1
    { HSPR_KNIG, 7,                     3, A_Pain,                         HS_KNIGHT_WALK1     }, // HS_KNIGHT_PAIN2
    { HSPR_KNIG, 8,                     6, NULL,                           HS_KNIGHT_DIE2      }, // HS_KNIGHT_DIE1
    { HSPR_KNIG, 9,                     6, A_Scream,                       HS_KNIGHT_DIE3      }, // HS_KNIGHT_DIE2
    { HSPR_KNIG, 10,                    6, NULL,                           HS_KNIGHT_DIE4      }, // HS_KNIGHT_DIE3
    { HSPR_KNIG, 11,                    6, NULL/*A_NoBlocking*/,           HS_KNIGHT_DIE5      }, // HS_KNIGHT_DIE4
    { HSPR_KNIG, 12,                    6, NULL,                           HS_KNIGHT_DIE6      }, // HS_KNIGHT_DIE5
    { HSPR_KNIG, 13,                    6, NULL,                           HS_KNIGHT_DIE7      }, // HS_KNIGHT_DIE6
    { HSPR_KNIG, 14,                   -1, NULL,                           HS_NULL             }, // HS_KNIGHT_DIE7
    { HSPR_SPAX, 0 | FF_FULLBRIGHT,     3, NULL/*A_ContMobjSound*/,        HS_SPINAXE2         }, // HS_SPINAXE1
    { HSPR_SPAX, 1 | FF_FULLBRIGHT,     3, NULL,                           HS_SPINAXE3         }, // HS_SPINAXE2
    { HSPR_SPAX, 2 | FF_FULLBRIGHT,     3, NULL,                           HS_SPINAXE1         }, // HS_SPINAXE3
    { HSPR_SPAX, 3 | FF_FULLBRIGHT,     6, NULL,                           HS_SPINAXEX2        }, // HS_SPINAXEX1
    { HSPR_SPAX, 4 | FF_FULLBRIGHT,     6, NULL,                           HS_SPINAXEX3        }, // HS_SPINAXEX2
    { HSPR_SPAX, 5 | FF_FULLBRIGHT,     6, NULL,                           HS_NULL             }, // HS_SPINAXEX3
    { HSPR_RAXE, 0 | FF_FULLBRIGHT,     5, NULL/*A_DripBlood*/,            HS_REDAXE2          }, // HS_REDAXE1
    { HSPR_RAXE, 1 | FF_FULLBRIGHT,     5, NULL/*A_DripBlood*/,            HS_REDAXE1          }, // HS_REDAXE2
    { HSPR_RAXE, 2 | FF_FULLBRIGHT,     6, NULL,                           HS_REDAXEX2         }, // HS_REDAXEX1
    { HSPR_RAXE, 3 | FF_FULLBRIGHT,     6, NULL,                           HS_REDAXEX3         }, // HS_REDAXEX2
    { HSPR_RAXE, 4 | FF_FULLBRIGHT,     6, NULL,                           HS_NULL             }, // HS_REDAXEX3
    { HSPR_SRCR, 0,                    10, A_Look,                         HS_SRCR1_LOOK2      }, // HS_SRCR1_LOOK1
    { HSPR_SRCR, 1,                    10, A_Look,                         HS_SRCR1_LOOK1      }, // HS_SRCR1_LOOK2
    { HSPR_SRCR, 0,                     5, NULL/*A_Sor1Chase*/,            HS_SRCR1_WALK2      }, // HS_SRCR1_WALK1
    { HSPR_SRCR, 1,                     5, NULL/*A_Sor1Chase*/,            HS_SRCR1_WALK3      }, // HS_SRCR1_WALK2
    { HSPR_SRCR, 2,                     5, NULL/*A_Sor1Chase*/,            HS_SRCR1_WALK4      }, // HS_SRCR1_WALK3
    { HSPR_SRCR, 3,                     5, NULL/*A_Sor1Chase*/,            HS_SRCR1_WALK1      }, // HS_SRCR1_WALK4
    { HSPR_SRCR, 16,                    6, NULL/*A_Sor1Pain*/,             HS_SRCR1_WALK1      }, // HS_SRCR1_PAIN1
    { HSPR_SRCR, 16,                    7, A_FaceTarget,                   HS_SRCR1_ATK2       }, // HS_SRCR1_ATK1
    { HSPR_SRCR, 17,                    6, A_FaceTarget,                   HS_SRCR1_ATK3       }, // HS_SRCR1_ATK2
    { HSPR_SRCR, 18,                   10, NULL/*A_Srcr1Attack*/,          HS_SRCR1_WALK1      }, // HS_SRCR1_ATK3
    { HSPR_SRCR, 18,                   10, A_FaceTarget,                   HS_SRCR1_ATK5       }, // HS_SRCR1_ATK4
    { HSPR_SRCR, 16,                    7, A_FaceTarget,                   HS_SRCR1_ATK6       }, // HS_SRCR1_ATK5
    { HSPR_SRCR, 17,                    6, A_FaceTarget,                   HS_SRCR1_ATK7       }, // HS_SRCR1_ATK6
    { HSPR_SRCR, 18,                   10, NULL/*A_Srcr1Attack*/,          HS_SRCR1_WALK1      }, // HS_SRCR1_ATK7
    { HSPR_SRCR, 4,                     7, NULL,                           HS_SRCR1_DIE2       }, // HS_SRCR1_DIE1
    { HSPR_SRCR, 5,                     7, A_Scream,                       HS_SRCR1_DIE3       }, // HS_SRCR1_DIE2
    { HSPR_SRCR, 6,                     7, NULL,                           HS_SRCR1_DIE4       }, // HS_SRCR1_DIE3
    { HSPR_SRCR, 7,                     6, NULL,                           HS_SRCR1_DIE5       }, // HS_SRCR1_DIE4
    { HSPR_SRCR, 8,                     6, NULL,                           HS_SRCR1_DIE6       }, // HS_SRCR1_DIE5
    { HSPR_SRCR, 9,                     6, NULL,                           HS_SRCR1_DIE7       }, // HS_SRCR1_DIE6
    { HSPR_SRCR, 10,                    6, NULL,                           HS_SRCR1_DIE8       }, // HS_SRCR1_DIE7
    { HSPR_SRCR, 11,                   25, NULL/*A_SorZap*/,               HS_SRCR1_DIE9       }, // HS_SRCR1_DIE8
    { HSPR_SRCR, 12,                    5, NULL,                           HS_SRCR1_DIE10      }, // HS_SRCR1_DIE9
    { HSPR_SRCR, 13,                    5, NULL,                           HS_SRCR1_DIE11      }, // HS_SRCR1_DIE10
    { HSPR_SRCR, 14,                    4, NULL,                           HS_SRCR1_DIE12      }, // HS_SRCR1_DIE11
    { HSPR_SRCR, 11,                   20, NULL/*A_SorZap*/,               HS_SRCR1_DIE13      }, // HS_SRCR1_DIE12
    { HSPR_SRCR, 12,                    5, NULL,                           HS_SRCR1_DIE14      }, // HS_SRCR1_DIE13
    { HSPR_SRCR, 13,                    5, NULL,                           HS_SRCR1_DIE15      }, // HS_SRCR1_DIE14
    { HSPR_SRCR, 14,                    4, NULL,                           HS_SRCR1_DIE16      }, // HS_SRCR1_DIE15
    { HSPR_SRCR, 11,                   12, NULL,                           HS_SRCR1_DIE17      }, // HS_SRCR1_DIE16
    { HSPR_SRCR, 15,                   -1, NULL/*A_SorcererRise*/,         HS_NULL             }, // HS_SRCR1_DIE17
    { HSPR_FX14, 0 | FF_FULLBRIGHT,     6, NULL,                           HS_SRCRFX1_2        }, // HS_SRCRFX1_1
    { HSPR_FX14, 1 | FF_FULLBRIGHT,     6, NULL,                           HS_SRCRFX1_3        }, // HS_SRCRFX1_2
    { HSPR_FX14, 2 | FF_FULLBRIGHT,     6, NULL,                           HS_SRCRFX1_1        }, // HS_SRCRFX1_3
    { HSPR_FX14, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_SRCRFXI1_2       }, // HS_SRCRFXI1_1
    { HSPR_FX14, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_SRCRFXI1_3       }, // HS_SRCRFXI1_2
    { HSPR_FX14, 5 | FF_FULLBRIGHT,     5, NULL,                           HS_SRCRFXI1_4       }, // HS_SRCRFXI1_3
    { HSPR_FX14, 6 | FF_FULLBRIGHT,     5, NULL,                           HS_SRCRFXI1_5       }, // HS_SRCRFXI1_4
    { HSPR_FX14, 7 | FF_FULLBRIGHT,     5, NULL,                           HS_NULL             }, // HS_SRCRFXI1_5
    { HSPR_SOR2, 0,                     4, NULL,                           HS_SOR2_RISE2       }, // HS_SOR2_RISE1
    { HSPR_SOR2, 1,                     4, NULL,                           HS_SOR2_RISE3       }, // HS_SOR2_RISE2
    { HSPR_SOR2, 2,                     4, NULL/*A_SorRise*/,              HS_SOR2_RISE4       }, // HS_SOR2_RISE3
    { HSPR_SOR2, 3,                     4, NULL,                           HS_SOR2_RISE5       }, // HS_SOR2_RISE4
    { HSPR_SOR2, 4,                     4, NULL,                           HS_SOR2_RISE6       }, // HS_SOR2_RISE5
    { HSPR_SOR2, 5,                     4, NULL,                           HS_SOR2_RISE7       }, // HS_SOR2_RISE6
    { HSPR_SOR2, 6,                    12, NULL/*A_SorSightSnd*/,          HS_SOR2_WALK1       }, // HS_SOR2_RISE7
    { HSPR_SOR2, 12,                   10, A_Look,                         HS_SOR2_LOOK2       }, // HS_SOR2_LOOK1
    { HSPR_SOR2, 13,                   10, A_Look,                         HS_SOR2_LOOK1       }, // HS_SOR2_LOOK2
    { HSPR_SOR2, 12,                    4, A_Chase,                        HS_SOR2_WALK2       }, // HS_SOR2_WALK1
    { HSPR_SOR2, 13,                    4, A_Chase,                        HS_SOR2_WALK3       }, // HS_SOR2_WALK2
    { HSPR_SOR2, 14,                    4, A_Chase,                        HS_SOR2_WALK4       }, // HS_SOR2_WALK3
    { HSPR_SOR2, 15,                    4, A_Chase,                        HS_SOR2_WALK1       }, // HS_SOR2_WALK4
    { HSPR_SOR2, 16,                    3, NULL,                           HS_SOR2_PAIN2       }, // HS_SOR2_PAIN1
    { HSPR_SOR2, 16,                    6, A_Pain,                         HS_SOR2_WALK1       }, // HS_SOR2_PAIN2
    { HSPR_SOR2, 17,                    9, NULL/*A_Srcr2Decide*/,          HS_SOR2_ATK2        }, // HS_SOR2_ATK1
    { HSPR_SOR2, 18,                    9, A_FaceTarget,                   HS_SOR2_ATK3        }, // HS_SOR2_ATK2
    { HSPR_SOR2, 19,                   20, NULL/*A_Srcr2Attack*/,          HS_SOR2_WALK1       }, // HS_SOR2_ATK3
    { HSPR_SOR2, 11,                    6, NULL,                           HS_SOR2_TELE2       }, // HS_SOR2_TELE1
    { HSPR_SOR2, 10,                    6, NULL,                           HS_SOR2_TELE3       }, // HS_SOR2_TELE2
    { HSPR_SOR2, 9,                     6, NULL,                           HS_SOR2_TELE4       }, // HS_SOR2_TELE3
    { HSPR_SOR2, 8,                     6, NULL,                           HS_SOR2_TELE5       }, // HS_SOR2_TELE4
    { HSPR_SOR2, 7,                     6, NULL,                           HS_SOR2_TELE6       }, // HS_SOR2_TELE5
    { HSPR_SOR2, 6,                     6, NULL,                           HS_SOR2_WALK1       }, // HS_SOR2_TELE6
    { HSPR_SDTH, 0,                     8, NULL/*A_Sor2DthInit*/,          HS_SOR2_DIE2        }, // HS_SOR2_DIE1
    { HSPR_SDTH, 1,                     8, NULL,                           HS_SOR2_DIE3        }, // HS_SOR2_DIE2
    { HSPR_SDTH, 2,                     8, NULL/*A_SorDSph*/,              HS_SOR2_DIE4        }, // HS_SOR2_DIE3
    { HSPR_SDTH, 3,                     7, NULL,                           HS_SOR2_DIE5        }, // HS_SOR2_DIE4
    { HSPR_SDTH, 4,                     7, NULL,                           HS_SOR2_DIE6        }, // HS_SOR2_DIE5
    { HSPR_SDTH, 5,                     7, NULL/*A_Sor2DthLoop*/,          HS_SOR2_DIE7        }, // HS_SOR2_DIE6
    { HSPR_SDTH, 6,                     6, NULL/*A_SorDExp*/,              HS_SOR2_DIE8        }, // HS_SOR2_DIE7
    { HSPR_SDTH, 7,                     6, NULL,                           HS_SOR2_DIE9        }, // HS_SOR2_DIE8
    { HSPR_SDTH, 8,                    18, NULL,                           HS_SOR2_DIE10       }, // HS_SOR2_DIE9
    { HSPR_SDTH, 9,                     6, NULL/*A_NoBlocking*/,           HS_SOR2_DIE11       }, // HS_SOR2_DIE10
    { HSPR_SDTH, 10,                    6, NULL/*A_SorDBon*/,              HS_SOR2_DIE12       }, // HS_SOR2_DIE11
    { HSPR_SDTH, 11,                    6, NULL,                           HS_SOR2_DIE13       }, // HS_SOR2_DIE12
    { HSPR_SDTH, 12,                    6, NULL,                           HS_SOR2_DIE14       }, // HS_SOR2_DIE13
    { HSPR_SDTH, 13,                    6, NULL,                           HS_SOR2_DIE15       }, // HS_SOR2_DIE14
    { HSPR_SDTH, 14,                   -1, NULL/*A_BossDeath*/,            HS_NULL             }, // HS_SOR2_DIE15
    { HSPR_FX16, 0 | FF_FULLBRIGHT,     3, NULL/*A_BlueSpark*/,            HS_SOR2FX1_2        }, // HS_SOR2FX1_1
    { HSPR_FX16, 1 | FF_FULLBRIGHT,     3, NULL/*A_BlueSpark*/,            HS_SOR2FX1_3        }, // HS_SOR2FX1_2
    { HSPR_FX16, 2 | FF_FULLBRIGHT,     3, NULL/*A_BlueSpark*/,            HS_SOR2FX1_1        }, // HS_SOR2FX1_3
    { HSPR_FX16, 6 | FF_FULLBRIGHT,     5, A_Explode,                      HS_SOR2FXI1_2       }, // HS_SOR2FXI1_1
    { HSPR_FX16, 7 | FF_FULLBRIGHT,     5, NULL,                           HS_SOR2FXI1_3       }, // HS_SOR2FXI1_2
    { HSPR_FX16, 8 | FF_FULLBRIGHT,     5, NULL,                           HS_SOR2FXI1_4       }, // HS_SOR2FXI1_3
    { HSPR_FX16, 9 | FF_FULLBRIGHT,     5, NULL,                           HS_SOR2FXI1_5       }, // HS_SOR2FXI1_4
    { HSPR_FX16, 10 | FF_FULLBRIGHT,    5, NULL,                           HS_SOR2FXI1_6       }, // HS_SOR2FXI1_5
    { HSPR_FX16, 11 | FF_FULLBRIGHT,    5, NULL,                           HS_NULL             }, // HS_SOR2FXI1_6
    { HSPR_FX16, 3 | FF_FULLBRIGHT,    12, NULL,                           HS_SOR2FXSPARK2     }, // HS_SOR2FXSPARK1
    { HSPR_FX16, 4 | FF_FULLBRIGHT,    12, NULL,                           HS_SOR2FXSPARK3     }, // HS_SOR2FXSPARK2
    { HSPR_FX16, 5 | FF_FULLBRIGHT,    12, NULL,                           HS_NULL             }, // HS_SOR2FXSPARK3
    { HSPR_FX11, 0 | FF_FULLBRIGHT,    35, NULL,                           HS_SOR2FX2_2        }, // HS_SOR2FX2_1
    { HSPR_FX11, 0 | FF_FULLBRIGHT,     5, NULL/*A_GenWizard*/,            HS_SOR2FX2_3        }, // HS_SOR2FX2_2
    { HSPR_FX11, 1 | FF_FULLBRIGHT,     5, NULL,                           HS_SOR2FX2_2        }, // HS_SOR2FX2_3
    { HSPR_FX11, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_SOR2FXI2_2       }, // HS_SOR2FXI2_1
    { HSPR_FX11, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_SOR2FXI2_3       }, // HS_SOR2FXI2_2
    { HSPR_FX11, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_SOR2FXI2_4       }, // HS_SOR2FXI2_3
    { HSPR_FX11, 5 | FF_FULLBRIGHT,     5, NULL,                           HS_SOR2FXI2_5       }, // HS_SOR2FXI2_4
    { HSPR_FX11, 6 | FF_FULLBRIGHT,     5, NULL,                           HS_NULL             }, // HS_SOR2FXI2_5
    { HSPR_SOR2, 6,                     8, NULL,                           HS_SOR2TELEFADE2    }, // HS_SOR2TELEFADE1
    { HSPR_SOR2, 7,                     6, NULL,                           HS_SOR2TELEFADE3    }, // HS_SOR2TELEFADE2
    { HSPR_SOR2, 8,                     6, NULL,                           HS_SOR2TELEFADE4    }, // HS_SOR2TELEFADE3
    { HSPR_SOR2, 9,                     6, NULL,                           HS_SOR2TELEFADE5    }, // HS_SOR2TELEFADE4
    { HSPR_SOR2, 10,                    6, NULL,                           HS_SOR2TELEFADE6    }, // HS_SOR2TELEFADE5
    { HSPR_SOR2, 11,                    6, NULL,                           HS_NULL             }, // HS_SOR2TELEFADE6
    { HSPR_MNTR, 0,                    10, A_Look,                         HS_MNTR_LOOK2       }, // HS_MNTR_LOOK1
    { HSPR_MNTR, 1,                    10, A_Look,                         HS_MNTR_LOOK1       }, // HS_MNTR_LOOK2
    { HSPR_MNTR, 0,                     5, A_Chase,                        HS_MNTR_WALK2       }, // HS_MNTR_WALK1
    { HSPR_MNTR, 1,                     5, A_Chase,                        HS_MNTR_WALK3       }, // HS_MNTR_WALK2
    { HSPR_MNTR, 2,                     5, A_Chase,                        HS_MNTR_WALK4       }, // HS_MNTR_WALK3
    { HSPR_MNTR, 3,                     5, A_Chase,                        HS_MNTR_WALK1       }, // HS_MNTR_WALK4
    { HSPR_MNTR, 21,                   10, A_FaceTarget,                   HS_MNTR_ATK1_2      }, // HS_MNTR_ATK1_1
    { HSPR_MNTR, 22,                    7, A_FaceTarget,                   HS_MNTR_ATK1_3      }, // HS_MNTR_ATK1_2
    { HSPR_MNTR, 23,                   12, NULL/*A_MinotaurAtk1*/,         HS_MNTR_WALK1       }, // HS_MNTR_ATK1_3
    { HSPR_MNTR, 21,                   10, NULL/*A_MinotaurDecide*/,       HS_MNTR_ATK2_2      }, // HS_MNTR_ATK2_1
    { HSPR_MNTR, 24,                    4, A_FaceTarget,                   HS_MNTR_ATK2_3      }, // HS_MNTR_ATK2_2
    { HSPR_MNTR, 25,                    9, NULL/*A_MinotaurAtk2*/,         HS_MNTR_WALK1       }, // HS_MNTR_ATK2_3
    { HSPR_MNTR, 21,                   10, A_FaceTarget,                   HS_MNTR_ATK3_2      }, // HS_MNTR_ATK3_1
    { HSPR_MNTR, 22,                    7, A_FaceTarget,                   HS_MNTR_ATK3_3      }, // HS_MNTR_ATK3_2
    { HSPR_MNTR, 23,                   12, NULL/*A_MinotaurAtk3*/,         HS_MNTR_WALK1       }, // HS_MNTR_ATK3_3
    { HSPR_MNTR, 23,                   12, NULL,                           HS_MNTR_ATK3_1      }, // HS_MNTR_ATK3_4
    { HSPR_MNTR, 20,                    2, NULL/*A_MinotaurCharge*/,       HS_MNTR_ATK4_1      }, // HS_MNTR_ATK4_1
    { HSPR_MNTR, 4,                     3, NULL,                           HS_MNTR_PAIN2       }, // HS_MNTR_PAIN1
    { HSPR_MNTR, 4,                     6, A_Pain,                         HS_MNTR_WALK1       }, // HS_MNTR_PAIN2
    { HSPR_MNTR, 5,                     6, NULL,                           HS_MNTR_DIE2        }, // HS_MNTR_DIE1
    { HSPR_MNTR, 6,                     5, NULL,                           HS_MNTR_DIE3        }, // HS_MNTR_DIE2
    { HSPR_MNTR, 7,                     6, A_Scream,                       HS_MNTR_DIE4        }, // HS_MNTR_DIE3
    { HSPR_MNTR, 8,                     5, NULL,                           HS_MNTR_DIE5        }, // HS_MNTR_DIE4
    { HSPR_MNTR, 9,                     6, NULL,                           HS_MNTR_DIE6        }, // HS_MNTR_DIE5
    { HSPR_MNTR, 10,                    5, NULL,                           HS_MNTR_DIE7        }, // HS_MNTR_DIE6
    { HSPR_MNTR, 11,                    6, NULL,                           HS_MNTR_DIE8        }, // HS_MNTR_DIE7
    { HSPR_MNTR, 12,                    5, NULL/*A_NoBlocking*/,           HS_MNTR_DIE9        }, // HS_MNTR_DIE8
    { HSPR_MNTR, 13,                    6, NULL,                           HS_MNTR_DIE10       }, // HS_MNTR_DIE9
    { HSPR_MNTR, 14,                    5, NULL,                           HS_MNTR_DIE11       }, // HS_MNTR_DIE10
    { HSPR_MNTR, 15,                    6, NULL,                           HS_MNTR_DIE12       }, // HS_MNTR_DIE11
    { HSPR_MNTR, 16,                    5, NULL,                           HS_MNTR_DIE13       }, // HS_MNTR_DIE12
    { HSPR_MNTR, 17,                    6, NULL,                           HS_MNTR_DIE14       }, // HS_MNTR_DIE13
    { HSPR_MNTR, 18,                    5, NULL,                           HS_MNTR_DIE15       }, // HS_MNTR_DIE14
    { HSPR_MNTR, 19,                   -1, NULL/*A_BossDeath*/,            HS_NULL             }, // HS_MNTR_DIE15
    { HSPR_FX12, 0 | FF_FULLBRIGHT,     6, NULL,                           HS_MNTRFX1_2        }, // HS_MNTRFX1_1
    { HSPR_FX12, 1 | FF_FULLBRIGHT,     6, NULL,                           HS_MNTRFX1_1        }, // HS_MNTRFX1_2
    { HSPR_FX12, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFXI1_2       }, // HS_MNTRFXI1_1
    { HSPR_FX12, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFXI1_3       }, // HS_MNTRFXI1_2
    { HSPR_FX12, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFXI1_4       }, // HS_MNTRFXI1_3
    { HSPR_FX12, 5 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFXI1_5       }, // HS_MNTRFXI1_4
    { HSPR_FX12, 6 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFXI1_6       }, // HS_MNTRFXI1_5
    { HSPR_FX12, 7 | FF_FULLBRIGHT,     5, NULL,                           HS_NULL             }, // HS_MNTRFXI1_6
    { HSPR_FX13, 0,                     2, NULL/*A_MntrFloorFire*/,        HS_MNTRFX2_1        }, // HS_MNTRFX2_1
    { HSPR_FX13, 8 | FF_FULLBRIGHT,     4, A_Explode,                      HS_MNTRFXI2_2       }, // HS_MNTRFXI2_1
    { HSPR_FX13, 9 | FF_FULLBRIGHT,     4, NULL,                           HS_MNTRFXI2_3       }, // HS_MNTRFXI2_2
    { HSPR_FX13, 10 | FF_FULLBRIGHT,    4, NULL,                           HS_MNTRFXI2_4       }, // HS_MNTRFXI2_3
    { HSPR_FX13, 11 | FF_FULLBRIGHT,    4, NULL,                           HS_MNTRFXI2_5       }, // HS_MNTRFXI2_4
    { HSPR_FX13, 12 | FF_FULLBRIGHT,    4, NULL,                           HS_NULL             }, // HS_MNTRFXI2_5
    { HSPR_FX13, 3 | FF_FULLBRIGHT,     4, NULL,                           HS_MNTRFX3_2        }, // HS_MNTRFX3_1
    { HSPR_FX13, 2 | FF_FULLBRIGHT,     4, NULL,                           HS_MNTRFX3_3        }, // HS_MNTRFX3_2
    { HSPR_FX13, 1 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFX3_4        }, // HS_MNTRFX3_3
    { HSPR_FX13, 2 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFX3_5        }, // HS_MNTRFX3_4
    { HSPR_FX13, 3 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFX3_6        }, // HS_MNTRFX3_5
    { HSPR_FX13, 4 | FF_FULLBRIGHT,     5, NULL,                           HS_MNTRFX3_7        }, // HS_MNTRFX3_6
    { HSPR_FX13, 5 | FF_FULLBRIGHT,     4, NULL,                           HS_MNTRFX3_8        }, // HS_MNTRFX3_7
    { HSPR_FX13, 6 | FF_FULLBRIGHT,     4, NULL,                           HS_MNTRFX3_9        }, // HS_MNTRFX3_8
    { HSPR_FX13, 7 | FF_FULLBRIGHT,     4, NULL,                           HS_NULL             }, // HS_MNTRFX3_9
    { HSPR_AKYY, 0 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY2            }, // HS_AKYY1
    { HSPR_AKYY, 1 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY3            }, // HS_AKYY2
    { HSPR_AKYY, 2 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY4            }, // HS_AKYY3
    { HSPR_AKYY, 3 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY5            }, // HS_AKYY4
    { HSPR_AKYY, 4 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY6            }, // HS_AKYY5
    { HSPR_AKYY, 5 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY7            }, // HS_AKYY6
    { HSPR_AKYY, 6 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY8            }, // HS_AKYY7
    { HSPR_AKYY, 7 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY9            }, // HS_AKYY8
    { HSPR_AKYY, 8 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY10           }, // HS_AKYY9
    { HSPR_AKYY, 9 | FF_FULLBRIGHT,     3, NULL,                           HS_AKYY1            }, // HS_AKYY10
    { HSPR_BKYY, 0 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY2            }, // HS_BKYY1
    { HSPR_BKYY, 1 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY3            }, // HS_BKYY2
    { HSPR_BKYY, 2 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY4            }, // HS_BKYY3
    { HSPR_BKYY, 3 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY5            }, // HS_BKYY4
    { HSPR_BKYY, 4 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY6            }, // HS_BKYY5
    { HSPR_BKYY, 5 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY7            }, // HS_BKYY6
    { HSPR_BKYY, 6 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY8            }, // HS_BKYY7
    { HSPR_BKYY, 7 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY9            }, // HS_BKYY8
    { HSPR_BKYY, 8 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY10           }, // HS_BKYY9
    { HSPR_BKYY, 9 | FF_FULLBRIGHT,     3, NULL,                           HS_BKYY1            }, // HS_BKYY10
    { HSPR_CKYY, 0 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY2            }, // HS_CKYY1
    { HSPR_CKYY, 1 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY3            }, // HS_CKYY2
    { HSPR_CKYY, 2 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY4            }, // HS_CKYY3
    { HSPR_CKYY, 3 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY5            }, // HS_CKYY4
    { HSPR_CKYY, 4 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY6            }, // HS_CKYY5
    { HSPR_CKYY, 5 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY7            }, // HS_CKYY6
    { HSPR_CKYY, 6 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY8            }, // HS_CKYY7
    { HSPR_CKYY, 7 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY9            }, // HS_CKYY8
    { HSPR_CKYY, 8 | FF_FULLBRIGHT,     3, NULL,                           HS_CKYY1            }, // HS_CKYY9
    { HSPR_AMG1, 0,                    -1, NULL,                           HS_NULL             }, // HS_AMG1
    { HSPR_AMG2, 0,                     4, NULL,                           HS_AMG2_2           }, // HS_AMG2_1
    { HSPR_AMG2, 1,                     4, NULL,                           HS_AMG2_3           }, // HS_AMG2_2
    { HSPR_AMG2, 2,                     4, NULL,                           HS_AMG2_1           }, // HS_AMG2_3
    { HSPR_AMM1, 0,                    -1, NULL,                           HS_NULL             }, // HS_AMM1
    { HSPR_AMM2, 0,                    -1, NULL,                           HS_NULL             }, // HS_AMM2
    { HSPR_AMC1, 0,                    -1, NULL,                           HS_NULL             }, // HS_AMC1
    { HSPR_AMC2, 0,                     5, NULL,                           HS_AMC2_2           }, // HS_AMC2_1
    { HSPR_AMC2, 1,                     5, NULL,                           HS_AMC2_3           }, // HS_AMC2_2
    { HSPR_AMC2, 2,                     5, NULL,                           HS_AMC2_1           }, // HS_AMC2_3
    { HSPR_AMS1, 0,                     5, NULL,                           HS_AMS1_2           }, // HS_AMS1_1
    { HSPR_AMS1, 1,                     5, NULL,                           HS_AMS1_1           }, // HS_AMS1_2
    { HSPR_AMS2, 0,                     5, NULL,                           HS_AMS2_2           }, // HS_AMS2_1
    { HSPR_AMS2, 1,                     5, NULL,                           HS_AMS2_1           }, // HS_AMS2_2
    { HSPR_AMP1, 0,                     4, NULL,                           HS_AMP1_2           }, // HS_AMP1_1
    { HSPR_AMP1, 1,                     4, NULL,                           HS_AMP1_3           }, // HS_AMP1_2
    { HSPR_AMP1, 2,                     4, NULL,                           HS_AMP1_1           }, // HS_AMP1_3
    { HSPR_AMP2, 0,                     4, NULL,                           HS_AMP2_2           }, // HS_AMP2_1
    { HSPR_AMP2, 1,                     4, NULL,                           HS_AMP2_3           }, // HS_AMP2_2
    { HSPR_AMP2, 2,                     4, NULL,                           HS_AMP2_1           }, // HS_AMP2_3
    { HSPR_AMB1, 0,                     4, NULL,                           HS_AMB1_2           }, // HS_AMB1_1
    { HSPR_AMB1, 1,                     4, NULL,                           HS_AMB1_3           }, // HS_AMB1_2
    { HSPR_AMB1, 2,                     4, NULL,                           HS_AMB1_1           }, // HS_AMB1_3
    { HSPR_AMB2, 0,                     4, NULL,                           HS_AMB2_2           }, // HS_AMB2_1
    { HSPR_AMB2, 1,                     4, NULL,                           HS_AMB2_3           }, // HS_AMB2_2
    { HSPR_AMB2, 2,                     4, NULL,                           HS_AMB2_1           }, // HS_AMB2_3
    { HSPR_AMG1, 0,                   100, NULL/*A_ESound*/,               HS_SND_WIND         }, // HS_SND_WIND
    { HSPR_AMG1, 0,                    85, NULL/*A_ESound*/,               HS_SND_WATERFALL    }  // HS_SND_WATERFALL
};


mobjinfo_t hereticmobjinfo[] = {

    {                           // MT_MISC0
     81,                        // doomednum
     HS_ITEM_PTN1_1,             // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_ITEMSHIELD1
     85,                        // doomednum
     HS_ITEM_SHLD1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                      },

    {                           // MT_ITEMSHIELD2
     31,                        // doomednum
     HS_ITEM_SHD2_1,             // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_MISC1
     8,                         // doomednum
     HS_ITEM_BAGH1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_MISC2
     35,                        // doomednum
     HS_ITEM_SPMP1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_ARTIINVISIBILITY
     75,                        // doomednum
     HS_ARTI_INVS1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | /*MF_SHADOW | */MF_COUNTITEM,     // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_MISC3
     82,                        // doomednum
     HS_ARTI_PTN2_1,             // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_ARTIFLY
     83,                        // doomednum
     HS_ARTI_SOAR1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_ARTIINVULNERABILITY
     84,                        // doomednum
     HS_ARTI_INVU1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_ARTITOMEOFPOWER
     86,                        // doomednum
     HS_ARTI_PWBK1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_ARTIEGG
     30,                        // doomednum
     HS_ARTI_EGGC1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_EGGFX
     -1,                        // doomednum
     HS_EGGFX1,                  // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_EGGFXI1_1,               // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     18 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_ARTISUPERHEAL
     32,                        // doomednum
     HS_ARTI_SPHL1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC4
     33,                        // doomednum
     HS_ARTI_TRCH1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_MISC5
     34,                        // doomednum
     HS_ARTI_FBMB1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_FIREBOMB
     -1,                        // doomednum
     HS_FIREBOMB1,               // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_phohit,                // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOGRAVITY/* | MF_SHADOW*/,  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_ARTITELEPORT
     36,                        // doomednum
     HS_ARTI_ATLP1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_COUNTITEM, // flags
     MF2_FLOATBOB,              // flags2
     0                          // flags3
                                     },

    {                           // MT_POD
     2035,                      // doomednum
     HS_POD_WAIT1,               // spawnstate
     45,                        // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_POD_PAIN1,               // painstate
     255,                       // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_POD_DIE1,                // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_podexp,                // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     54 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID | MF_NOBLOOD | MF_SHOOTABLE | MF_DROPOFF, // flags
     MF2_PASSMOBJ,              // flags2
     MF3_WINDTHRUST | MF3_PUSHABLE | MF3_SLIDE | MF3_TELESTOMP   // flags3
                                      },

    {                           // MT_PODGOO
     -1,                        // doomednum
     HS_PODGOO1,                 // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_PODGOOX,                 // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     2 * FRACUNIT,             // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_LOGRAV | MF3_CANNOTPUSH       // flags3
                                      },

    {                           // MT_PODGENERATOR
     43,                        // doomednum
     HS_PODGENERATOR,            // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOSECTOR,       // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SPLASH
     -1,                        // doomednum
     HS_SPLASH1,                 // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_SPLASHX,                 // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     2 * FRACUNIT,             // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_LOGRAV | MF3_CANNOTPUSH       // flags3
                                      },

    {                           // MT_SPLASHBASE
     -1,                        // doomednum
     HS_SPLASHBASE1,             // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_LAVASPLASH
     -1,                        // doomednum
     HS_LAVASPLASH1,             // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_LAVASMOKE
     -1,                        // doomednum
     HS_LAVASMOKE1,              // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SLUDGECHUNK
     -1,                        // doomednum
     HS_SLUDGECHUNK1,            // spawnstate
     1000,                      // spawnhealth
     0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_SLUDGECHUNKX,            // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     2 * FRACUNIT,             // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_LOGRAV | MF3_CANNOTPUSH       // flags3
                                      },

    {                           // MT_SLUDGESPLASH
     -1,                        // doomednum
     HS_SLUDGESPLASH1,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SKULLHANG70
     17,                        // doomednum
     HS_SKULLHANG70_1,           // spawnstate
     1000,                      // spawnhealth
0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     70 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SKULLHANG60
     24,                        // doomednum
     HS_SKULLHANG60_1,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     60 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SKULLHANG45
     25,                        // doomednum
     HS_SKULLHANG45_1,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     45 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SKULLHANG35
     26,                        // doomednum
     HS_SKULLHANG35_1,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     35 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_CHANDELIER
     28,                        // doomednum
     HS_CHANDELIER1,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     60 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SERPTORCH
     27,                        // doomednum
     HS_SERPTORCH1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     54 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SMALLPILLAR
     29,                        // doomednum
     HS_SMALLPILLAR,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     34 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_STALAGMITESMALL
     37,                        // doomednum
     HS_STALAGMITESMALL,         // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     32 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_STALAGMITELARGE
     38,                        // doomednum
     HS_STALAGMITELARGE,         // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     64 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_STALACTITESMALL
     39,                        // doomednum
     HS_STALACTITESMALL,         // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     36 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_STALACTITELARGE
     40,                        // doomednum
     HS_STALACTITELARGE,         // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     68 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC6
     76,                        // doomednum
     HS_FIREBRAZIER1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     44 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_BARREL
     44,                        // doomednum
     HS_BARREL,                  // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     32 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC7
     47,                        // doomednum
     HS_BRPILLAR,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     14 * FRACUNIT,             // radius
     14 * FRACUNIT,             // radius
     128 * FRACUNIT,            // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC8
     48,                        // doomednum
     HS_MOSS1,                   // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     23 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC9
     49,                        // doomednum
     HS_MOSS2,                   // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     27 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPAWNCEILING | MF_NOGRAVITY,    // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC10
     50,                        // doomednum
     HS_WALLTORCH1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOGRAVITY,              // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC11
     51,                        // doomednum
     HS_HANGINGCORPSE,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     104 * FRACUNIT,            // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY, // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_KEYGIZMOBLUE
     94,                        // doomednum
     HS_KEYGIZMO1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     50 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_KEYGIZMOGREEN
     95,                        // doomednum
     HS_KEYGIZMO1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     50 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_KEYGIZMOYELLOW
     96,                        // doomednum
     HS_KEYGIZMO1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     50 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_KEYGIZMOFLOAT
     -1,                        // doomednum
     HS_KGZ_START,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID | MF_NOGRAVITY,   // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC12
     87,                        // doomednum
     HS_VOLCANO1,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     20 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID,                  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_VOLCANOBLAST
     -1,                        // doomednum
     HS_VOLCANOBALL1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_VOLCANOBALLX1,           // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_volhit,                // deathsound
     2 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_LOGRAV | MF3_NOTELEPORT | MF3_FIREDAMAGE       // flags3
                                      },

    {                           // MT_VOLCANOTBLAST
     -1,                        // doomednum
     HS_VOLCANOTBALL1,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_VOLCANOTBALLX1,          // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     2 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_LOGRAV | MF3_NOTELEPORT | MF3_FIREDAMAGE       // flags3
                                      },

    {                           // MT_TELEGLITGEN
     74,                        // doomednum
     HS_TELEGLITGEN1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_NOSECTOR,        // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_TELEGLITGEN2
     52,                        // doomednum
     HS_TELEGLITGEN2,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_NOSECTOR,        // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_TELEGLITTER
     -1,                        // doomednum
     HS_TELEGLITTER1_1,          // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_TELEGLITTER2
     -1,                        // doomednum
     HS_TELEGLITTER2_1,          // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_TFOG
     -1,                        // doomednum
     HS_TFOG1,                   // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_TELEPORTMAN
     14,                        // doomednum
     HS_NULL,                    // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOSECTOR,       // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_STAFFPUFF
     -1,                        // doomednum
     HS_STAFFPUFF1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_stfhit,                // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_STAFFPUFF2
     -1,                        // doomednum
     HS_STAFFPUFF2_1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_stfpow,                // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_BEAKPUFF
     -1,                        // doomednum
     HS_STAFFPUFF1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_chicatk,               // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC13
     2005,                      // doomednum
     HS_WGNT,                    // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_GAUNTLETPUFF1
     -1,                        // doomednum
     HS_GAUNTLETPUFF1_1,         // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_GAUNTLETPUFF2
     -1,                        // doomednum
     HS_GAUNTLETPUFF2_1,         // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,  // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MISC14
     53,                        // doomednum
     HS_BLSR,                    // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_BLASTERFX1
     -1,                        // doomednum
     HS_BLASTERFX1_1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_BLASTERFXI1_1,           // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_blshit,                // deathsound
     184 * FRACUNIT,            // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_BLASTERSMOKE
     -1,                        // doomednum
     HS_BLASTERSMOKE1,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,  // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_CANNOTPUSH    // flags3
                                      },

    {                           // MT_RIPPER
     -1,                        // doomednum
     HS_RIPPER1,                 // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_RIPPERX1,                // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_hrnhit,                // deathsound
     14 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_RIP   // flags3
                                      },

    {                           // MT_BLASTERPUFF1
     -1,                        // doomednum
     HS_BLASTERPUFF1_1,          // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_BLASTERPUFF2
     -1,                        // doomednum
     HS_BLASTERPUFF2_1,          // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_WMACE
     2002,                      // doomednum
     HS_WMCE,                    // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MACEFX1
     -1,                        // doomednum
     HS_MACEFX1_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_lobsht,                // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_MACEFXI1_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     20 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_FLOORBOUNCE | MF3_THRUGHOST | MF3_NOTELEPORT   // flags3
                                      },

    {                           // MT_MACEFX2
     -1,                        // doomednum
     HS_MACEFX2_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_MACEFXI2_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     6,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_LOGRAV | MF3_FLOORBOUNCE | MF3_THRUGHOST | MF3_NOTELEPORT      // flags3
                                      },

    {                           // MT_MACEFX3
     -1,                        // doomednum
     HS_MACEFX3_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_MACEFXI1_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     7 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     4,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_LOGRAV | MF3_FLOORBOUNCE | MF3_THRUGHOST | MF3_NOTELEPORT      // flags3
                                      },

    {                           // MT_MACEFX4
     -1,                        // doomednum
     HS_MACEFX4_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_MACEFXI4_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     7 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     18,                        // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_LOGRAV | MF3_FLOORBOUNCE | MF3_THRUGHOST | MF3_TELESTOMP       // flags3
                                      },

    {                           // MT_WSKULLROD
     2004,                      // doomednum
     HS_WSKL,                    // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_HORNRODFX1
     -1,                        // doomednum
     HS_HRODFX1_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_hrnsht,                // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_HRODFXI1_1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_hrnhit,                // deathsound
     22 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_WINDTHRUST | MF3_NOTELEPORT    // flags3
                                      },

    {                           // MT_HORNRODFX2
     -1,                        // doomednum
     HS_HRODFX2_1,               // spawnstate
     4 * 35,                    // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_hrnsht,                // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_HRODFXI2_1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_ramphit,               // deathsound
     22 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     10,                        // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_RAINPLR1
     -1,                        // doomednum
     HS_RAINPLR1_1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_RAINPLR1X_1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_RAINPLR2
     -1,                        // doomednum
     HS_RAINPLR2_1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_RAINPLR2X_1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_RAINPLR3
     -1,                        // doomednum
     HS_RAINPLR3_1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_RAINPLR3X_1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_RAINPLR4
     -1,                        // doomednum
     HS_RAINPLR4_1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_RAINPLR4X_1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_GOLDWANDFX1
     -1,                        // doomednum
     HS_GWANDFX1_1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_GWANDFXI1_1,             // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_gldhit,                // deathsound
     22 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_GOLDWANDFX2
     -1,                        // doomednum
     HS_GWANDFX2_1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_GWANDFXI1_1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     18 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_GOLDWANDPUFF1
     -1,                        // doomednum
     HS_GWANDPUFF1_1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_GOLDWANDPUFF2
     -1,                        // doomednum
     HS_GWANDFXI1_1,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_WPHOENIXROD
     2003,                      // doomednum
     HS_WPHX,                    // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_PHOENIXFX1
     -1,                        // doomednum
     HS_PHOENIXFX1_1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_phosht,                // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_PHOENIXFXI1_1,           // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_phohit,                // deathsound
     20 * FRACUNIT,             // speed
     11 * FRACUNIT,             // radius
     11 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     20,                        // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_THRUGHOST | MF3_NOTELEPORT     // flags3
                                      },

    // The following thing is present in the mobjinfo table from Heretic 1.0,
    // but not in Heretic 1.3 (ie. it was removed).  It has been re-inserted
    // here to support HHE patches.

    {                           // MT_PHOENIXFX_REMOVED
     -1,                        // doomednum
     HS_PHOENIXFXIX_1,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_PHOENIXFXIX_3,           // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT     // flags3
                                      },

    {                           // MT_PHOENIXPUFF
     -1,                        // doomednum
     HS_PHOENIXPUFF1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,  // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_CANNOTPUSH    // flags3
                                      },

    {                           // MT_PHOENIXFX2
     -1,                        // doomednum
     HS_PHOENIXFX2_1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_PHOENIXFXI2_1,           // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     6 * FRACUNIT,              // radius
     6 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_FIREDAMAGE    // flags3
                                      },

    {                           // MT_MISC15
     2001,                      // doomednum
     HS_WBOW,                    // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_CRBOWFX1
     -1,                        // doomednum
     HS_CRBOWFX1,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_bowsht,                // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_CRBOWFXI1_1,             // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_hrnhit,                // deathsound
     30 * FRACUNIT,             // speed
     11 * FRACUNIT,             // radius
     11 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     10,                        // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_CRBOWFX2
     -1,                        // doomednum
     HS_CRBOWFX2,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_bowsht,                // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_CRBOWFXI1_1,             // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_hrnhit,                // deathsound
     32 * FRACUNIT,             // speed
     11 * FRACUNIT,             // radius
     11 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     6,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_CRBOWFX3
     -1,                        // doomednum
     HS_CRBOWFX3,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_CRBOWFXI3_1,             // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_hrnhit,                // deathsound
     20 * FRACUNIT,             // speed
     11 * FRACUNIT,             // radius
     11 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_WINDTHRUST | MF3_THRUGHOST | MF3_NOTELEPORT    // flags3
                                      },

    {                           // MT_CRBOWFX4
     -1,                        // doomednum
     HS_CRBOWFX4_1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0,                         // flags2
     MF3_LOGRAV                 // flags3
                                      },

    {                           // MT_BLOOD
     -1,                        // doomednum
     HS_BLOOD1,                  // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_BLOODSPLATTER
     -1,                        // doomednum
     HS_BLOODSPLATTER1,          // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_BLOODSPLATTERX,          // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_CANNOTPUSH    // flags3
                                      },

    {                           // MT_PLAYER
     -1,                        // doomednum
     HS_PLAY,                    // spawnstate
     100,                       // spawnhealth
    0,                      // gibhealth
     HS_PLAY_RUN1,               // seestate
     hsfx_None,                  // seesound
     0,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_PLAY_PAIN,               // painstate
     255,                       // painchance
     hsfx_plrpai,                // painsound
     HS_NULL,                    // meleestate
     HS_PLAY_ATK1,               // missilestate
     HS_NULL,                    // crashstate
     HS_PLAY_DIE1,               // deathstate
     HS_PLAY_XDIE1,              // xdeathstate
     hsfx_plrdth,                // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     56 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_PICKUP | MF_NOTDMATCH,   // flags
     MF2_PASSMOBJ, // flags2
     MF3_WINDTHRUST | MF3_SLIDE | MF3_TELESTOMP   // flags3
                                      },

    {                           // MT_BLOODYSKULL
     -1,                        // doomednum
     HS_BLOODYSKULL1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     4 * FRACUNIT,              // radius
     4 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_DROPOFF,        // flags
     0,                         // flags2
     MF3_LOGRAV | MF3_CANNOTPUSH        // flags3
                                      },

    {                           // MT_CHICPLAYER
     -1,                        // doomednum
     HS_CHICPLAY,                // spawnstate
     100,                       // spawnhealth
    0,                      // gibhealth
     HS_CHICPLAY_RUN1,           // seestate
     hsfx_None,                  // seesound
     0,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_CHICPLAY_PAIN,           // painstate
     255,                       // painchance
     hsfx_chicpai,               // painsound
     HS_NULL,                    // meleestate
     HS_CHICPLAY_ATK1,           // missilestate
     HS_NULL,                    // crashstate
     HS_CHICKEN_DIE1,            // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_chicdth,               // deathsound
     0,                         // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     24 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_NOTDMATCH,       // flags
     MF2_PASSMOBJ, // flags2
     MF3_WINDTHRUST | MF3_SLIDE | MF3_LOGRAV | MF3_TELESTOMP      // flags3
                                      },

    {                           // MT_CHICKEN
     -1,                        // doomednum
     HS_CHICKEN_LOOK1,           // spawnstate
     10,                        // spawnhealth
    0,                      // gibhealth
     HS_CHICKEN_WALK1,           // seestate
     hsfx_chicpai,               // seesound
     8,                         // reactiontime
     hsfx_chicatk,               // attacksound
     HS_CHICKEN_PAIN1,           // painstate
     200,                       // painchance
     hsfx_chicpai,               // painsound
     HS_CHICKEN_ATK1,            // meleestate
     0,                         // missilestate
     HS_NULL,                    // crashstate
     HS_CHICKEN_DIE1,            // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_chicdth,               // deathsound
     4,                         // speed
     9 * FRACUNIT,              // radius
     9 * FRACUNIT,              // radius
     22 * FRACUNIT,             // height
     40,                        // mass
     0,                         // damage
     hsfx_chicact,               // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,       // flags
     MF2_PASSMOBJ, // flags2
     MF3_WINDTHRUST             // flags3
                                      },

    {                           // MT_FEATHER
     -1,                        // doomednum
     HS_FEATHER1,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_FEATHERX,                // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     2 * FRACUNIT,              // radius
     2 * FRACUNIT,              // radius
     4 * FRACUNIT,              // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,   // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_LOGRAV | MF3_CANNOTPUSH | MF3_WINDTHRUST      // flags3
                                      },

    {                           // MT_MUMMY
     68,                        // doomednum
     HS_MUMMY_LOOK1,             // spawnstate
     80,                        // spawnhealth
    0,                      // gibhealth
     HS_MUMMY_WALK1,             // seestate
     hsfx_mumsit,                // seesound
     8,                         // reactiontime
     hsfx_mumat1,                // attacksound
     HS_MUMMY_PAIN1,             // painstate
     128,                       // painchance
     hsfx_mumpai,                // painsound
     HS_MUMMY_ATK1,              // meleestate
     0,                         // missilestate
     HS_NULL,                    // crashstate
     HS_MUMMY_DIE1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_mumdth,                // deathsound
     12,                        // speed
     22 * FRACUNIT,             // radius
     22 * FRACUNIT,             // radius
     62 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     hsfx_mumact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                      },

    {                           // MT_MUMMYLEADER
     45,                        // doomednum
     HS_MUMMY_LOOK1,             // spawnstate
     100,                       // spawnhealth
    0,                      // gibhealth
     HS_MUMMY_WALK1,             // seestate
     hsfx_mumsit,                // seesound
     8,                         // reactiontime
     hsfx_mumat1,                // attacksound
     HS_MUMMY_PAIN1,             // painstate
     64,                        // painchance
     hsfx_mumpai,                // painsound
     HS_MUMMY_ATK1,              // meleestate
     HS_MUMMYL_ATK1,             // missilestate
     HS_NULL,                    // crashstate
     HS_MUMMY_DIE1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_mumdth,                // deathsound
     12,                        // speed
     22 * FRACUNIT,             // radius
     22 * FRACUNIT,             // radius
     62 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     hsfx_mumact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                     },

    {                           // MT_MUMMYGHOST
     69,                        // doomednum
     HS_MUMMY_LOOK1,             // spawnstate
     80,                        // spawnhealth
    0,                      // gibhealth
     HS_MUMMY_WALK1,             // seestate
     hsfx_mumsit,                // seesound
     8,                         // reactiontime
     hsfx_mumat1,                // attacksound
     HS_MUMMY_PAIN1,             // painstate
     128,                       // painchance
     hsfx_mumpai,                // painsound
     HS_MUMMY_ATK1,              // meleestate
     0,                         // missilestate
     HS_NULL,                    // crashstate
     HS_MUMMY_DIE1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_mumdth,                // deathsound
     12,                        // speed
     22 * FRACUNIT,             // radius
     22 * FRACUNIT,             // radius
     62 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     hsfx_mumact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL/* | MF_SHADOW*/,        // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                     },

    {                           // MT_MUMMYLEADERGHOST
     46,                        // doomednum
     HS_MUMMY_LOOK1,             // spawnstate
     100,                       // spawnhealth
    0,                      // gibhealth
     HS_MUMMY_WALK1,             // seestate
     hsfx_mumsit,                // seesound
     8,                         // reactiontime
     hsfx_mumat1,                // attacksound
     HS_MUMMY_PAIN1,             // painstate
     64,                        // painchance
     hsfx_mumpai,                // painsound
     HS_MUMMY_ATK1,              // meleestate
     HS_MUMMYL_ATK1,             // missilestate
     HS_NULL,                    // crashstate
     HS_MUMMY_DIE1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_mumdth,                // deathsound
     12,                        // speed
     22 * FRACUNIT,             // radius
     22 * FRACUNIT,             // radius
     62 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     hsfx_mumact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL/* | MF_SHADOW*/,        // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                     },

    {                           // MT_MUMMYSOUL
     -1,                        // doomednum
     HS_MUMMY_SOUL1,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MUMMYFX1
     -1,                        // doomednum
     HS_MUMMYFX1_1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_MUMMYFXI1_1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     9 * FRACUNIT,              // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     14 * FRACUNIT,             // height
     100,                       // mass
     4,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_BEAST
     70,                        // doomednum
     HS_BEAST_LOOK1,             // spawnstate
     220,                       // spawnhealth
    0,                      // gibhealth
     HS_BEAST_WALK1,             // seestate
     hsfx_bstsit,                // seesound
     8,                         // reactiontime
     hsfx_bstatk,                // attacksound
     HS_BEAST_PAIN1,             // painstate
     100,                       // painchance
     hsfx_bstpai,                // painsound
     0,                         // meleestate
     HS_BEAST_ATK1,              // missilestate
     HS_NULL,                    // crashstate
     HS_BEAST_DIE1,              // deathstate
     HS_BEAST_XDIE1,             // xdeathstate
     hsfx_bstdth,                // deathsound
     14,                        // speed
     32 * FRACUNIT,             // radius
     32 * FRACUNIT,             // radius
     74 * FRACUNIT,             // height
     200,                       // mass
     0,                         // damage
     hsfx_bstact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                     },

    {                           // MT_BEASTBALL
     -1,                        // doomednum
     HS_BEASTBALL1,              // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_BEASTBALLX1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     12 * FRACUNIT,             // speed
     9 * FRACUNIT,              // radius
     9 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     4,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_WINDTHRUST | MF3_NOTELEPORT    // flags3
                                      },

    {                           // MT_BURNBALL
     -1,                        // doomednum
     HS_BURNBALL1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_BEASTBALLX1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     6 * FRACUNIT,              // radius
     6 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_BURNBALLFB
     -1,                        // doomednum
     HS_BURNBALLFB1,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_BEASTBALLX1,             // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     6 * FRACUNIT,              // radius
     6 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_PUFFY
     -1,                        // doomednum
     HS_PUFFY1,                  // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_PUFFY1,                  // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     6 * FRACUNIT,              // radius
     6 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE, // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_SNAKE
     92,                        // doomednum
     HS_SNAKE_LOOK1,             // spawnstate
     280,                       // spawnhealth
    0,                      // gibhealth
     HS_SNAKE_WALK1,             // seestate
     hsfx_snksit,                // seesound
     8,                         // reactiontime
     hsfx_snkatk,                // attacksound
     HS_SNAKE_PAIN1,             // painstate
     48,                        // painchance
     hsfx_snkpai,                // painsound
     0,                         // meleestate
     HS_SNAKE_ATK1,              // missilestate
     HS_NULL,                    // crashstate
     HS_SNAKE_DIE1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_snkdth,                // deathsound
     10,                        // speed
     22 * FRACUNIT,             // radius
     22 * FRACUNIT,             // radius
     70 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_snkact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                     },

    {                           // MT_SNAKEPRO_A
     -1,                        // doomednum
     HS_SNAKEPRO_A1,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_SNAKEPRO_AX1,            // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     14 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_WINDTHRUST | MF3_NOTELEPORT    // flags3
                                      },

    {                           // MT_SNAKEPRO_B
     -1,                        // doomednum
     HS_SNAKEPRO_B1,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_SNAKEPRO_BX1,            // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     14 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_HEAD
     6,                         // doomednum
     HS_HEAD_LOOK,               // spawnstate
     700,                       // spawnhealth
    0,                      // gibhealth
     HS_HEAD_FLOAT,              // seestate
     hsfx_hedsit,                // seesound
     8,                         // reactiontime
     hsfx_hedat1,                // attacksound
     HS_HEAD_PAIN1,              // painstate
     32,                        // painchance
     hsfx_hedpai,                // painsound
     0,                         // meleestate
     HS_HEAD_ATK1,               // missilestate
     HS_NULL,                    // crashstate
     HS_HEAD_DIE1,               // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_heddth,                // deathsound
     6,                         // speed
     40 * FRACUNIT,             // radius
     40 * FRACUNIT,             // radius
     72 * FRACUNIT,             // height
     325,                       // mass
     0,                         // damage
     hsfx_hedact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_NOBLOOD,       // flags
     MF2_PASSMOBJ,              // flags2
     0                          // flags3
                                     },

    {                           // MT_HEADFX1
     -1,                        // doomednum
     HS_HEADFX1_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_HEADFXI1_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     13 * FRACUNIT,             // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_THRUGHOST     // flags3
                                      },

    {                           // MT_HEADFX2
     -1,                        // doomednum
     HS_HEADFX2_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_HEADFXI2_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     8 * FRACUNIT,              // speed
     12 * FRACUNIT,             // radius
     12 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_HEADFX3
     -1,                        // doomednum
     HS_HEADFX3_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_HEADFXI3_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     14 * FRACUNIT,             // radius
     14 * FRACUNIT,             // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     5,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_WINDTHRUST | MF3_NOTELEPORT    // flags3
                                      },

    {                           // MT_WHIRLWIND
     -1,                        // doomednum
     HS_HEADFX4_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_HEADFXI4_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     74 * FRACUNIT,             // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY/* | MF_SHADOW*/,        // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_CLINK
     90,                        // doomednum
     HS_CLINK_LOOK1,             // spawnstate
     150,                       // spawnhealth
    0,                      // gibhealth
     HS_CLINK_WALK1,             // seestate
     hsfx_clksit,                // seesound
     8,                         // reactiontime
     hsfx_clkatk,                // attacksound
     HS_CLINK_PAIN1,             // painstate
     32,                        // painchance
     hsfx_clkpai,                // painsound
     HS_CLINK_ATK1,              // meleestate
     0,                         // missilestate
     HS_NULL,                    // crashstate
     HS_CLINK_DIE1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_clkdth,                // deathsound
     14,                        // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     64 * FRACUNIT,             // height
     75,                        // mass
     0,                         // damage
     hsfx_clkact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_NOBLOOD,       // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                     },

    {                           // MT_WIZARD
     15,                        // doomednum
     HS_WIZARD_LOOK1,            // spawnstate
     180,                       // spawnhealth
    0,                      // gibhealth
     HS_WIZARD_WALK1,            // seestate
     hsfx_wizsit,                // seesound
     8,                         // reactiontime
     hsfx_wizatk,                // attacksound
     HS_WIZARD_PAIN1,            // painstate
     64,                        // painchance
     hsfx_wizpai,                // painsound
     0,                         // meleestate
     HS_WIZARD_ATK1,             // missilestate
     HS_NULL,                    // crashstate
     HS_WIZARD_DIE1,             // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_wizdth,                // deathsound
     12,                        // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     68 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_wizact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_FLOAT | MF_NOGRAVITY,  // flags
     MF2_PASSMOBJ,              // flags2
     0                          // flags3
                                     },

    {                           // MT_WIZFX1
     -1,                        // doomednum
     HS_WIZFX1_1,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_WIZFXI1_1,               // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     18 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_IMP
     66,                        // doomednum
     HS_IMP_LOOK1,               // spawnstate
     40,                        // spawnhealth
    0,                      // gibhealth
     HS_IMP_FLY1,                // seestate
     hsfx_impsit,                // seesound
     8,                         // reactiontime
     hsfx_impat1,                // attacksound
     HS_IMP_PAIN1,               // painstate
     200,                       // painchance
     hsfx_imppai,                // painsound
     HS_IMP_MEATK1,              // meleestate
     HS_IMP_MSATK1_1,            // missilestate
     HS_IMP_CRASH1,              // crashstate
     HS_IMP_DIE1,                // deathstate
     HS_IMP_XDIE1,               // xdeathstate
     hsfx_impdth,                // deathsound
     10,                        // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     36 * FRACUNIT,             // height
     50,                        // mass
     0,                         // damage
     hsfx_impact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL,  // flags
     MF2_PASSMOBJ,              // flags2
     MF3_SPAWNFLOAT             // flags3
                                      },

    {                           // MT_IMPLEADER
     5,                         // doomednum
     HS_IMP_LOOK1,               // spawnstate
     80,                        // spawnhealth
    0,                      // gibhealth
     HS_IMP_FLY1,                // seestate
     hsfx_impsit,                // seesound
     8,                         // reactiontime
     hsfx_impat2,                // attacksound
     HS_IMP_PAIN1,               // painstate
     200,                       // painchance
     hsfx_imppai,                // painsound
     0,                         // meleestate
     HS_IMP_MSATK2_1,            // missilestate
     HS_IMP_CRASH1,              // crashstate
     HS_IMP_DIE1,                // deathstate
     HS_IMP_XDIE1,               // xdeathstate
     hsfx_impdth,                // deathsound
     10,                        // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     36 * FRACUNIT,             // height
     50,                        // mass
     0,                         // damage
     hsfx_impact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL,  // flags
     MF2_PASSMOBJ,              // flags2
     MF3_SPAWNFLOAT             // flags3
                                      },

    {                           // MT_IMPCHUNK1
     -1,                        // doomednum
     HS_IMP_CHUNKA1,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_IMPCHUNK2
     -1,                        // doomednum
     HS_IMP_CHUNKB1,             // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_IMPBALL
     -1,                        // doomednum
     HS_IMPFX1,                  // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_IMPFXI1,                 // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     10 * FRACUNIT,             // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_WINDTHRUST | MF3_NOTELEPORT    // flags3
                                      },

    {                           // MT_KNIGHT
     64,                        // doomednum
     HS_KNIGHT_STND1,            // spawnstate
     200,                       // spawnhealth
    0,                      // gibhealth
     HS_KNIGHT_WALK1,            // seestate
     hsfx_kgtsit,                // seesound
     8,                         // reactiontime
     hsfx_kgtatk,                // attacksound
     HS_KNIGHT_PAIN1,            // painstate
     100,                       // painchance
     hsfx_kgtpai,                // painsound
     HS_KNIGHT_ATK1,             // meleestate
     HS_KNIGHT_ATK1,             // missilestate
     HS_NULL,                    // crashstate
     HS_KNIGHT_DIE1,             // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_kgtdth,                // deathsound
     12,                        // speed
     24 * FRACUNIT,             // radius
     24 * FRACUNIT,             // radius
     78 * FRACUNIT,             // height
     150,                       // mass
     0,                         // damage
     hsfx_kgtact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                      },

    {                           // MT_KNIGHTGHOST
     65,                        // doomednum
     HS_KNIGHT_STND1,            // spawnstate
     200,                       // spawnhealth
    0,                      // gibhealth
     HS_KNIGHT_WALK1,            // seestate
     hsfx_kgtsit,                // seesound
     8,                         // reactiontime
     hsfx_kgtatk,                // attacksound
     HS_KNIGHT_PAIN1,            // painstate
     100,                       // painchance
     hsfx_kgtpai,                // painsound
     HS_KNIGHT_ATK1,             // meleestate
     HS_KNIGHT_ATK1,             // missilestate
     HS_NULL,                    // crashstate
     HS_KNIGHT_DIE1,             // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_kgtdth,                // deathsound
     12,                        // speed
     24 * FRACUNIT,             // radius
     24 * FRACUNIT,             // radius
     78 * FRACUNIT,             // height
     150,                       // mass
     0,                         // damage
     hsfx_kgtact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL/* | MF_SHADOW*/,        // flags
     MF2_PASSMOBJ, // flags2
     0                          // flags3
                                     },

    {                           // MT_KNIGHTAXE
     -1,                        // doomednum
     HS_SPINAXE1,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_SPINAXEX1,               // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_hrnhit,                // deathsound
     9 * FRACUNIT,              // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     2,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_WINDTHRUST | MF3_NOTELEPORT | MF3_THRUGHOST    // flags3
                                      },

    {                           // MT_REDAXE
     -1,                        // doomednum
     HS_REDAXE1,                 // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_REDAXEX1,                // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_hrnhit,                // deathsound
     9 * FRACUNIT,              // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     8 * FRACUNIT,              // height
     100,                       // mass
     7,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_THRUGHOST     // flags3
                                      },

    {                           // MT_SORCERER1
     7,                         // doomednum
     HS_SRCR1_LOOK1,             // spawnstate
     2000,                      // spawnhealth
    0,                      // gibhealth
     HS_SRCR1_WALK1,             // seestate
     hsfx_sbtsit,                // seesound
     8,                         // reactiontime
     hsfx_sbtatk,                // attacksound
     HS_SRCR1_PAIN1,             // painstate
     56,                        // painchance
     hsfx_sbtpai,                // painsound
     0,                         // meleestate
     HS_SRCR1_ATK1,              // missilestate
     HS_NULL,                    // crashstate
     HS_SRCR1_DIE1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_sbtdth,                // deathsound
     16,                        // speed
     28 * FRACUNIT,             // radius
     28 * FRACUNIT,             // radius
     100 * FRACUNIT,            // height
     800,                       // mass
     0,                         // damage
     hsfx_sbtact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,    // flags
     MF2_PASSMOBJ, // flags2
     MF3_BOSS                   // flags3
                                      },

    {                           // MT_SRCRFX1
     -1,                        // doomednum
     HS_SRCRFX1_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_SRCRFXI1_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     20 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // height
     100,                       // mass
     10,                        // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_FIREDAMAGE    // flags3
                                      },

    {                           // MT_SORCERER2
     -1,                        // doomednum
     HS_SOR2_LOOK1,              // spawnstate
     3500,                      // spawnhealth
    0,                      // gibhealth
     HS_SOR2_WALK1,              // seestate
     hsfx_sorsit,                // seesound
     8,                         // reactiontime
     hsfx_soratk,                // attacksound
     HS_SOR2_PAIN1,              // painstate
     32,                        // painchance
     hsfx_sorpai,                // painsound
     0,                         // meleestate
     HS_SOR2_ATK1,               // missilestate
     HS_NULL,                    // crashstate
     HS_SOR2_DIE1,               // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     14,                        // speed
     16 * FRACUNIT,             // radius
     16 * FRACUNIT,             // radius
     70 * FRACUNIT,             // height
     300,                       // mass
     0,                         // damage
     hsfx_soract,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,       // flags
     MF2_PASSMOBJ, // flags2
     MF3_BOSS                   // flags3
                                     },

    {                           // MT_SOR2FX1
     -1,                        // doomednum
     HS_SOR2FX1_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_SOR2FXI1_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     20 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     1,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_SOR2FXSPARK
     -1,                        // doomednum
     HS_SOR2FXSPARK1,            // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOGRAVITY,      // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_CANNOTPUSH    // flags3
                                      },

    {                           // MT_SOR2FX2
     -1,                        // doomednum
     HS_SOR2FX2_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_SOR2FXI2_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     6 * FRACUNIT,              // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     10,                        // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT             // flags3
                                      },

    {                           // MT_SOR2TELEFADE
     -1,                        // doomednum
     HS_SOR2TELEFADE1,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP,             // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_MINOTAUR
     9,                         // doomednum
     HS_MNTR_LOOK1,              // spawnstate
     3000,                      // spawnhealth
    0,                      // gibhealth
     HS_MNTR_WALK1,              // seestate
     hsfx_minsit,                // seesound
     8,                         // reactiontime
     hsfx_minat1,                // attacksound
     HS_MNTR_PAIN1,              // painstate
     25,                        // painchance
     hsfx_minpai,                // painsound
     HS_MNTR_ATK1_1,             // meleestate
     HS_MNTR_ATK2_1,             // missilestate
     HS_NULL,                    // crashstate
     HS_MNTR_DIE1,               // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_mindth,                // deathsound
     16,                        // speed
     28 * FRACUNIT,             // radius
     28 * FRACUNIT,             // radius
     100 * FRACUNIT,            // height
     800,                       // mass
     7,                         // damage
     hsfx_minact,                // activesound
     MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,       // flags
     MF2_PASSMOBJ, // flags2
     MF3_BOSS                   // flags3
                                     },

    {                           // MT_MNTRFX1
     -1,                        // doomednum
     HS_MNTRFX1_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_MNTRFXI1_1,              // deathstate
     HS_NULL,                    // xdeathstate
     0,                         // deathsound
     20 * FRACUNIT,             // speed
     10 * FRACUNIT,             // radius
     10 * FRACUNIT,             // radius
     6 * FRACUNIT,              // height
     100,                       // mass
     3,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_FIREDAMAGE    // flags3
                                      },

    {                           // MT_MNTRFX2
     -1,                        // doomednum
     HS_MNTRFX2_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_MNTRFXI2_1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_phohit,                // deathsound
     14 * FRACUNIT,             // speed
     5 * FRACUNIT,              // radius
     5 * FRACUNIT,              // radius
     12 * FRACUNIT,             // height
     100,                       // mass
     4,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_FIREDAMAGE    // flags3
                                      },

    {                           // MT_MNTRFX3
     -1,                        // doomednum
     HS_MNTRFX3_1,               // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     0,                         // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_MNTRFXI2_1,              // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_phohit,                // deathsound
     0,                         // speed
     8 * FRACUNIT,              // radius
     8 * FRACUNIT,              // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     4,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,    // flags
     0,                         // flags2
     MF3_NOTELEPORT | MF3_FIREDAMAGE    // flags3
                                      },

    {                           // MT_AKYY
     73,                        // doomednum
     HS_AKYY1,                   // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_NOTDMATCH, // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_BKYY
     79,                        // doomednum
     HS_BKYY1,                   // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_NOTDMATCH, // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_CKEY
     80,                        // doomednum
     HS_CKYY1,                   // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL | MF_NOTDMATCH, // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMGWNDWIMPY
     10,                        // doomednum
     HS_AMG1,                    // spawnstate
     AMMO_GWND_WIMPY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMGWNDHEFTY
     12,                        // doomednum
     HS_AMG2_1,                  // spawnstate
     AMMO_GWND_HEFTY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMMACEWIMPY
     13,                        // doomednum
     HS_AMM1,                    // spawnstate
     AMMO_MACE_WIMPY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMMACEHEFTY
     16,                        // doomednum
     HS_AMM2,                    // spawnstate
     AMMO_MACE_HEFTY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMCBOWWIMPY
     18,                        // doomednum
     HS_AMC1,                    // spawnstate
     AMMO_CBOW_WIMPY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMCBOWHEFTY
     19,                        // doomednum
     HS_AMC2_1,                  // spawnstate
     AMMO_CBOW_HEFTY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMSKRDWIMPY
     20,                        // doomednum
     HS_AMS1_1,                  // spawnstate
     AMMO_SKRD_WIMPY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMSKRDHEFTY
     21,                        // doomednum
     HS_AMS2_1,                  // spawnstate
     AMMO_SKRD_HEFTY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMPHRDWIMPY
     22,                        // doomednum
     HS_AMP1_1,                  // spawnstate
     AMMO_PHRD_WIMPY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMPHRDHEFTY
     23,                        // doomednum
     HS_AMP2_1,                  // spawnstate
     AMMO_PHRD_HEFTY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMBLSRWIMPY
     54,                        // doomednum
     HS_AMB1_1,                  // spawnstate
     AMMO_BLSR_WIMPY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_AMBLSRHEFTY
     55,                        // doomednum
     HS_AMB2_1,                  // spawnstate
     AMMO_BLSR_HEFTY,           // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_SPECIAL,                // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SOUNDWIND
     42,                        // doomednum
     HS_SND_WIND,                // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOSECTOR,       // flags
     0,                         // flags2
     0                          // flags3
                                      },

    {                           // MT_SOUNDWATERFALL
     41,                        // doomednum
     HS_SND_WATERFALL,           // spawnstate
     1000,                      // spawnhealth
    0,                      // gibhealth
     HS_NULL,                    // seestate
     hsfx_None,                  // seesound
     8,                         // reactiontime
     hsfx_None,                  // attacksound
     HS_NULL,                    // painstate
     0,                         // painchance
     hsfx_None,                  // painsound
     HS_NULL,                    // meleestate
     HS_NULL,                    // missilestate
     HS_NULL,                    // crashstate
     HS_NULL,                    // deathstate
     HS_NULL,                    // xdeathstate
     hsfx_None,                  // deathsound
     0,                         // speed
     20 * FRACUNIT,             // radius
     20 * FRACUNIT,             // radius
     16 * FRACUNIT,             // height
     100,                       // mass
     0,                         // damage
     hsfx_None,                  // activesound
     MF_NOBLOCKMAP | MF_NOSECTOR,       // flags
     0,                         // flags2
     0                          // flags3
     }
};
