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
#include "sounds.h"

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

void A_FreeTargMobj(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RestoreSpecialThing1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RestoreSpecialThing2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HideThing(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_UnHideThing(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RestoreArtifact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Explode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PodPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RemovePod(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MakePod(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_InitKeyGizmo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VolcanoSet(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VolcanoBlast(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeastPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VolcBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnTeleGlitter(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnTeleGlitter2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_AccTeleGlitter(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light0(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_StaffAttackPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_StaffAttackPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeakReady(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeakRaise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeakAttackPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeakAttackPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GauntletAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireBlasterPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireBlasterPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnRippers(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireMacePL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireMacePL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MacePL1Check(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MaceBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MaceBallImpact2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_DeathBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireSkullRodPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireSkullRodPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullRodPL2Seek(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_AddPlayerRain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HideInCeiling(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullRodStorm(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RainImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireGoldWandPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireGoldWandPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePhoenixPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_InitPhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ShutdownPhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PhoenixPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FlameEnd(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FloatPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCrossbowPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCrossbowPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BoltSpark(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_NoBlocking(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullPop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FlameSnd(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckBurnGone(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckSkullFloor(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckSkullDone(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Feathers(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ChicLook(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ChicChase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ChicPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ChicAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MummyAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MummyAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MummySoul(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ContMobjSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MummyFX1Seek(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeastAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SnakeAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SnakeAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BossDeath2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadIceImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadFireGrow(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WhirlwindSeek(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ClinkAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WizAtk1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WizAtk2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WizAtk3(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GhostOff(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpMeAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpMsAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpMsAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpXDeath1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpXDeath2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpExplode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_KnightAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_DripBlood(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Sor1Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Sor1Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Srcr1Attack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorZap(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorcererRise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorRise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorSightSnd(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Srcr2Decide(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Srcr2Attack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Sor2DthInit(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorDSph(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Sor2DthLoop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorDExp(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorDBon(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BlueSpark(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GenWizard(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurAtk1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurDecide(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurAtk2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurAtk3(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurCharge(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MntrFloorFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ESound(mobj_t *actor, player_t *player, pspdef_t *psp);

state_t hereticstates[NUMHSTATES] =
{
  //  sprite     frame               tics  action                  nextstate                 state
    { HSPR_IMPX, 0,                    -1, NULL,                   HS_NULL             }, // HS_NULL
    { HSPR_ACLO, 4,                  1050, A_FreeTargMobj,         HS_NULL             }, // HS_FREETARGMOBJ
    { HSPR_PTN1, 0,                     3, NULL,                   HS_ITEM_PTN1_2      }, // HS_ITEM_PTN1_1
    { HSPR_PTN1, 1,                     3, NULL,                   HS_ITEM_PTN1_3      }, // HS_ITEM_PTN1_2
    { HSPR_PTN1, 2,                     3, NULL,                   HS_ITEM_PTN1_1      }, // HS_ITEM_PTN1_3
    { HSPR_SHLD, 0,                    -1, NULL,                   HS_NULL             }, // HS_ITEM_SHLD1
    { HSPR_SHD2, 0,                    -1, NULL,                   HS_NULL             }, // HS_ITEM_SHD2_1
    { HSPR_BAGH, 0,                    -1, NULL,                   HS_NULL             }, // HS_ITEM_BAGH1
    { HSPR_SPMP, 0,                    -1, NULL,                   HS_NULL             }, // HS_ITEM_SPMP1
    { HSPR_ACLO, 4,                  1400, NULL,                   HS_HIDESPECIAL2     }, // HS_HIDESPECIAL1
    { HSPR_ACLO, 0,                     4, A_RestoreSpecialThing1, HS_HIDESPECIAL3     }, // HS_HIDESPECIAL2
    { HSPR_ACLO, 1,                     4, NULL,                   HS_HIDESPECIAL4     }, // HS_HIDESPECIAL3
    { HSPR_ACLO, 0,                     4, NULL,                   HS_HIDESPECIAL5     }, // HS_HIDESPECIAL4
    { HSPR_ACLO, 1,                     4, NULL,                   HS_HIDESPECIAL6     }, // HS_HIDESPECIAL5
    { HSPR_ACLO, 2,                     4, NULL,                   HS_HIDESPECIAL7     }, // HS_HIDESPECIAL6
    { HSPR_ACLO, 1,                     4, NULL,                   HS_HIDESPECIAL8     }, // HS_HIDESPECIAL7
    { HSPR_ACLO, 2,                     4, NULL,                   HS_HIDESPECIAL9     }, // HS_HIDESPECIAL8
    { HSPR_ACLO, 3,                     4, NULL,                   HS_HIDESPECIAL10    }, // HS_HIDESPECIAL9
    { HSPR_ACLO, 2,                     4, NULL,                   HS_HIDESPECIAL11    }, // HS_HIDESPECIAL10
    { HSPR_ACLO, 3,                     4, A_RestoreSpecialThing2, HS_NULL             }, // HS_HIDESPECIAL11
    { HSPR_ACLO, 3,                     3, NULL,                   HS_DORMANTARTI2     }, // HS_DORMANTARTI1
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DORMANTARTI3     }, // HS_DORMANTARTI2
    { HSPR_ACLO, 3,                     3, NULL,                   HS_DORMANTARTI4     }, // HS_DORMANTARTI3
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DORMANTARTI5     }, // HS_DORMANTARTI4
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DORMANTARTI6     }, // HS_DORMANTARTI5
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DORMANTARTI7     }, // HS_DORMANTARTI6
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DORMANTARTI8     }, // HS_DORMANTARTI7
    { HSPR_ACLO, 0,                     3, NULL,                   HS_DORMANTARTI9     }, // HS_DORMANTARTI8
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DORMANTARTI10    }, // HS_DORMANTARTI9
    { HSPR_ACLO, 0,                     3, NULL,                   HS_DORMANTARTI11    }, // HS_DORMANTARTI10
    { HSPR_ACLO, 0,                  1400, A_HideThing,            HS_DORMANTARTI12    }, // HS_DORMANTARTI11
    { HSPR_ACLO, 0,                     3, A_UnHideThing,          HS_DORMANTARTI13    }, // HS_DORMANTARTI12
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DORMANTARTI14    }, // HS_DORMANTARTI13
    { HSPR_ACLO, 0,                     3, NULL,                   HS_DORMANTARTI15    }, // HS_DORMANTARTI14
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DORMANTARTI16    }, // HS_DORMANTARTI15
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DORMANTARTI17    }, // HS_DORMANTARTI16
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DORMANTARTI18    }, // HS_DORMANTARTI17
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DORMANTARTI19    }, // HS_DORMANTARTI18
    { HSPR_ACLO, 3,                     3, NULL,                   HS_DORMANTARTI20    }, // HS_DORMANTARTI19
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DORMANTARTI21    }, // HS_DORMANTARTI20
    { HSPR_ACLO, 3,                     3, A_RestoreArtifact,      HS_NULL             }, // HS_DORMANTARTI21
    { HSPR_ACLO, 3,                     3, NULL,                   HS_DEADARTI2        }, // HS_DEADARTI1
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DEADARTI3        }, // HS_DEADARTI2
    { HSPR_ACLO, 3,                     3, NULL,                   HS_DEADARTI4        }, // HS_DEADARTI3
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DEADARTI5        }, // HS_DEADARTI4
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DEADARTI6        }, // HS_DEADARTI5
    { HSPR_ACLO, 2,                     3, NULL,                   HS_DEADARTI7        }, // HS_DEADARTI6
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DEADARTI8        }, // HS_DEADARTI7
    { HSPR_ACLO, 0,                     3, NULL,                   HS_DEADARTI9        }, // HS_DEADARTI8
    { HSPR_ACLO, 1,                     3, NULL,                   HS_DEADARTI10       }, // HS_DEADARTI9
    { HSPR_ACLO, 0,                     3, NULL,                   HS_NULL             }, // HS_DEADARTI10
    { HSPR_INVS, 0 | FF_FULLBRIGHT,   350, NULL,                   HS_ARTI_INVS1       }, // HS_ARTI_INVS1
    { HSPR_PTN2, 0,                     4, NULL,                   HS_ARTI_PTN2_2      }, // HS_ARTI_PTN2_1
    { HSPR_PTN2, 1,                     4, NULL,                   HS_ARTI_PTN2_3      }, // HS_ARTI_PTN2_2
    { HSPR_PTN2, 2,                     4, NULL,                   HS_ARTI_PTN2_1      }, // HS_ARTI_PTN2_3
    { HSPR_SOAR, 0,                     5, NULL,                   HS_ARTI_SOAR2       }, // HS_ARTI_SOAR1
    { HSPR_SOAR, 1,                     5, NULL,                   HS_ARTI_SOAR3       }, // HS_ARTI_SOAR2
    { HSPR_SOAR, 2,                     5, NULL,                   HS_ARTI_SOAR4       }, // HS_ARTI_SOAR3
    { HSPR_SOAR, 1,                     5, NULL,                   HS_ARTI_SOAR1       }, // HS_ARTI_SOAR4
    { HSPR_INVU, 0,                     3, NULL,                   HS_ARTI_INVU2       }, // HS_ARTI_INVU1
    { HSPR_INVU, 1,                     3, NULL,                   HS_ARTI_INVU3       }, // HS_ARTI_INVU2
    { HSPR_INVU, 2,                     3, NULL,                   HS_ARTI_INVU4       }, // HS_ARTI_INVU3
    { HSPR_INVU, 3,                     3, NULL,                   HS_ARTI_INVU1       }, // HS_ARTI_INVU4
    { HSPR_PWBK, 0,                   350, NULL,                   HS_ARTI_PWBK1       }, // HS_ARTI_PWBK1
    { HSPR_EGGC, 0,                     6, NULL,                   HS_ARTI_EGGC2       }, // HS_ARTI_EGGC1
    { HSPR_EGGC, 1,                     6, NULL,                   HS_ARTI_EGGC3       }, // HS_ARTI_EGGC2
    { HSPR_EGGC, 2,                     6, NULL,                   HS_ARTI_EGGC4       }, // HS_ARTI_EGGC3
    { HSPR_EGGC, 1,                     6, NULL,                   HS_ARTI_EGGC1       }, // HS_ARTI_EGGC4
    { HSPR_EGGM, 0,                     4, NULL,                   HS_EGGFX2           }, // HS_EGGFX1
    { HSPR_EGGM, 1,                     4, NULL,                   HS_EGGFX3           }, // HS_EGGFX2
    { HSPR_EGGM, 2,                     4, NULL,                   HS_EGGFX4           }, // HS_EGGFX3
    { HSPR_EGGM, 3,                     4, NULL,                   HS_EGGFX5           }, // HS_EGGFX4
    { HSPR_EGGM, 4,                     4, NULL,                   HS_EGGFX1           }, // HS_EGGFX5
    { HSPR_FX01, 4 | FF_FULLBRIGHT,     3, NULL,                   HS_EGGFXI1_2        }, // HS_EGGFXI1_1
    { HSPR_FX01, 5 | FF_FULLBRIGHT,     3, NULL,                   HS_EGGFXI1_3        }, // HS_EGGFXI1_2
    { HSPR_FX01, 6 | FF_FULLBRIGHT,     3, NULL,                   HS_EGGFXI1_4        }, // HS_EGGFXI1_3
    { HSPR_FX01, 7 | FF_FULLBRIGHT,     3, NULL,                   HS_NULL             }, // HS_EGGFXI1_4
    { HSPR_SPHL, 0,                   350, NULL,                   HS_ARTI_SPHL1       }, // HS_ARTI_SPHL1
    { HSPR_TRCH, 0 | FF_FULLBRIGHT,     3, NULL,                   HS_ARTI_TRCH2       }, // HS_ARTI_TRCH1
    { HSPR_TRCH, 1 | FF_FULLBRIGHT,     3, NULL,                   HS_ARTI_TRCH3       }, // HS_ARTI_TRCH2
    { HSPR_TRCH, 2 | FF_FULLBRIGHT,     3, NULL,                   HS_ARTI_TRCH1       }, // HS_ARTI_TRCH3
    { HSPR_FBMB, 4,                   350, NULL,                   HS_ARTI_FBMB1       }, // HS_ARTI_FBMB1
    { HSPR_FBMB, 0,                    10, NULL,                   HS_FIREBOMB2        }, // HS_FIREBOMB1
    { HSPR_FBMB, 1,                    10, NULL,                   HS_FIREBOMB3        }, // HS_FIREBOMB2
    { HSPR_FBMB, 2,                    10, NULL,                   HS_FIREBOMB4        }, // HS_FIREBOMB3
    { HSPR_FBMB, 3,                    10, NULL,                   HS_FIREBOMB5        }, // HS_FIREBOMB4
    { HSPR_FBMB, 4,                     6, A_Scream,               HS_FIREBOMB6        }, // HS_FIREBOMB5
    { HSPR_XPL1, 0 | FF_FULLBRIGHT,     4, A_Explode,              HS_FIREBOMB7        }, // HS_FIREBOMB6
    { HSPR_XPL1, 1 | FF_FULLBRIGHT,     4, NULL,                   HS_FIREBOMB8        }, // HS_FIREBOMB7
    { HSPR_XPL1, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_FIREBOMB9        }, // HS_FIREBOMB8
    { HSPR_XPL1, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_FIREBOMB10       }, // HS_FIREBOMB9
    { HSPR_XPL1, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_FIREBOMB11       }, // HS_FIREBOMB10
    { HSPR_XPL1, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_FIREBOMB11
    { HSPR_ATLP, 0,                     4, NULL,                   HS_ARTI_ATLP2       }, // HS_ARTI_ATLP1
    { HSPR_ATLP, 1,                     4, NULL,                   HS_ARTI_ATLP3       }, // HS_ARTI_ATLP2
    { HSPR_ATLP, 2,                     4, NULL,                   HS_ARTI_ATLP4       }, // HS_ARTI_ATLP3
    { HSPR_ATLP, 1,                     4, NULL,                   HS_ARTI_ATLP1       }, // HS_ARTI_ATLP4
    { HSPR_PPOD, 0,                    10, NULL,                   HS_POD_WAIT1        }, // HS_POD_WAIT1
    { HSPR_PPOD, 1,                    14, A_PodPain,              HS_POD_WAIT1        }, // HS_POD_PAIN1
    { HSPR_PPOD, 2 | FF_FULLBRIGHT,     5, A_RemovePod,            HS_POD_DIE2         }, // HS_POD_DIE1
    { HSPR_PPOD, 3 | FF_FULLBRIGHT,     5, A_Scream,               HS_POD_DIE3         }, // HS_POD_DIE2
    { HSPR_PPOD, 4 | FF_FULLBRIGHT,     5, A_Explode,              HS_POD_DIE4         }, // HS_POD_DIE3
    { HSPR_PPOD, 5 | FF_FULLBRIGHT,    10, NULL,                   HS_FREETARGMOBJ     }, // HS_POD_DIE4
    { HSPR_PPOD, 8,                     3, NULL,                   HS_POD_GROW2        }, // HS_POD_GROW1
    { HSPR_PPOD, 9,                     3, NULL,                   HS_POD_GROW3        }, // HS_POD_GROW2
    { HSPR_PPOD, 10,                    3, NULL,                   HS_POD_GROW4        }, // HS_POD_GROW3
    { HSPR_PPOD, 11,                    3, NULL,                   HS_POD_GROW5        }, // HS_POD_GROW4
    { HSPR_PPOD, 12,                    3, NULL,                   HS_POD_GROW6        }, // HS_POD_GROW5
    { HSPR_PPOD, 13,                    3, NULL,                   HS_POD_GROW7        }, // HS_POD_GROW6
    { HSPR_PPOD, 14,                    3, NULL,                   HS_POD_GROW8        }, // HS_POD_GROW7
    { HSPR_PPOD, 15,                    3, NULL,                   HS_POD_WAIT1        }, // HS_POD_GROW8
    { HSPR_PPOD, 6,                     8, NULL,                   HS_PODGOO2          }, // HS_PODGOO1
    { HSPR_PPOD, 7,                     8, NULL,                   HS_PODGOO1          }, // HS_PODGOO2
    { HSPR_PPOD, 6,                    10, NULL,                   HS_NULL             }, // HS_PODGOOX
    { HSPR_AMG1, 0,                    35, A_MakePod,              HS_PODGENERATOR     }, // HS_PODGENERATOR
    { HSPR_SPSH, 0,                     8, NULL,                   HS_SPLASH2          }, // HS_SPLASH1
    { HSPR_SPSH, 1,                     8, NULL,                   HS_SPLASH3          }, // HS_SPLASH2
    { HSPR_SPSH, 2,                     8, NULL,                   HS_SPLASH4          }, // HS_SPLASH3
    { HSPR_SPSH, 3,                    16, NULL,                   HS_NULL             }, // HS_SPLASH4
    { HSPR_SPSH, 3,                    10, NULL,                   HS_NULL             }, // HS_SPLASHX
    { HSPR_SPSH, 4,                     5, NULL,                   HS_SPLASHBASE2      }, // HS_SPLASHBASE1
    { HSPR_SPSH, 5,                     5, NULL,                   HS_SPLASHBASE3      }, // HS_SPLASHBASE2
    { HSPR_SPSH, 6,                     5, NULL,                   HS_SPLASHBASE4      }, // HS_SPLASHBASE3
    { HSPR_SPSH, 7,                     5, NULL,                   HS_SPLASHBASE5      }, // HS_SPLASHBASE4
    { HSPR_SPSH, 8,                     5, NULL,                   HS_SPLASHBASE6      }, // HS_SPLASHBASE5
    { HSPR_SPSH, 9,                     5, NULL,                   HS_SPLASHBASE7      }, // HS_SPLASHBASE6
    { HSPR_SPSH, 10,                    5, NULL,                   HS_NULL             }, // HS_SPLASHBASE7
    { HSPR_LVAS, 0 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASPLASH2      }, // HS_LAVASPLASH1
    { HSPR_LVAS, 1 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASPLASH3      }, // HS_LAVASPLASH2
    { HSPR_LVAS, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASPLASH4      }, // HS_LAVASPLASH3
    { HSPR_LVAS, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASPLASH5      }, // HS_LAVASPLASH4
    { HSPR_LVAS, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASPLASH6      }, // HS_LAVASPLASH5
    { HSPR_LVAS, 5 | FF_FULLBRIGHT,     5, NULL,                   HS_NULL             }, // HS_LAVASPLASH6
    { HSPR_LVAS, 6 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASMOKE2       }, // HS_LAVASMOKE1
    { HSPR_LVAS, 7 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASMOKE3       }, // HS_LAVASMOKE2
    { HSPR_LVAS, 8 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASMOKE4       }, // HS_LAVASMOKE3
    { HSPR_LVAS, 9 | FF_FULLBRIGHT,     5, NULL,                   HS_LAVASMOKE5       }, // HS_LAVASMOKE4
    { HSPR_LVAS, 10 | FF_FULLBRIGHT,    5, NULL,                   HS_NULL             }, // HS_LAVASMOKE5
    { HSPR_SLDG, 0,                     8, NULL,                   HS_SLUDGECHUNK2     }, // HS_SLUDGECHUNK1
    { HSPR_SLDG, 1,                     8, NULL,                   HS_SLUDGECHUNK3     }, // HS_SLUDGECHUNK2
    { HSPR_SLDG, 2,                     8, NULL,                   HS_SLUDGECHUNK4     }, // HS_SLUDGECHUNK3
    { HSPR_SLDG, 3,                     8, NULL,                   HS_NULL             }, // HS_SLUDGECHUNK4
    { HSPR_SLDG, 3,                     6, NULL,                   HS_NULL             }, // HS_SLUDGECHUNKX
    { HSPR_SLDG, 4,                     5, NULL,                   HS_SLUDGESPLASH2    }, // HS_SLUDGESPLASH1
    { HSPR_SLDG, 5,                     5, NULL,                   HS_SLUDGESPLASH3    }, // HS_SLUDGESPLASH2
    { HSPR_SLDG, 6,                     5, NULL,                   HS_SLUDGESPLASH4    }, // HS_SLUDGESPLASH3
    { HSPR_SLDG, 7,                     5, NULL,                   HS_NULL             }, // HS_SLUDGESPLASH4
    { HSPR_SKH1, 0,                    -1, NULL,                   HS_NULL             }, // HS_SKULLHANG70_1
    { HSPR_SKH2, 0,                    -1, NULL,                   HS_NULL             }, // HS_SKULLHANG60_1
    { HSPR_SKH3, 0,                    -1, NULL,                   HS_NULL             }, // HS_SKULLHANG45_1
    { HSPR_SKH4, 0,                    -1, NULL,                   HS_NULL             }, // HS_SKULLHANG35_1
    { HSPR_CHDL, 0,                     4, NULL,                   HS_CHANDELIER2      }, // HS_CHANDELIER1
    { HSPR_CHDL, 1,                     4, NULL,                   HS_CHANDELIER3      }, // HS_CHANDELIER2
    { HSPR_CHDL, 2,                     4, NULL,                   HS_CHANDELIER1      }, // HS_CHANDELIER3
    { HSPR_SRTC, 0,                     4, NULL,                   HS_SERPTORCH2       }, // HS_SERPTORCH1
    { HSPR_SRTC, 1,                     4, NULL,                   HS_SERPTORCH3       }, // HS_SERPTORCH2
    { HSPR_SRTC, 2,                     4, NULL,                   HS_SERPTORCH1       }, // HS_SERPTORCH3
    { HSPR_SMPL, 0,                    -1, NULL,                   HS_NULL             }, // HS_SMALLPILLAR
    { HSPR_STGS, 0,                    -1, NULL,                   HS_NULL             }, // HS_STALAGMITESMALL
    { HSPR_STGL, 0,                    -1, NULL,                   HS_NULL             }, // HS_STALAGMITELARGE
    { HSPR_STCS, 0,                    -1, NULL,                   HS_NULL             }, // HS_STALACTITESMALL
    { HSPR_STCL, 0,                    -1, NULL,                   HS_NULL             }, // HS_STALACTITELARGE
    { HSPR_KFR1, 0 | FF_FULLBRIGHT,     3, NULL,                   HS_FIREBRAZIER2     }, // HS_FIREBRAZIER1
    { HSPR_KFR1, 1 | FF_FULLBRIGHT,     3, NULL,                   HS_FIREBRAZIER3     }, // HS_FIREBRAZIER2
    { HSPR_KFR1, 2 | FF_FULLBRIGHT,     3, NULL,                   HS_FIREBRAZIER4     }, // HS_FIREBRAZIER3
    { HSPR_KFR1, 3 | FF_FULLBRIGHT,     3, NULL,                   HS_FIREBRAZIER5     }, // HS_FIREBRAZIER4
    { HSPR_KFR1, 4 | FF_FULLBRIGHT,     3, NULL,                   HS_FIREBRAZIER6     }, // HS_FIREBRAZIER5
    { HSPR_KFR1, 5 | FF_FULLBRIGHT,     3, NULL,                   HS_FIREBRAZIER7     }, // HS_FIREBRAZIER6
    { HSPR_KFR1, 6 | FF_FULLBRIGHT,     3, NULL,                   HS_FIREBRAZIER8     }, // HS_FIREBRAZIER7
    { HSPR_KFR1, 7 | FF_FULLBRIGHT,     3, NULL,                   HS_FIREBRAZIER1     }, // HS_FIREBRAZIER8
    { HSPR_BARL, 0,                    -1, NULL,                   HS_NULL             }, // HS_BARREL
    { HSPR_BRPL, 0,                    -1, NULL,                   HS_NULL             }, // HS_BRPILLAR
    { HSPR_MOS1, 0,                    -1, NULL,                   HS_NULL             }, // HS_MOSS1
    { HSPR_MOS2, 0,                    -1, NULL,                   HS_NULL             }, // HS_MOSS2
    { HSPR_WTRH, 0 | FF_FULLBRIGHT,     6, NULL,                   HS_WALLTORCH2       }, // HS_WALLTORCH1
    { HSPR_WTRH, 1 | FF_FULLBRIGHT,     6, NULL,                   HS_WALLTORCH3       }, // HS_WALLTORCH2
    { HSPR_WTRH, 2 | FF_FULLBRIGHT,     6, NULL,                   HS_WALLTORCH1       }, // HS_WALLTORCH3
    { HSPR_HCOR, 0,                    -1, NULL,                   HS_NULL             }, // HS_HANGINGCORPSE
    { HSPR_KGZ1, 0,                     1, NULL,                   HS_KEYGIZMO2        }, // HS_KEYGIZMO1
    { HSPR_KGZ1, 0,                     1, A_InitKeyGizmo,         HS_KEYGIZMO3        }, // HS_KEYGIZMO2
    { HSPR_KGZ1, 0,                    -1, NULL,                   HS_NULL             }, // HS_KEYGIZMO3
    { HSPR_KGZB, 0,                     1, NULL,                   HS_KGZ_START        }, // HS_KGZ_START
    { HSPR_KGZB, 0 | FF_FULLBRIGHT,    -1, NULL,                   HS_NULL             }, // HS_KGZ_BLUEFLOAT1
    { HSPR_KGZG, 0 | FF_FULLBRIGHT,    -1, NULL,                   HS_NULL             }, // HS_KGZ_GREENFLOAT1
    { HSPR_KGZY, 0 | FF_FULLBRIGHT,    -1, NULL,                   HS_NULL             }, // HS_KGZ_YELLOWFLOAT1
    { HSPR_VLCO, 0,                   350, NULL,                   HS_VOLCANO2         }, // HS_VOLCANO1
    { HSPR_VLCO, 0,                    35, A_VolcanoSet,           HS_VOLCANO3         }, // HS_VOLCANO2
    { HSPR_VLCO, 1,                     3, NULL,                   HS_VOLCANO4         }, // HS_VOLCANO3
    { HSPR_VLCO, 2,                     3, NULL,                   HS_VOLCANO5         }, // HS_VOLCANO4
    { HSPR_VLCO, 3,                     3, NULL,                   HS_VOLCANO6         }, // HS_VOLCANO5
    { HSPR_VLCO, 1,                     3, NULL,                   HS_VOLCANO7         }, // HS_VOLCANO6
    { HSPR_VLCO, 2,                     3, NULL,                   HS_VOLCANO8         }, // HS_VOLCANO7
    { HSPR_VLCO, 3,                     3, NULL,                   HS_VOLCANO9         }, // HS_VOLCANO8
    { HSPR_VLCO, 4,                    10, A_VolcanoBlast,         HS_VOLCANO2         }, // HS_VOLCANO9
    { HSPR_VFBL, 0,                     4, A_BeastPuff,            HS_VOLCANOBALL2     }, // HS_VOLCANOBALL1
    { HSPR_VFBL, 1,                     4, A_BeastPuff,            HS_VOLCANOBALL1     }, // HS_VOLCANOBALL2
    { HSPR_XPL1, 0,                     4, A_VolcBallImpact,       HS_VOLCANOBALLX2    }, // HS_VOLCANOBALLX1
    { HSPR_XPL1, 1,                     4, NULL,                   HS_VOLCANOBALLX3    }, // HS_VOLCANOBALLX2
    { HSPR_XPL1, 2,                     4, NULL,                   HS_VOLCANOBALLX4    }, // HS_VOLCANOBALLX3
    { HSPR_XPL1, 3,                     4, NULL,                   HS_VOLCANOBALLX5    }, // HS_VOLCANOBALLX4
    { HSPR_XPL1, 4,                     4, NULL,                   HS_VOLCANOBALLX6    }, // HS_VOLCANOBALLX5
    { HSPR_XPL1, 5,                     4, NULL,                   HS_NULL             }, // HS_VOLCANOBALLX6
    { HSPR_VTFB, 0,                     4, NULL,                   HS_VOLCANOTBALL2    }, // HS_VOLCANOTBALL1
    { HSPR_VTFB, 1,                     4, NULL,                   HS_VOLCANOTBALL1    }, // HS_VOLCANOTBALL2
    { HSPR_SFFI, 2,                     4, NULL,                   HS_VOLCANOTBALLX2   }, // HS_VOLCANOTBALLX1
    { HSPR_SFFI, 1,                     4, NULL,                   HS_VOLCANOTBALLX3   }, // HS_VOLCANOTBALLX2
    { HSPR_SFFI, 0,                     4, NULL,                   HS_VOLCANOTBALLX4   }, // HS_VOLCANOTBALLX3
    { HSPR_SFFI, 1,                     4, NULL,                   HS_VOLCANOTBALLX5   }, // HS_VOLCANOTBALLX4
    { HSPR_SFFI, 2,                     4, NULL,                   HS_VOLCANOTBALLX6   }, // HS_VOLCANOTBALLX5
    { HSPR_SFFI, 3,                     4, NULL,                   HS_VOLCANOTBALLX7   }, // HS_VOLCANOTBALLX6
    { HSPR_SFFI, 4,                     4, NULL,                   HS_NULL             }, // HS_VOLCANOTBALLX7
    { HSPR_TGLT, 0,                     8, A_SpawnTeleGlitter,     HS_TELEGLITGEN1     }, // HS_TELEGLITGEN1
    { HSPR_TGLT, 5,                     8, A_SpawnTeleGlitter2,    HS_TELEGLITGEN2     }, // HS_TELEGLITGEN2
    { HSPR_TGLT, 0 | FF_FULLBRIGHT,     2, NULL,                   HS_TELEGLITTER1_2   }, // HS_TELEGLITTER1_1
    { HSPR_TGLT, 1 | FF_FULLBRIGHT,     2, A_AccTeleGlitter,       HS_TELEGLITTER1_3   }, // HS_TELEGLITTER1_2
    { HSPR_TGLT, 2 | FF_FULLBRIGHT,     2, NULL,                   HS_TELEGLITTER1_4   }, // HS_TELEGLITTER1_3
    { HSPR_TGLT, 3 | FF_FULLBRIGHT,     2, A_AccTeleGlitter,       HS_TELEGLITTER1_5   }, // HS_TELEGLITTER1_4
    { HSPR_TGLT, 4 | FF_FULLBRIGHT,     2, NULL,                   HS_TELEGLITTER1_1   }, // HS_TELEGLITTER1_5
    { HSPR_TGLT, 5 | FF_FULLBRIGHT,     2, NULL,                   HS_TELEGLITTER2_2   }, // HS_TELEGLITTER2_1
    { HSPR_TGLT, 6 | FF_FULLBRIGHT,     2, A_AccTeleGlitter,       HS_TELEGLITTER2_3   }, // HS_TELEGLITTER2_2
    { HSPR_TGLT, 7 | FF_FULLBRIGHT,     2, NULL,                   HS_TELEGLITTER2_4   }, // HS_TELEGLITTER2_3
    { HSPR_TGLT, 8 | FF_FULLBRIGHT,     2, A_AccTeleGlitter,       HS_TELEGLITTER2_5   }, // HS_TELEGLITTER2_4
    { HSPR_TGLT, 9 | FF_FULLBRIGHT,     2, NULL,                   HS_TELEGLITTER2_1   }, // HS_TELEGLITTER2_5
    { HSPR_TELE, 0 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG2            }, // HS_TFOG1
    { HSPR_TELE, 1 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG3            }, // HS_TFOG2
    { HSPR_TELE, 2 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG4            }, // HS_TFOG3
    { HSPR_TELE, 3 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG5            }, // HS_TFOG4
    { HSPR_TELE, 4 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG6            }, // HS_TFOG5
    { HSPR_TELE, 5 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG7            }, // HS_TFOG6
    { HSPR_TELE, 6 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG8            }, // HS_TFOG7
    { HSPR_TELE, 7 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG9            }, // HS_TFOG8
    { HSPR_TELE, 6 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG10           }, // HS_TFOG9
    { HSPR_TELE, 5 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG11           }, // HS_TFOG10
    { HSPR_TELE, 4 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG12           }, // HS_TFOG11
    { HSPR_TELE, 3 | FF_FULLBRIGHT,     6, NULL,                   HS_TFOG13           }, // HS_TFOG12
    { HSPR_TELE, 2 | FF_FULLBRIGHT,     6, NULL,                   HS_NULL             }, // HS_TFOG13
    { HSPR_STFF, 0,                     0, A_Light0,               HS_NULL             }, // HS_LIGHTDONE
    { HSPR_STFF, 0,                     1, A_WeaponReady,          HS_STAFFREADY       }, // HS_STAFFREADY
    { HSPR_STFF, 0,                     1, A_Lower,                HS_STAFFDOWN        }, // HS_STAFFDOWN
    { HSPR_STFF, 0,                     1, A_Raise,                HS_STAFFUP          }, // HS_STAFFUP
    { HSPR_STFF, 3,                     4, A_WeaponReady,          HS_STAFFREADY2_2    }, // HS_STAFFREADY2_1
    { HSPR_STFF, 4,                     4, A_WeaponReady,          HS_STAFFREADY2_3    }, // HS_STAFFREADY2_2
    { HSPR_STFF, 5,                     4, A_WeaponReady,          HS_STAFFREADY2_1    }, // HS_STAFFREADY2_3
    { HSPR_STFF, 3,                     1, A_Lower,                HS_STAFFDOWN2       }, // HS_STAFFDOWN2
    { HSPR_STFF, 3,                     1, A_Raise,                HS_STAFFUP2         }, // HS_STAFFUP2
    { HSPR_STFF, 1,                     6, NULL,                   HS_STAFFATK1_2      }, // HS_STAFFATK1_1
    { HSPR_STFF, 2,                     8, A_StaffAttackPL1,       HS_STAFFATK1_3      }, // HS_STAFFATK1_2
    { HSPR_STFF, 1,                     8, A_ReFire,               HS_STAFFREADY       }, // HS_STAFFATK1_3
    { HSPR_STFF, 6,                     6, NULL,                   HS_STAFFATK2_2      }, // HS_STAFFATK2_1
    { HSPR_STFF, 7,                     8, A_StaffAttackPL2,       HS_STAFFATK2_3      }, // HS_STAFFATK2_2
    { HSPR_STFF, 6,                     8, A_ReFire,               HS_STAFFREADY2_1    }, // HS_STAFFATK2_3
    { HSPR_PUF3, 0 | FF_FULLBRIGHT,     4, NULL,                   HS_STAFFPUFF2       }, // HS_STAFFPUFF1
    { HSPR_PUF3, 1,                     4, NULL,                   HS_STAFFPUFF3       }, // HS_STAFFPUFF2
    { HSPR_PUF3, 2,                     4, NULL,                   HS_STAFFPUFF4       }, // HS_STAFFPUFF3
    { HSPR_PUF3, 3,                     4, NULL,                   HS_NULL             }, // HS_STAFFPUFF4
    { HSPR_PUF4, 0 | FF_FULLBRIGHT,     4, NULL,                   HS_STAFFPUFF2_2     }, // HS_STAFFPUFF2_1
    { HSPR_PUF4, 1 | FF_FULLBRIGHT,     4, NULL,                   HS_STAFFPUFF2_3     }, // HS_STAFFPUFF2_2
    { HSPR_PUF4, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_STAFFPUFF2_4     }, // HS_STAFFPUFF2_3
    { HSPR_PUF4, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_STAFFPUFF2_5     }, // HS_STAFFPUFF2_4
    { HSPR_PUF4, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_STAFFPUFF2_6     }, // HS_STAFFPUFF2_5
    { HSPR_PUF4, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_STAFFPUFF2_6
    { HSPR_BEAK, 0,                     1, A_BeakReady,            HS_BEAKREADY        }, // HS_BEAKREADY
    { HSPR_BEAK, 0,                     1, A_Lower,                HS_BEAKDOWN         }, // HS_BEAKDOWN
    { HSPR_BEAK, 0,                     1, A_BeakRaise,            HS_BEAKUP           }, // HS_BEAKUP
    { HSPR_BEAK, 0,                    18, A_BeakAttackPL1,        HS_BEAKREADY        }, // HS_BEAKATK1_1
    { HSPR_BEAK, 0,                    12, A_BeakAttackPL2,        HS_BEAKREADY        }, // HS_BEAKATK2_1
    { HSPR_WGNT, 0,                    -1, NULL,                   HS_NULL             }, // HS_WGNT
    { HSPR_GAUN, 0,                     1, A_WeaponReady,          HS_GAUNTLETREADY    }, // HS_GAUNTLETREADY
    { HSPR_GAUN, 0,                     1, A_Lower,                HS_GAUNTLETDOWN     }, // HS_GAUNTLETDOWN
    { HSPR_GAUN, 0,                     1, A_Raise,                HS_GAUNTLETUP       }, // HS_GAUNTLETUP
    { HSPR_GAUN, 6,                     4, A_WeaponReady,          HS_GAUNTLETREADY2_2 }, // HS_GAUNTLETREADY2_1
    { HSPR_GAUN, 7,                     4, A_WeaponReady,          HS_GAUNTLETREADY2_3 }, // HS_GAUNTLETREADY2_2
    { HSPR_GAUN, 8,                     4, A_WeaponReady,          HS_GAUNTLETREADY2_1 }, // HS_GAUNTLETREADY2_3
    { HSPR_GAUN, 6,                     1, A_Lower,                HS_GAUNTLETDOWN2    }, // HS_GAUNTLETDOWN2
    { HSPR_GAUN, 6,                     1, A_Raise,                HS_GAUNTLETUP2      }, // HS_GAUNTLETUP2
    { HSPR_GAUN, 1,                     4, NULL,                   HS_GAUNTLETATK1_2   }, // HS_GAUNTLETATK1_1
    { HSPR_GAUN, 2,                     4, NULL,                   HS_GAUNTLETATK1_3   }, // HS_GAUNTLETATK1_2
    { HSPR_GAUN, 3 | FF_FULLBRIGHT,     4, A_GauntletAttack,       HS_GAUNTLETATK1_4   }, // HS_GAUNTLETATK1_3
    { HSPR_GAUN, 4 | FF_FULLBRIGHT,     4, A_GauntletAttack,       HS_GAUNTLETATK1_5   }, // HS_GAUNTLETATK1_4
    { HSPR_GAUN, 5 | FF_FULLBRIGHT,     4, A_GauntletAttack,       HS_GAUNTLETATK1_6   }, // HS_GAUNTLETATK1_5
    { HSPR_GAUN, 2,                     4, A_ReFire,               HS_GAUNTLETATK1_7   }, // HS_GAUNTLETATK1_6
    { HSPR_GAUN, 1,                     4, A_Light0,               HS_GAUNTLETREADY    }, // HS_GAUNTLETATK1_7
    { HSPR_GAUN, 9,                     4, NULL,                   HS_GAUNTLETATK2_2   }, // HS_GAUNTLETATK2_1
    { HSPR_GAUN, 10,                    4, NULL,                   HS_GAUNTLETATK2_3   }, // HS_GAUNTLETATK2_2
    { HSPR_GAUN, 11 | FF_FULLBRIGHT,    4, A_GauntletAttack,       HS_GAUNTLETATK2_4   }, // HS_GAUNTLETATK2_3
    { HSPR_GAUN, 12 | FF_FULLBRIGHT,    4, A_GauntletAttack,       HS_GAUNTLETATK2_5   }, // HS_GAUNTLETATK2_4
    { HSPR_GAUN, 13 | FF_FULLBRIGHT,    4, A_GauntletAttack,       HS_GAUNTLETATK2_6   }, // HS_GAUNTLETATK2_5
    { HSPR_GAUN, 10,                    4, A_ReFire,               HS_GAUNTLETATK2_7   }, // HS_GAUNTLETATK2_6
    { HSPR_GAUN, 9,                     4, A_Light0,               HS_GAUNTLETREADY2_1 }, // HS_GAUNTLETATK2_7
    { HSPR_PUF1, 0 | FF_FULLBRIGHT,     4, NULL,                   HS_GAUNTLETPUFF1_2  }, // HS_GAUNTLETPUFF1_1
    { HSPR_PUF1, 1 | FF_FULLBRIGHT,     4, NULL,                   HS_GAUNTLETPUFF1_3  }, // HS_GAUNTLETPUFF1_2
    { HSPR_PUF1, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_GAUNTLETPUFF1_4  }, // HS_GAUNTLETPUFF1_3
    { HSPR_PUF1, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_GAUNTLETPUFF1_4
    { HSPR_PUF1, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_GAUNTLETPUFF2_2  }, // HS_GAUNTLETPUFF2_1
    { HSPR_PUF1, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_GAUNTLETPUFF2_3  }, // HS_GAUNTLETPUFF2_2
    { HSPR_PUF1, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_GAUNTLETPUFF2_4  }, // HS_GAUNTLETPUFF2_3
    { HSPR_PUF1, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_GAUNTLETPUFF2_4
    { HSPR_WBLS, 0,                    -1, NULL,                   HS_NULL             }, // HS_BLSR
    { HSPR_BLSR, 0,                     1, A_WeaponReady,          HS_BLASTERREADY     }, // HS_BLASTERREADY
    { HSPR_BLSR, 0,                     1, A_Lower,                HS_BLASTERDOWN      }, // HS_BLASTERDOWN
    { HSPR_BLSR, 0,                     1, A_Raise,                HS_BLASTERUP        }, // HS_BLASTERUP
    { HSPR_BLSR, 1,                     3, NULL,                   HS_BLASTERATK1_2    }, // HS_BLASTERATK1_1
    { HSPR_BLSR, 2,                     3, NULL,                   HS_BLASTERATK1_3    }, // HS_BLASTERATK1_2
    { HSPR_BLSR, 3,                     2, A_FireBlasterPL1,       HS_BLASTERATK1_4    }, // HS_BLASTERATK1_3
    { HSPR_BLSR, 2,                     2, NULL,                   HS_BLASTERATK1_5    }, // HS_BLASTERATK1_4
    { HSPR_BLSR, 1,                     2, NULL,                   HS_BLASTERATK1_6    }, // HS_BLASTERATK1_5
    { HSPR_BLSR, 0,                     0, A_ReFire,               HS_BLASTERREADY     }, // HS_BLASTERATK1_6
    { HSPR_BLSR, 1,                     0, NULL,                   HS_BLASTERATK2_2    }, // HS_BLASTERATK2_1
    { HSPR_BLSR, 2,                     0, NULL,                   HS_BLASTERATK2_3    }, // HS_BLASTERATK2_2
    { HSPR_BLSR, 3,                     3, A_FireBlasterPL2,       HS_BLASTERATK2_4    }, // HS_BLASTERATK2_3
    { HSPR_BLSR, 2,                     4, NULL,                   HS_BLASTERATK2_5    }, // HS_BLASTERATK2_4
    { HSPR_BLSR, 1,                     4, NULL,                   HS_BLASTERATK2_6    }, // HS_BLASTERATK2_5
    { HSPR_BLSR, 0,                     0, A_ReFire,               HS_BLASTERREADY     }, // HS_BLASTERATK2_6
    { HSPR_ACLO, 4,                   200, NULL,                   HS_BLASTERFX1_1     }, // HS_BLASTERFX1_1
    { HSPR_FX18, 0 | FF_FULLBRIGHT,     3, A_SpawnRippers,         HS_BLASTERFXI1_2    }, // HS_BLASTERFXI1_1
    { HSPR_FX18, 1 | FF_FULLBRIGHT,     3, NULL,                   HS_BLASTERFXI1_3    }, // HS_BLASTERFXI1_2
    { HSPR_FX18, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERFXI1_4    }, // HS_BLASTERFXI1_3
    { HSPR_FX18, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERFXI1_5    }, // HS_BLASTERFXI1_4
    { HSPR_FX18, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERFXI1_6    }, // HS_BLASTERFXI1_5
    { HSPR_FX18, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERFXI1_7    }, // HS_BLASTERFXI1_6
    { HSPR_FX18, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_BLASTERFXI1_7
    { HSPR_FX18, 7,                     4, NULL,                   HS_BLASTERSMOKE2    }, // HS_BLASTERSMOKE1
    { HSPR_FX18, 8,                     4, NULL,                   HS_BLASTERSMOKE3    }, // HS_BLASTERSMOKE2
    { HSPR_FX18, 9,                     4, NULL,                   HS_BLASTERSMOKE4    }, // HS_BLASTERSMOKE3
    { HSPR_FX18, 10,                    4, NULL,                   HS_BLASTERSMOKE5    }, // HS_BLASTERSMOKE4
    { HSPR_FX18, 11,                    4, NULL,                   HS_NULL             }, // HS_BLASTERSMOKE5
    { HSPR_FX18, 12,                    4, NULL,                   HS_RIPPER2          }, // HS_RIPPER1
    { HSPR_FX18, 13,                    5, NULL,                   HS_RIPPER1          }, // HS_RIPPER2
    { HSPR_FX18, 14 | FF_FULLBRIGHT,    4, NULL,                   HS_RIPPERX2         }, // HS_RIPPERX1
    { HSPR_FX18, 15 | FF_FULLBRIGHT,    4, NULL,                   HS_RIPPERX3         }, // HS_RIPPERX2
    { HSPR_FX18, 16 | FF_FULLBRIGHT,    4, NULL,                   HS_RIPPERX4         }, // HS_RIPPERX3
    { HSPR_FX18, 17 | FF_FULLBRIGHT,    4, NULL,                   HS_RIPPERX5         }, // HS_RIPPERX4
    { HSPR_FX18, 18 | FF_FULLBRIGHT,    4, NULL,                   HS_NULL             }, // HS_RIPPERX5
    { HSPR_FX17, 0 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERPUFF1_2   }, // HS_BLASTERPUFF1_1
    { HSPR_FX17, 1 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERPUFF1_3   }, // HS_BLASTERPUFF1_2
    { HSPR_FX17, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERPUFF1_4   }, // HS_BLASTERPUFF1_3
    { HSPR_FX17, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERPUFF1_5   }, // HS_BLASTERPUFF1_4
    { HSPR_FX17, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_BLASTERPUFF1_5
    { HSPR_FX17, 5 | FF_FULLBRIGHT,     3, NULL,                   HS_BLASTERPUFF2_2   }, // HS_BLASTERPUFF2_1
    { HSPR_FX17, 6 | FF_FULLBRIGHT,     3, NULL,                   HS_BLASTERPUFF2_3   }, // HS_BLASTERPUFF2_2
    { HSPR_FX17, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERPUFF2_4   }, // HS_BLASTERPUFF2_3
    { HSPR_FX17, 8 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERPUFF2_5   }, // HS_BLASTERPUFF2_4
    { HSPR_FX17, 9 | FF_FULLBRIGHT,     4, NULL,                   HS_BLASTERPUFF2_6   }, // HS_BLASTERPUFF2_5
    { HSPR_FX17, 10 | FF_FULLBRIGHT,    4, NULL,                   HS_BLASTERPUFF2_7   }, // HS_BLASTERPUFF2_6
    { HSPR_FX17, 11 | FF_FULLBRIGHT,    4, NULL,                   HS_NULL             }, // HS_BLASTERPUFF2_7
    { HSPR_WMCE, 0,                    -1, NULL,                   HS_NULL             }, // HS_WMCE
    { HSPR_MACE, 0,                     1, A_WeaponReady,          HS_MACEREADY        }, // HS_MACEREADY
    { HSPR_MACE, 0,                     1, A_Lower,                HS_MACEDOWN         }, // HS_MACEDOWN
    { HSPR_MACE, 0,                     1, A_Raise,                HS_MACEUP           }, // HS_MACEUP
    { HSPR_MACE, 1,                     4, NULL,                   HS_MACEATK1_2       }, // HS_MACEATK1_1
    { HSPR_MACE, 2,                     3, A_FireMacePL1,          HS_MACEATK1_3       }, // HS_MACEATK1_2
    { HSPR_MACE, 3,                     3, A_FireMacePL1,          HS_MACEATK1_4       }, // HS_MACEATK1_3
    { HSPR_MACE, 4,                     3, A_FireMacePL1,          HS_MACEATK1_5       }, // HS_MACEATK1_4
    { HSPR_MACE, 5,                     3, A_FireMacePL1,          HS_MACEATK1_6       }, // HS_MACEATK1_5
    { HSPR_MACE, 2,                     4, A_ReFire,               HS_MACEATK1_7       }, // HS_MACEATK1_6
    { HSPR_MACE, 3,                     4, NULL,                   HS_MACEATK1_8       }, // HS_MACEATK1_7
    { HSPR_MACE, 4,                     4, NULL,                   HS_MACEATK1_9       }, // HS_MACEATK1_8
    { HSPR_MACE, 5,                     4, NULL,                   HS_MACEATK1_10      }, // HS_MACEATK1_9
    { HSPR_MACE, 1,                     4, NULL,                   HS_MACEREADY        }, // HS_MACEATK1_10
    { HSPR_MACE, 1,                     4, NULL,                   HS_MACEATK2_2       }, // HS_MACEATK2_1
    { HSPR_MACE, 3,                     4, A_FireMacePL2,          HS_MACEATK2_3       }, // HS_MACEATK2_2
    { HSPR_MACE, 1,                     4, NULL,                   HS_MACEATK2_4       }, // HS_MACEATK2_3
    { HSPR_MACE, 0,                     8, A_ReFire,               HS_MACEREADY        }, // HS_MACEATK2_4
    { HSPR_FX02, 0,                     4, A_MacePL1Check,         HS_MACEFX1_2        }, // HS_MACEFX1_1
    { HSPR_FX02, 1,                     4, A_MacePL1Check,         HS_MACEFX1_1        }, // HS_MACEFX1_2
    { HSPR_FX02, 5 | FF_FULLBRIGHT,     4, A_MaceBallImpact,       HS_MACEFXI1_2       }, // HS_MACEFXI1_1
    { HSPR_FX02, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_MACEFXI1_3       }, // HS_MACEFXI1_2
    { HSPR_FX02, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_MACEFXI1_4       }, // HS_MACEFXI1_3
    { HSPR_FX02, 8 | FF_FULLBRIGHT,     4, NULL,                   HS_MACEFXI1_5       }, // HS_MACEFXI1_4
    { HSPR_FX02, 9 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_MACEFXI1_5
    { HSPR_FX02, 2,                     4, NULL,                   HS_MACEFX2_2        }, // HS_MACEFX2_1
    { HSPR_FX02, 3,                     4, NULL,                   HS_MACEFX2_1        }, // HS_MACEFX2_2
    { HSPR_FX02, 5 | FF_FULLBRIGHT,     4, A_MaceBallImpact2,      HS_MACEFXI1_2       }, // HS_MACEFXI2_1
    { HSPR_FX02, 0,                     4, NULL,                   HS_MACEFX3_2        }, // HS_MACEFX3_1
    { HSPR_FX02, 1,                     4, NULL,                   HS_MACEFX3_1        }, // HS_MACEFX3_2
    { HSPR_FX02, 4,                    99, NULL,                   HS_MACEFX4_1        }, // HS_MACEFX4_1
    { HSPR_FX02, 2 | FF_FULLBRIGHT,     4, A_DeathBallImpact,      HS_MACEFXI1_2       }, // HS_MACEFXI4_1
    { HSPR_WSKL, 0,                    -1, NULL,                   HS_NULL             }, // HS_WSKL
    { HSPR_HROD, 0,                     1, A_WeaponReady,          HS_HORNRODREADY     }, // HS_HORNRODREADY
    { HSPR_HROD, 0,                     1, A_Lower,                HS_HORNRODDOWN      }, // HS_HORNRODDOWN
    { HSPR_HROD, 0,                     1, A_Raise,                HS_HORNRODUP        }, // HS_HORNRODUP
    { HSPR_HROD, 0,                     4, A_FireSkullRodPL1,      HS_HORNRODATK1_2    }, // HS_HORNRODATK1_1
    { HSPR_HROD, 1,                     4, A_FireSkullRodPL1,      HS_HORNRODATK1_3    }, // HS_HORNRODATK1_2
    { HSPR_HROD, 1,                     0, A_ReFire,               HS_HORNRODREADY     }, // HS_HORNRODATK1_3
    { HSPR_HROD, 2,                     2, NULL,                   HS_HORNRODATK2_2    }, // HS_HORNRODATK2_1
    { HSPR_HROD, 3,                     3, NULL,                   HS_HORNRODATK2_3    }, // HS_HORNRODATK2_2
    { HSPR_HROD, 4,                     2, NULL,                   HS_HORNRODATK2_4    }, // HS_HORNRODATK2_3
    { HSPR_HROD, 5,                     3, NULL,                   HS_HORNRODATK2_5    }, // HS_HORNRODATK2_4
    { HSPR_HROD, 6,                     4, A_FireSkullRodPL2,      HS_HORNRODATK2_6    }, // HS_HORNRODATK2_5
    { HSPR_HROD, 5,                     2, NULL,                   HS_HORNRODATK2_7    }, // HS_HORNRODATK2_6
    { HSPR_HROD, 4,                     3, NULL,                   HS_HORNRODATK2_8    }, // HS_HORNRODATK2_7
    { HSPR_HROD, 3,                     2, NULL,                   HS_HORNRODATK2_9    }, // HS_HORNRODATK2_8
    { HSPR_HROD, 2,                     2, A_ReFire,               HS_HORNRODREADY     }, // HS_HORNRODATK2_9
    { HSPR_FX00, 0 | FF_FULLBRIGHT,     6, NULL,                   HS_HRODFX1_2        }, // HS_HRODFX1_1
    { HSPR_FX00, 1 | FF_FULLBRIGHT,     6, NULL,                   HS_HRODFX1_1        }, // HS_HRODFX1_2
    { HSPR_FX00, 7 | FF_FULLBRIGHT,     5, NULL,                   HS_HRODFXI1_2       }, // HS_HRODFXI1_1
    { HSPR_FX00, 8 | FF_FULLBRIGHT,     5, NULL,                   HS_HRODFXI1_3       }, // HS_HRODFXI1_2
    { HSPR_FX00, 9 | FF_FULLBRIGHT,     4, NULL,                   HS_HRODFXI1_4       }, // HS_HRODFXI1_3
    { HSPR_FX00, 10 | FF_FULLBRIGHT,    4, NULL,                   HS_HRODFXI1_5       }, // HS_HRODFXI1_4
    { HSPR_FX00, 11 | FF_FULLBRIGHT,    3, NULL,                   HS_HRODFXI1_6       }, // HS_HRODFXI1_5
    { HSPR_FX00, 12 | FF_FULLBRIGHT,    3, NULL,                   HS_NULL             }, // HS_HRODFXI1_6
    { HSPR_FX00, 2 | FF_FULLBRIGHT,     3, NULL,                   HS_HRODFX2_2        }, // HS_HRODFX2_1
    { HSPR_FX00, 3 | FF_FULLBRIGHT,     3, A_SkullRodPL2Seek,      HS_HRODFX2_3        }, // HS_HRODFX2_2
    { HSPR_FX00, 4 | FF_FULLBRIGHT,     3, NULL,                   HS_HRODFX2_4        }, // HS_HRODFX2_3
    { HSPR_FX00, 5 | FF_FULLBRIGHT,     3, A_SkullRodPL2Seek,      HS_HRODFX2_1        }, // HS_HRODFX2_4
    { HSPR_FX00, 7 | FF_FULLBRIGHT,     5, A_AddPlayerRain,        HS_HRODFXI2_2       }, // HS_HRODFXI2_1
    { HSPR_FX00, 8 | FF_FULLBRIGHT,     5, NULL,                   HS_HRODFXI2_3       }, // HS_HRODFXI2_2
    { HSPR_FX00, 9 | FF_FULLBRIGHT,     4, NULL,                   HS_HRODFXI2_4       }, // HS_HRODFXI2_3
    { HSPR_FX00, 10 | FF_FULLBRIGHT,    3, NULL,                   HS_HRODFXI2_5       }, // HS_HRODFXI2_4
    { HSPR_FX00, 11 | FF_FULLBRIGHT,    3, NULL,                   HS_HRODFXI2_6       }, // HS_HRODFXI2_5
    { HSPR_FX00, 12 | FF_FULLBRIGHT,    3, NULL,                   HS_HRODFXI2_7       }, // HS_HRODFXI2_6
    { HSPR_FX00, 6,                     1, A_HideInCeiling,        HS_HRODFXI2_8       }, // HS_HRODFXI2_7
    { HSPR_FX00, 6,                     1, A_SkullRodStorm,        HS_HRODFXI2_8       }, // HS_HRODFXI2_8
    { HSPR_FX20, 0 | FF_FULLBRIGHT,    -1, NULL,                   HS_NULL             }, // HS_RAINPLR1_1
    { HSPR_FX21, 0 | FF_FULLBRIGHT,    -1, NULL,                   HS_NULL             }, // HS_RAINPLR2_1
    { HSPR_FX22, 0 | FF_FULLBRIGHT,    -1, NULL,                   HS_NULL             }, // HS_RAINPLR3_1
    { HSPR_FX23, 0 | FF_FULLBRIGHT,    -1, NULL,                   HS_NULL             }, // HS_RAINPLR4_1
    { HSPR_FX20, 1 | FF_FULLBRIGHT,     4, A_RainImpact,           HS_RAINPLR1X_2      }, // HS_RAINPLR1X_1
    { HSPR_FX20, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR1X_3      }, // HS_RAINPLR1X_2
    { HSPR_FX20, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR1X_4      }, // HS_RAINPLR1X_3
    { HSPR_FX20, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR1X_5      }, // HS_RAINPLR1X_4
    { HSPR_FX20, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_RAINPLR1X_5
    { HSPR_FX21, 1 | FF_FULLBRIGHT,     4, A_RainImpact,           HS_RAINPLR2X_2      }, // HS_RAINPLR2X_1
    { HSPR_FX21, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR2X_3      }, // HS_RAINPLR2X_2
    { HSPR_FX21, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR2X_4      }, // HS_RAINPLR2X_3
    { HSPR_FX21, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR2X_5      }, // HS_RAINPLR2X_4
    { HSPR_FX21, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_RAINPLR2X_5
    { HSPR_FX22, 1 | FF_FULLBRIGHT,     4, A_RainImpact,           HS_RAINPLR3X_2      }, // HS_RAINPLR3X_1
    { HSPR_FX22, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR3X_3      }, // HS_RAINPLR3X_2
    { HSPR_FX22, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR3X_4      }, // HS_RAINPLR3X_3
    { HSPR_FX22, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR3X_5      }, // HS_RAINPLR3X_4
    { HSPR_FX22, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_RAINPLR3X_5
    { HSPR_FX23, 1 | FF_FULLBRIGHT,     4, A_RainImpact,           HS_RAINPLR4X_2      }, // HS_RAINPLR4X_1
    { HSPR_FX23, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR4X_3      }, // HS_RAINPLR4X_2
    { HSPR_FX23, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR4X_4      }, // HS_RAINPLR4X_3
    { HSPR_FX23, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINPLR4X_5      }, // HS_RAINPLR4X_4
    { HSPR_FX23, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_RAINPLR4X_5
    { HSPR_FX20, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINAIRXPLR1_2   }, // HS_RAINAIRXPLR1_1
    { HSPR_FX21, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINAIRXPLR2_2   }, // HS_RAINAIRXPLR2_1
    { HSPR_FX22, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINAIRXPLR3_2   }, // HS_RAINAIRXPLR3_1
    { HSPR_FX23, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINAIRXPLR4_2   }, // HS_RAINAIRXPLR4_1
    { HSPR_FX20, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINAIRXPLR1_3   }, // HS_RAINAIRXPLR1_2
    { HSPR_FX21, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINAIRXPLR2_3   }, // HS_RAINAIRXPLR2_2
    { HSPR_FX22, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINAIRXPLR3_3   }, // HS_RAINAIRXPLR3_2
    { HSPR_FX23, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_RAINAIRXPLR4_3   }, // HS_RAINAIRXPLR4_2
    { HSPR_FX20, 8 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_RAINAIRXPLR1_3
    { HSPR_FX21, 8 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_RAINAIRXPLR2_3
    { HSPR_FX22, 8 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_RAINAIRXPLR3_3
    { HSPR_FX23, 8 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_RAINAIRXPLR4_3
    { HSPR_GWND, 0,                     1, A_WeaponReady,          HS_GOLDWANDREADY    }, // HS_GOLDWANDREADY
    { HSPR_GWND, 0,                     1, A_Lower,                HS_GOLDWANDDOWN     }, // HS_GOLDWANDDOWN
    { HSPR_GWND, 0,                     1, A_Raise,                HS_GOLDWANDUP       }, // HS_GOLDWANDUP
    { HSPR_GWND, 1,                     3, NULL,                   HS_GOLDWANDATK1_2   }, // HS_GOLDWANDATK1_1
    { HSPR_GWND, 2,                     5, A_FireGoldWandPL1,      HS_GOLDWANDATK1_3   }, // HS_GOLDWANDATK1_2
    { HSPR_GWND, 3,                     3, NULL,                   HS_GOLDWANDATK1_4   }, // HS_GOLDWANDATK1_3
    { HSPR_GWND, 3,                     0, A_ReFire,               HS_GOLDWANDREADY    }, // HS_GOLDWANDATK1_4
    { HSPR_GWND, 1,                     3, NULL,                   HS_GOLDWANDATK2_2   }, // HS_GOLDWANDATK2_1
    { HSPR_GWND, 2,                     4, A_FireGoldWandPL2,      HS_GOLDWANDATK2_3   }, // HS_GOLDWANDATK2_2
    { HSPR_GWND, 3,                     3, NULL,                   HS_GOLDWANDATK2_4   }, // HS_GOLDWANDATK2_3
    { HSPR_GWND, 3,                     0, A_ReFire,               HS_GOLDWANDREADY    }, // HS_GOLDWANDATK2_4
    { HSPR_FX01, 0 | FF_FULLBRIGHT,     6, NULL,                   HS_GWANDFX1_2       }, // HS_GWANDFX1_1
    { HSPR_FX01, 1 | FF_FULLBRIGHT,     6, NULL,                   HS_GWANDFX1_1       }, // HS_GWANDFX1_2
    { HSPR_FX01, 4 | FF_FULLBRIGHT,     3, NULL,                   HS_GWANDFXI1_2      }, // HS_GWANDFXI1_1
    { HSPR_FX01, 5 | FF_FULLBRIGHT,     3, NULL,                   HS_GWANDFXI1_3      }, // HS_GWANDFXI1_2
    { HSPR_FX01, 6 | FF_FULLBRIGHT,     3, NULL,                   HS_GWANDFXI1_4      }, // HS_GWANDFXI1_3
    { HSPR_FX01, 7 | FF_FULLBRIGHT,     3, NULL,                   HS_NULL             }, // HS_GWANDFXI1_4
    { HSPR_FX01, 2 | FF_FULLBRIGHT,     6, NULL,                   HS_GWANDFX2_2       }, // HS_GWANDFX2_1
    { HSPR_FX01, 3 | FF_FULLBRIGHT,     6, NULL,                   HS_GWANDFX2_1       }, // HS_GWANDFX2_2
    { HSPR_PUF2, 0 | FF_FULLBRIGHT,     3, NULL,                   HS_GWANDPUFF1_2     }, // HS_GWANDPUFF1_1
    { HSPR_PUF2, 1 | FF_FULLBRIGHT,     3, NULL,                   HS_GWANDPUFF1_3     }, // HS_GWANDPUFF1_2
    { HSPR_PUF2, 2 | FF_FULLBRIGHT,     3, NULL,                   HS_GWANDPUFF1_4     }, // HS_GWANDPUFF1_3
    { HSPR_PUF2, 3 | FF_FULLBRIGHT,     3, NULL,                   HS_GWANDPUFF1_5     }, // HS_GWANDPUFF1_4
    { HSPR_PUF2, 4 | FF_FULLBRIGHT,     3, NULL,                   HS_NULL             }, // HS_GWANDPUFF1_5
    { HSPR_WPHX, 0,                    -1, NULL,                   HS_NULL             }, // HS_WPHX
    { HSPR_PHNX, 0,                     1, A_WeaponReady,          HS_PHOENIXREADY     }, // HS_PHOENIXREADY
    { HSPR_PHNX, 0,                     1, A_Lower,                HS_PHOENIXDOWN      }, // HS_PHOENIXDOWN
    { HSPR_PHNX, 0,                     1, A_Raise,                HS_PHOENIXUP        }, // HS_PHOENIXUP
    { HSPR_PHNX, 1,                     5, NULL,                   HS_PHOENIXATK1_2    }, // HS_PHOENIXATK1_1
    { HSPR_PHNX, 2,                     7, A_FirePhoenixPL1,       HS_PHOENIXATK1_3    }, // HS_PHOENIXATK1_2
    { HSPR_PHNX, 3,                     4, NULL,                   HS_PHOENIXATK1_4    }, // HS_PHOENIXATK1_3
    { HSPR_PHNX, 1,                     4, NULL,                   HS_PHOENIXATK1_5    }, // HS_PHOENIXATK1_4
    { HSPR_PHNX, 1,                     0, A_ReFire,               HS_PHOENIXREADY     }, // HS_PHOENIXATK1_5
    { HSPR_PHNX, 1,                     3, A_InitPhoenixPL2,       HS_PHOENIXATK2_2    }, // HS_PHOENIXATK2_1
    { HSPR_PHNX, 2 | FF_FULLBRIGHT,     1, A_FirePhoenixPL2,       HS_PHOENIXATK2_3    }, // HS_PHOENIXATK2_2
    { HSPR_PHNX, 1,                     4, A_ReFire,               HS_PHOENIXATK2_4    }, // HS_PHOENIXATK2_3
    { HSPR_PHNX, 1,                     4, A_ShutdownPhoenixPL2,   HS_PHOENIXREADY     }, // HS_PHOENIXATK2_4
    { HSPR_FX04, 0 | FF_FULLBRIGHT,     4, A_PhoenixPuff,          HS_PHOENIXFX1_1     }, // HS_PHOENIXFX1_1
    { HSPR_FX08, 0 | FF_FULLBRIGHT,     6, A_Explode,              HS_PHOENIXFXI1_2    }, // HS_PHOENIXFXI1_1
    { HSPR_FX08, 1 | FF_FULLBRIGHT,     5, NULL,                   HS_PHOENIXFXI1_3    }, // HS_PHOENIXFXI1_2
    { HSPR_FX08, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_PHOENIXFXI1_4    }, // HS_PHOENIXFXI1_3
    { HSPR_FX08, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_PHOENIXFXI1_5    }, // HS_PHOENIXFXI1_4
    { HSPR_FX08, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_PHOENIXFXI1_6    }, // HS_PHOENIXFXI1_5
    { HSPR_FX08, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_PHOENIXFXI1_7    }, // HS_PHOENIXFXI1_6
    { HSPR_FX08, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_PHOENIXFXI1_8    }, // HS_PHOENIXFXI1_7
    { HSPR_FX08, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_PHOENIXFXI1_8
    { HSPR_FX08, 8 | FF_FULLBRIGHT,     8, NULL,                   HS_PHOENIXFXIX_1    }, // HS_PHOENIXFXIX_1
    { HSPR_FX08, 9 | FF_FULLBRIGHT,     8, NULL,                   HS_PHOENIXFXIX_2    }, // HS_PHOENIXFXIX_2
    { HSPR_FX08, 10 | FF_FULLBRIGHT,    8, NULL,                   HS_NULL             }, // HS_PHOENIXFXIX_3
    { HSPR_FX04, 1,                     4, NULL,                   HS_PHOENIXPUFF2     }, // HS_PHOENIXPUFF1
    { HSPR_FX04, 2,                     4, NULL,                   HS_PHOENIXPUFF3     }, // HS_PHOENIXPUFF2
    { HSPR_FX04, 3,                     4, NULL,                   HS_PHOENIXPUFF4     }, // HS_PHOENIXPUFF3
    { HSPR_FX04, 4,                     4, NULL,                   HS_PHOENIXPUFF5     }, // HS_PHOENIXPUFF4
    { HSPR_FX04, 5,                     4, NULL,                   HS_NULL             }, // HS_PHOENIXPUFF5
    { HSPR_FX09, 0 | FF_FULLBRIGHT,     2, NULL,                   HS_PHOENIXFX2_2     }, // HS_PHOENIXFX2_1
    { HSPR_FX09, 1 | FF_FULLBRIGHT,     2, NULL,                   HS_PHOENIXFX2_3     }, // HS_PHOENIXFX2_2
    { HSPR_FX09, 0 | FF_FULLBRIGHT,     2, NULL,                   HS_PHOENIXFX2_4     }, // HS_PHOENIXFX2_3
    { HSPR_FX09, 1 | FF_FULLBRIGHT,     2, NULL,                   HS_PHOENIXFX2_5     }, // HS_PHOENIXFX2_4
    { HSPR_FX09, 0 | FF_FULLBRIGHT,     2, NULL,                   HS_PHOENIXFX2_6     }, // HS_PHOENIXFX2_5
    { HSPR_FX09, 1 | FF_FULLBRIGHT,     2, A_FlameEnd,             HS_PHOENIXFX2_7     }, // HS_PHOENIXFX2_6
    { HSPR_FX09, 2 | FF_FULLBRIGHT,     2, NULL,                   HS_PHOENIXFX2_8     }, // HS_PHOENIXFX2_7
    { HSPR_FX09, 3 | FF_FULLBRIGHT,     2, NULL,                   HS_PHOENIXFX2_9     }, // HS_PHOENIXFX2_8
    { HSPR_FX09, 4 | FF_FULLBRIGHT,     2, NULL,                   HS_PHOENIXFX2_10    }, // HS_PHOENIXFX2_9
    { HSPR_FX09, 5 | FF_FULLBRIGHT,     2, NULL,                   HS_NULL             }, // HS_PHOENIXFX2_10
    { HSPR_FX09, 6 | FF_FULLBRIGHT,     3, NULL,                   HS_PHOENIXFXI2_2    }, // HS_PHOENIXFXI2_1
    { HSPR_FX09, 7 | FF_FULLBRIGHT,     3, A_FloatPuff,            HS_PHOENIXFXI2_3    }, // HS_PHOENIXFXI2_2
    { HSPR_FX09, 8 | FF_FULLBRIGHT,     4, NULL,                   HS_PHOENIXFXI2_4    }, // HS_PHOENIXFXI2_3
    { HSPR_FX09, 9 | FF_FULLBRIGHT,     5, NULL,                   HS_PHOENIXFXI2_5    }, // HS_PHOENIXFXI2_4
    { HSPR_FX09, 10 | FF_FULLBRIGHT,    5, NULL,                   HS_NULL             }, // HS_PHOENIXFXI2_5
    { HSPR_WBOW, 0,                    -1, NULL,                   HS_NULL             }, // HS_WBOW
    { HSPR_CRBW, 0,                     1, A_WeaponReady,          HS_CRBOW2           }, // HS_CRBOW1
    { HSPR_CRBW, 0,                     1, A_WeaponReady,          HS_CRBOW3           }, // HS_CRBOW2
    { HSPR_CRBW, 0,                     1, A_WeaponReady,          HS_CRBOW4           }, // HS_CRBOW3
    { HSPR_CRBW, 0,                     1, A_WeaponReady,          HS_CRBOW5           }, // HS_CRBOW4
    { HSPR_CRBW, 0,                     1, A_WeaponReady,          HS_CRBOW6           }, // HS_CRBOW5
    { HSPR_CRBW, 0,                     1, A_WeaponReady,          HS_CRBOW7           }, // HS_CRBOW6
    { HSPR_CRBW, 1,                     1, A_WeaponReady,          HS_CRBOW8           }, // HS_CRBOW7
    { HSPR_CRBW, 1,                     1, A_WeaponReady,          HS_CRBOW9           }, // HS_CRBOW8
    { HSPR_CRBW, 1,                     1, A_WeaponReady,          HS_CRBOW10          }, // HS_CRBOW9
    { HSPR_CRBW, 1,                     1, A_WeaponReady,          HS_CRBOW11          }, // HS_CRBOW10
    { HSPR_CRBW, 1,                     1, A_WeaponReady,          HS_CRBOW12          }, // HS_CRBOW11
    { HSPR_CRBW, 1,                     1, A_WeaponReady,          HS_CRBOW13          }, // HS_CRBOW12
    { HSPR_CRBW, 2,                     1, A_WeaponReady,          HS_CRBOW14          }, // HS_CRBOW13
    { HSPR_CRBW, 2,                     1, A_WeaponReady,          HS_CRBOW15          }, // HS_CRBOW14
    { HSPR_CRBW, 2,                     1, A_WeaponReady,          HS_CRBOW16          }, // HS_CRBOW15
    { HSPR_CRBW, 2,                     1, A_WeaponReady,          HS_CRBOW17          }, // HS_CRBOW16
    { HSPR_CRBW, 2,                     1, A_WeaponReady,          HS_CRBOW18          }, // HS_CRBOW17
    { HSPR_CRBW, 2,                     1, A_WeaponReady,          HS_CRBOW1           }, // HS_CRBOW18
    { HSPR_CRBW, 0,                     1, A_Lower,                HS_CRBOWDOWN        }, // HS_CRBOWDOWN
    { HSPR_CRBW, 0,                     1, A_Raise,                HS_CRBOWUP          }, // HS_CRBOWUP
    { HSPR_CRBW, 3,                     6, A_FireCrossbowPL1,      HS_CRBOWATK1_2      }, // HS_CRBOWATK1_1
    { HSPR_CRBW, 4,                     3, NULL,                   HS_CRBOWATK1_3      }, // HS_CRBOWATK1_2
    { HSPR_CRBW, 5,                     3, NULL,                   HS_CRBOWATK1_4      }, // HS_CRBOWATK1_3
    { HSPR_CRBW, 6,                     3, NULL,                   HS_CRBOWATK1_5      }, // HS_CRBOWATK1_4
    { HSPR_CRBW, 7,                     3, NULL,                   HS_CRBOWATK1_6      }, // HS_CRBOWATK1_5
    { HSPR_CRBW, 0,                     4, NULL,                   HS_CRBOWATK1_7      }, // HS_CRBOWATK1_6
    { HSPR_CRBW, 1,                     4, NULL,                   HS_CRBOWATK1_8      }, // HS_CRBOWATK1_7
    { HSPR_CRBW, 2,                     5, A_ReFire,               HS_CRBOW1           }, // HS_CRBOWATK1_8
    { HSPR_CRBW, 3,                     5, A_FireCrossbowPL2,      HS_CRBOWATK2_2      }, // HS_CRBOWATK2_1
    { HSPR_CRBW, 4,                     3, NULL,                   HS_CRBOWATK2_3      }, // HS_CRBOWATK2_2
    { HSPR_CRBW, 5,                     2, NULL,                   HS_CRBOWATK2_4      }, // HS_CRBOWATK2_3
    { HSPR_CRBW, 6,                     3, NULL,                   HS_CRBOWATK2_5      }, // HS_CRBOWATK2_4
    { HSPR_CRBW, 7,                     2, NULL,                   HS_CRBOWATK2_6      }, // HS_CRBOWATK2_5
    { HSPR_CRBW, 0,                     3, NULL,                   HS_CRBOWATK2_7      }, // HS_CRBOWATK2_6
    { HSPR_CRBW, 1,                     3, NULL,                   HS_CRBOWATK2_8      }, // HS_CRBOWATK2_7
    { HSPR_CRBW, 2,                     4, A_ReFire,               HS_CRBOW1           }, // HS_CRBOWATK2_8
    { HSPR_FX03, 1 | FF_FULLBRIGHT,     1, NULL,                   HS_CRBOWFX1         }, // HS_CRBOWFX1
    { HSPR_FX03, 7 | FF_FULLBRIGHT,     8, NULL,                   HS_CRBOWFXI1_2      }, // HS_CRBOWFXI1_1
    { HSPR_FX03, 8 | FF_FULLBRIGHT,     8, NULL,                   HS_CRBOWFXI1_3      }, // HS_CRBOWFXI1_2
    { HSPR_FX03, 9 | FF_FULLBRIGHT,     8, NULL,                   HS_NULL             }, // HS_CRBOWFXI1_3
    { HSPR_FX03, 1 | FF_FULLBRIGHT,     1, A_BoltSpark,            HS_CRBOWFX2         }, // HS_CRBOWFX2
    { HSPR_FX03, 0 | FF_FULLBRIGHT,     1, NULL,                   HS_CRBOWFX3         }, // HS_CRBOWFX3
    { HSPR_FX03, 2 | FF_FULLBRIGHT,     8, NULL,                   HS_CRBOWFXI3_2      }, // HS_CRBOWFXI3_1
    { HSPR_FX03, 3 | FF_FULLBRIGHT,     8, NULL,                   HS_CRBOWFXI3_3      }, // HS_CRBOWFXI3_2
    { HSPR_FX03, 4 | FF_FULLBRIGHT,     8, NULL,                   HS_NULL             }, // HS_CRBOWFXI3_3
    { HSPR_FX03, 5 | FF_FULLBRIGHT,     8, NULL,                   HS_CRBOWFX4_2       }, // HS_CRBOWFX4_1
    { HSPR_FX03, 6 | FF_FULLBRIGHT,     8, NULL,                   HS_NULL             }, // HS_CRBOWFX4_2
    { HSPR_BLOD, 2,                     8, NULL,                   HS_BLOOD2           }, // HS_BLOOD1
    { HSPR_BLOD, 1,                     8, NULL,                   HS_BLOOD3           }, // HS_BLOOD2
    { HSPR_BLOD, 0,                     8, NULL,                   HS_NULL             }, // HS_BLOOD3
    { HSPR_BLOD, 2,                     8, NULL,                   HS_BLOODSPLATTER2   }, // HS_BLOODSPLATTER1
    { HSPR_BLOD, 1,                     8, NULL,                   HS_BLOODSPLATTER3   }, // HS_BLOODSPLATTER2
    { HSPR_BLOD, 0,                     8, NULL,                   HS_NULL             }, // HS_BLOODSPLATTER3
    { HSPR_BLOD, 0,                     6, NULL,                   HS_NULL             }, // HS_BLOODSPLATTERX
    { HSPR_PLAY, 0,                    -1, NULL,                   HS_NULL             }, // HS_PLAY
    { HSPR_PLAY, 0,                     4, NULL,                   HS_PLAY_RUN2        }, // HS_PLAY_RUN1
    { HSPR_PLAY, 1,                     4, NULL,                   HS_PLAY_RUN3        }, // HS_PLAY_RUN2
    { HSPR_PLAY, 2,                     4, NULL,                   HS_PLAY_RUN4        }, // HS_PLAY_RUN3
    { HSPR_PLAY, 3,                     4, NULL,                   HS_PLAY_RUN1        }, // HS_PLAY_RUN4
    { HSPR_PLAY, 4,                    12, NULL,                   HS_PLAY             }, // HS_PLAY_ATK1
    { HSPR_PLAY, 5 | FF_FULLBRIGHT,     6, NULL,                   HS_PLAY_ATK1        }, // HS_PLAY_ATK2
    { HSPR_PLAY, 6,                     4, NULL,                   HS_PLAY_PAIN2       }, // HS_PLAY_PAIN
    { HSPR_PLAY, 6,                     4, A_Pain,                 HS_PLAY             }, // HS_PLAY_PAIN2
    { HSPR_PLAY, 7,                     6, NULL,                   HS_PLAY_DIE2        }, // HS_PLAY_DIE1
    { HSPR_PLAY, 8,                     6, A_Scream,               HS_PLAY_DIE3        }, // HS_PLAY_DIE2
    { HSPR_PLAY, 9,                     6, NULL,                   HS_PLAY_DIE4        }, // HS_PLAY_DIE3
    { HSPR_PLAY, 10,                    6, NULL,                   HS_PLAY_DIE5        }, // HS_PLAY_DIE4
    { HSPR_PLAY, 11,                    6, A_NoBlocking,           HS_PLAY_DIE6        }, // HS_PLAY_DIE5
    { HSPR_PLAY, 12,                    6, NULL,                   HS_PLAY_DIE7        }, // HS_PLAY_DIE6
    { HSPR_PLAY, 13,                    6, NULL,                   HS_PLAY_DIE8        }, // HS_PLAY_DIE7
    { HSPR_PLAY, 14,                    6, NULL,                   HS_PLAY_DIE9        }, // HS_PLAY_DIE8
    { HSPR_PLAY, 15,                   -1, NULL,                   HS_NULL             }, // HS_PLAY_DIE9
    { HSPR_PLAY, 16,                    5, A_Scream,               HS_PLAY_XDIE2       }, // HS_PLAY_XDIE1
    { HSPR_PLAY, 17,                    5, A_SkullPop,             HS_PLAY_XDIE3       }, // HS_PLAY_XDIE2
    { HSPR_PLAY, 18,                    5, A_NoBlocking,           HS_PLAY_XDIE4       }, // HS_PLAY_XDIE3
    { HSPR_PLAY, 19,                    5, NULL,                   HS_PLAY_XDIE5       }, // HS_PLAY_XDIE4
    { HSPR_PLAY, 20,                    5, NULL,                   HS_PLAY_XDIE6       }, // HS_PLAY_XDIE5
    { HSPR_PLAY, 21,                    5, NULL,                   HS_PLAY_XDIE7       }, // HS_PLAY_XDIE6
    { HSPR_PLAY, 22,                    5, NULL,                   HS_PLAY_XDIE8       }, // HS_PLAY_XDIE7
    { HSPR_PLAY, 23,                    5, NULL,                   HS_PLAY_XDIE9       }, // HS_PLAY_XDIE8
    { HSPR_PLAY, 24,                   -1, NULL,                   HS_NULL             }, // HS_PLAY_XDIE9
    { HSPR_FDTH, 0 | FF_FULLBRIGHT,     5, A_FlameSnd,             HS_PLAY_FDTH2       }, // HS_PLAY_FDTH1
    { HSPR_FDTH, 1 | FF_FULLBRIGHT,     4, NULL,                   HS_PLAY_FDTH3       }, // HS_PLAY_FDTH2
    { HSPR_FDTH, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_PLAY_FDTH4       }, // HS_PLAY_FDTH3
    { HSPR_FDTH, 3 | FF_FULLBRIGHT,     4, A_Scream,               HS_PLAY_FDTH5       }, // HS_PLAY_FDTH4
    { HSPR_FDTH, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_PLAY_FDTH6       }, // HS_PLAY_FDTH5
    { HSPR_FDTH, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_PLAY_FDTH7       }, // HS_PLAY_FDTH6
    { HSPR_FDTH, 6 | FF_FULLBRIGHT,     5, A_FlameSnd,             HS_PLAY_FDTH8       }, // HS_PLAY_FDTH7
    { HSPR_FDTH, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_PLAY_FDTH9       }, // HS_PLAY_FDTH8
    { HSPR_FDTH, 8 | FF_FULLBRIGHT,     5, NULL,                   HS_PLAY_FDTH10      }, // HS_PLAY_FDTH9
    { HSPR_FDTH, 9 | FF_FULLBRIGHT,     4, NULL,                   HS_PLAY_FDTH11      }, // HS_PLAY_FDTH10
    { HSPR_FDTH, 10 | FF_FULLBRIGHT,    5, NULL,                   HS_PLAY_FDTH12      }, // HS_PLAY_FDTH11
    { HSPR_FDTH, 11 | FF_FULLBRIGHT,    4, NULL,                   HS_PLAY_FDTH13      }, // HS_PLAY_FDTH12
    { HSPR_FDTH, 12 | FF_FULLBRIGHT,    5, NULL,                   HS_PLAY_FDTH14      }, // HS_PLAY_FDTH13
    { HSPR_FDTH, 13 | FF_FULLBRIGHT,    4, NULL,                   HS_PLAY_FDTH15      }, // HS_PLAY_FDTH14
    { HSPR_FDTH, 14 | FF_FULLBRIGHT,    5, A_NoBlocking,           HS_PLAY_FDTH16      }, // HS_PLAY_FDTH15
    { HSPR_FDTH, 15 | FF_FULLBRIGHT,    4, NULL,                   HS_PLAY_FDTH17      }, // HS_PLAY_FDTH16
    { HSPR_FDTH, 16 | FF_FULLBRIGHT,    5, NULL,                   HS_PLAY_FDTH18      }, // HS_PLAY_FDTH17
    { HSPR_FDTH, 17 | FF_FULLBRIGHT,    4, NULL,                   HS_PLAY_FDTH19      }, // HS_PLAY_FDTH18
    { HSPR_ACLO, 4,                    35, A_CheckBurnGone,        HS_PLAY_FDTH19      }, // HS_PLAY_FDTH19
    { HSPR_ACLO, 4,                     8, NULL,                   HS_NULL             }, // HS_PLAY_FDTH20
    { HSPR_BSKL, 0,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL2     }, // HS_BLOODYSKULL1
    { HSPR_BSKL, 1,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL3     }, // HS_BLOODYSKULL2
    { HSPR_BSKL, 2,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL4     }, // HS_BLOODYSKULL3
    { HSPR_BSKL, 3,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL5     }, // HS_BLOODYSKULL4
    { HSPR_BSKL, 4,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL1     }, // HS_BLOODYSKULL5
    { HSPR_BSKL, 5,                    16, A_CheckSkullDone,       HS_BLOODYSKULLX1    }, // HS_BLOODYSKULLX1
    { HSPR_BSKL, 5,                  1050, NULL,                   HS_NULL             }, // HS_BLOODYSKULLX2
    { HSPR_CHKN, 0,                    -1, NULL,                   HS_NULL             }, // HS_CHICPLAY
    { HSPR_CHKN, 0,                     3, NULL,                   HS_CHICPLAY_RUN2    }, // HS_CHICPLAY_RUN1
    { HSPR_CHKN, 1,                     3, NULL,                   HS_CHICPLAY_RUN3    }, // HS_CHICPLAY_RUN2
    { HSPR_CHKN, 0,                     3, NULL,                   HS_CHICPLAY_RUN4    }, // HS_CHICPLAY_RUN3
    { HSPR_CHKN, 1,                     3, NULL,                   HS_CHICPLAY_RUN1    }, // HS_CHICPLAY_RUN4
    { HSPR_CHKN, 2,                    12, NULL,                   HS_CHICPLAY         }, // HS_CHICPLAY_ATK1
    { HSPR_CHKN, 3,                     4, A_Feathers,             HS_CHICPLAY_PAIN2   }, // HS_CHICPLAY_PAIN
    { HSPR_CHKN, 2,                     4, A_Pain,                 HS_CHICPLAY         }, // HS_CHICPLAY_PAIN2
    { HSPR_CHKN, 0,                    10, A_ChicLook,             HS_CHICKEN_LOOK2    }, // HS_CHICKEN_LOOK1
    { HSPR_CHKN, 1,                    10, A_ChicLook,             HS_CHICKEN_LOOK1    }, // HS_CHICKEN_LOOK2
    { HSPR_CHKN, 0,                     3, A_ChicChase,            HS_CHICKEN_WALK2    }, // HS_CHICKEN_WALK1
    { HSPR_CHKN, 1,                     3, A_ChicChase,            HS_CHICKEN_WALK1    }, // HS_CHICKEN_WALK2
    { HSPR_CHKN, 3,                     5, A_Feathers,             HS_CHICKEN_PAIN2    }, // HS_CHICKEN_PAIN1
    { HSPR_CHKN, 2,                     5, A_ChicPain,             HS_CHICKEN_WALK1    }, // HS_CHICKEN_PAIN2
    { HSPR_CHKN, 0,                     8, A_FaceTarget,           HS_CHICKEN_ATK2     }, // HS_CHICKEN_ATK1
    { HSPR_CHKN, 2,                    10, A_ChicAttack,           HS_CHICKEN_WALK1    }, // HS_CHICKEN_ATK2
    { HSPR_CHKN, 4,                     6, A_Scream,               HS_CHICKEN_DIE2     }, // HS_CHICKEN_DIE1
    { HSPR_CHKN, 5,                     6, A_Feathers,             HS_CHICKEN_DIE3     }, // HS_CHICKEN_DIE2
    { HSPR_CHKN, 6,                     6, NULL,                   HS_CHICKEN_DIE4     }, // HS_CHICKEN_DIE3
    { HSPR_CHKN, 7,                     6, A_NoBlocking,           HS_CHICKEN_DIE5     }, // HS_CHICKEN_DIE4
    { HSPR_CHKN, 8,                     6, NULL,                   HS_CHICKEN_DIE6     }, // HS_CHICKEN_DIE5
    { HSPR_CHKN, 9,                     6, NULL,                   HS_CHICKEN_DIE7     }, // HS_CHICKEN_DIE6
    { HSPR_CHKN, 10,                    6, NULL,                   HS_CHICKEN_DIE8     }, // HS_CHICKEN_DIE7
    { HSPR_CHKN, 11,                   -1, NULL,                   HS_NULL             }, // HS_CHICKEN_DIE8
    { HSPR_CHKN, 12,                    3, NULL,                   HS_FEATHER2         }, // HS_FEATHER1
    { HSPR_CHKN, 13,                    3, NULL,                   HS_FEATHER3         }, // HS_FEATHER2
    { HSPR_CHKN, 14,                    3, NULL,                   HS_FEATHER4         }, // HS_FEATHER3
    { HSPR_CHKN, 15,                    3, NULL,                   HS_FEATHER5         }, // HS_FEATHER4
    { HSPR_CHKN, 16,                    3, NULL,                   HS_FEATHER6         }, // HS_FEATHER5
    { HSPR_CHKN, 15,                    3, NULL,                   HS_FEATHER7         }, // HS_FEATHER6
    { HSPR_CHKN, 14,                    3, NULL,                   HS_FEATHER8         }, // HS_FEATHER7
    { HSPR_CHKN, 13,                    3, NULL,                   HS_FEATHER1         }, // HS_FEATHER8
    { HSPR_CHKN, 13,                    6, NULL,                   HS_NULL             }, // HS_FEATHERX
    { HSPR_MUMM, 0,                    10, A_Look,                 HS_MUMMY_LOOK2      }, // HS_MUMMY_LOOK1
    { HSPR_MUMM, 1,                    10, A_Look,                 HS_MUMMY_LOOK1      }, // HS_MUMMY_LOOK2
    { HSPR_MUMM, 0,                     4, A_Chase,                HS_MUMMY_WALK2      }, // HS_MUMMY_WALK1
    { HSPR_MUMM, 1,                     4, A_Chase,                HS_MUMMY_WALK3      }, // HS_MUMMY_WALK2
    { HSPR_MUMM, 2,                     4, A_Chase,                HS_MUMMY_WALK4      }, // HS_MUMMY_WALK3
    { HSPR_MUMM, 3,                     4, A_Chase,                HS_MUMMY_WALK1      }, // HS_MUMMY_WALK4
    { HSPR_MUMM, 4,                     6, A_FaceTarget,           HS_MUMMY_ATK2       }, // HS_MUMMY_ATK1
    { HSPR_MUMM, 5,                     6, A_MummyAttack,          HS_MUMMY_ATK3       }, // HS_MUMMY_ATK2
    { HSPR_MUMM, 6,                     6, A_FaceTarget,           HS_MUMMY_WALK1      }, // HS_MUMMY_ATK3
    { HSPR_MUMM, 23,                    5, A_FaceTarget,           HS_MUMMYL_ATK2      }, // HS_MUMMYL_ATK1
    { HSPR_MUMM, 24 | FF_FULLBRIGHT,    5, A_FaceTarget,           HS_MUMMYL_ATK3      }, // HS_MUMMYL_ATK2
    { HSPR_MUMM, 23,                    5, A_FaceTarget,           HS_MUMMYL_ATK4      }, // HS_MUMMYL_ATK3
    { HSPR_MUMM, 24 | FF_FULLBRIGHT,    5, A_FaceTarget,           HS_MUMMYL_ATK5      }, // HS_MUMMYL_ATK4
    { HSPR_MUMM, 23,                    5, A_FaceTarget,           HS_MUMMYL_ATK6      }, // HS_MUMMYL_ATK5
    { HSPR_MUMM, 24 | FF_FULLBRIGHT,   15, A_MummyAttack2,         HS_MUMMY_WALK1      }, // HS_MUMMYL_ATK6
    { HSPR_MUMM, 7,                     4, NULL,                   HS_MUMMY_PAIN2      }, // HS_MUMMY_PAIN1
    { HSPR_MUMM, 7,                     4, A_Pain,                 HS_MUMMY_WALK1      }, // HS_MUMMY_PAIN2
    { HSPR_MUMM, 8,                     5, NULL,                   HS_MUMMY_DIE2       }, // HS_MUMMY_DIE1
    { HSPR_MUMM, 9,                     5, A_Scream,               HS_MUMMY_DIE3       }, // HS_MUMMY_DIE2
    { HSPR_MUMM, 10,                    5, A_MummySoul,            HS_MUMMY_DIE4       }, // HS_MUMMY_DIE3
    { HSPR_MUMM, 11,                    5, NULL,                   HS_MUMMY_DIE5       }, // HS_MUMMY_DIE4
    { HSPR_MUMM, 12,                    5, A_NoBlocking,           HS_MUMMY_DIE6       }, // HS_MUMMY_DIE5
    { HSPR_MUMM, 13,                    5, NULL,                   HS_MUMMY_DIE7       }, // HS_MUMMY_DIE6
    { HSPR_MUMM, 14,                    5, NULL,                   HS_MUMMY_DIE8       }, // HS_MUMMY_DIE7
    { HSPR_MUMM, 15,                   -1, NULL,                   HS_NULL             }, // HS_MUMMY_DIE8
    { HSPR_MUMM, 16,                    5, NULL,                   HS_MUMMY_SOUL2      }, // HS_MUMMY_SOUL1
    { HSPR_MUMM, 17,                    5, NULL,                   HS_MUMMY_SOUL3      }, // HS_MUMMY_SOUL2
    { HSPR_MUMM, 18,                    5, NULL,                   HS_MUMMY_SOUL4      }, // HS_MUMMY_SOUL3
    { HSPR_MUMM, 19,                    9, NULL,                   HS_MUMMY_SOUL5      }, // HS_MUMMY_SOUL4
    { HSPR_MUMM, 20,                    5, NULL,                   HS_MUMMY_SOUL6      }, // HS_MUMMY_SOUL5
    { HSPR_MUMM, 21,                    5, NULL,                   HS_MUMMY_SOUL7      }, // HS_MUMMY_SOUL6
    { HSPR_MUMM, 22,                    5, NULL,                   HS_NULL             }, // HS_MUMMY_SOUL7
    { HSPR_FX15, 0 | FF_FULLBRIGHT,     5, A_ContMobjSound,        HS_MUMMYFX1_2       }, // HS_MUMMYFX1_1
    { HSPR_FX15, 1 | FF_FULLBRIGHT,     5, A_MummyFX1Seek,         HS_MUMMYFX1_3       }, // HS_MUMMYFX1_2
    { HSPR_FX15, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_MUMMYFX1_4       }, // HS_MUMMYFX1_3
    { HSPR_FX15, 1 | FF_FULLBRIGHT,     5, A_MummyFX1Seek,         HS_MUMMYFX1_1       }, // HS_MUMMYFX1_4
    { HSPR_FX15, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_MUMMYFXI1_2      }, // HS_MUMMYFXI1_1
    { HSPR_FX15, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_MUMMYFXI1_3      }, // HS_MUMMYFXI1_2
    { HSPR_FX15, 5 | FF_FULLBRIGHT,     5, NULL,                   HS_MUMMYFXI1_4      }, // HS_MUMMYFXI1_3
    { HSPR_FX15, 6 | FF_FULLBRIGHT,     5, NULL,                   HS_NULL             }, // HS_MUMMYFXI1_4
    { HSPR_BEAS, 0,                    10, A_Look,                 HS_BEAST_LOOK2      }, // HS_BEAST_LOOK1
    { HSPR_BEAS, 1,                    10, A_Look,                 HS_BEAST_LOOK1      }, // HS_BEAST_LOOK2
    { HSPR_BEAS, 0,                     3, A_Chase,                HS_BEAST_WALK2      }, // HS_BEAST_WALK1
    { HSPR_BEAS, 1,                     3, A_Chase,                HS_BEAST_WALK3      }, // HS_BEAST_WALK2
    { HSPR_BEAS, 2,                     3, A_Chase,                HS_BEAST_WALK4      }, // HS_BEAST_WALK3
    { HSPR_BEAS, 3,                     3, A_Chase,                HS_BEAST_WALK5      }, // HS_BEAST_WALK4
    { HSPR_BEAS, 4,                     3, A_Chase,                HS_BEAST_WALK6      }, // HS_BEAST_WALK5
    { HSPR_BEAS, 5,                     3, A_Chase,                HS_BEAST_WALK1      }, // HS_BEAST_WALK6
    { HSPR_BEAS, 7,                    10, A_FaceTarget,           HS_BEAST_ATK2       }, // HS_BEAST_ATK1
    { HSPR_BEAS, 8,                    10, A_BeastAttack,          HS_BEAST_WALK1      }, // HS_BEAST_ATK2
    { HSPR_BEAS, 6,                     3, NULL,                   HS_BEAST_PAIN2      }, // HS_BEAST_PAIN1
    { HSPR_BEAS, 6,                     3, A_Pain,                 HS_BEAST_WALK1      }, // HS_BEAST_PAIN2
    { HSPR_BEAS, 17,                    6, NULL,                   HS_BEAST_DIE2       }, // HS_BEAST_DIE1
    { HSPR_BEAS, 18,                    6, A_Scream,               HS_BEAST_DIE3       }, // HS_BEAST_DIE2
    { HSPR_BEAS, 19,                    6, NULL,                   HS_BEAST_DIE4       }, // HS_BEAST_DIE3
    { HSPR_BEAS, 20,                    6, NULL,                   HS_BEAST_DIE5       }, // HS_BEAST_DIE4
    { HSPR_BEAS, 21,                    6, NULL,                   HS_BEAST_DIE6       }, // HS_BEAST_DIE5
    { HSPR_BEAS, 22,                    6, A_NoBlocking,           HS_BEAST_DIE7       }, // HS_BEAST_DIE6
    { HSPR_BEAS, 23,                    6, NULL,                   HS_BEAST_DIE8       }, // HS_BEAST_DIE7
    { HSPR_BEAS, 24,                    6, NULL,                   HS_BEAST_DIE9       }, // HS_BEAST_DIE8
    { HSPR_BEAS, 25,                   -1, NULL,                   HS_NULL             }, // HS_BEAST_DIE9
    { HSPR_BEAS, 9,                     5, NULL,                   HS_BEAST_XDIE2      }, // HS_BEAST_XDIE1
    { HSPR_BEAS, 10,                    6, A_Scream,               HS_BEAST_XDIE3      }, // HS_BEAST_XDIE2
    { HSPR_BEAS, 11,                    5, NULL,                   HS_BEAST_XDIE4      }, // HS_BEAST_XDIE3
    { HSPR_BEAS, 12,                    6, NULL,                   HS_BEAST_XDIE5      }, // HS_BEAST_XDIE4
    { HSPR_BEAS, 13,                    5, NULL,                   HS_BEAST_XDIE6      }, // HS_BEAST_XDIE5
    { HSPR_BEAS, 14,                    6, A_NoBlocking,           HS_BEAST_XDIE7      }, // HS_BEAST_XDIE6
    { HSPR_BEAS, 15,                    5, NULL,                   HS_BEAST_XDIE8      }, // HS_BEAST_XDIE7
    { HSPR_BEAS, 16,                   -1, NULL,                   HS_NULL             }, // HS_BEAST_XDIE8
    { HSPR_FRB1, 0,                     2, A_BeastPuff,            HS_BEASTBALL2       }, // HS_BEASTBALL1
    { HSPR_FRB1, 0,                     2, A_BeastPuff,            HS_BEASTBALL3       }, // HS_BEASTBALL2
    { HSPR_FRB1, 1,                     2, A_BeastPuff,            HS_BEASTBALL4       }, // HS_BEASTBALL3
    { HSPR_FRB1, 1,                     2, A_BeastPuff,            HS_BEASTBALL5       }, // HS_BEASTBALL4
    { HSPR_FRB1, 2,                     2, A_BeastPuff,            HS_BEASTBALL6       }, // HS_BEASTBALL5
    { HSPR_FRB1, 2,                     2, A_BeastPuff,            HS_BEASTBALL1       }, // HS_BEASTBALL6
    { HSPR_FRB1, 3,                     4, NULL,                   HS_BEASTBALLX2      }, // HS_BEASTBALLX1
    { HSPR_FRB1, 4,                     4, NULL,                   HS_BEASTBALLX3      }, // HS_BEASTBALLX2
    { HSPR_FRB1, 5,                     4, NULL,                   HS_BEASTBALLX4      }, // HS_BEASTBALLX3
    { HSPR_FRB1, 6,                     4, NULL,                   HS_BEASTBALLX5      }, // HS_BEASTBALLX4
    { HSPR_FRB1, 7,                     4, NULL,                   HS_NULL             }, // HS_BEASTBALLX5
    { HSPR_FRB1, 0,                     4, NULL,                   HS_BURNBALL2        }, // HS_BURNBALL1
    { HSPR_FRB1, 1,                     4, NULL,                   HS_BURNBALL3        }, // HS_BURNBALL2
    { HSPR_FRB1, 2,                     4, NULL,                   HS_BURNBALL4        }, // HS_BURNBALL3
    { HSPR_FRB1, 3,                     4, NULL,                   HS_BURNBALL5        }, // HS_BURNBALL4
    { HSPR_FRB1, 4,                     4, NULL,                   HS_BURNBALL6        }, // HS_BURNBALL5
    { HSPR_FRB1, 5,                     4, NULL,                   HS_BURNBALL7        }, // HS_BURNBALL6
    { HSPR_FRB1, 6,                     4, NULL,                   HS_BURNBALL8        }, // HS_BURNBALL7
    { HSPR_FRB1, 7,                     4, NULL,                   HS_NULL             }, // HS_BURNBALL8
    { HSPR_FRB1, 0 | FF_FULLBRIGHT,     4, NULL,                   HS_BURNBALLFB2      }, // HS_BURNBALLFB1
    { HSPR_FRB1, 1 | FF_FULLBRIGHT,     4, NULL,                   HS_BURNBALLFB3      }, // HS_BURNBALLFB2
    { HSPR_FRB1, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_BURNBALLFB4      }, // HS_BURNBALLFB3
    { HSPR_FRB1, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_BURNBALLFB5      }, // HS_BURNBALLFB4
    { HSPR_FRB1, 4 | FF_FULLBRIGHT,     4, NULL,                   HS_BURNBALLFB6      }, // HS_BURNBALLFB5
    { HSPR_FRB1, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_BURNBALLFB7      }, // HS_BURNBALLFB6
    { HSPR_FRB1, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_BURNBALLFB8      }, // HS_BURNBALLFB7
    { HSPR_FRB1, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_BURNBALLFB8
    { HSPR_FRB1, 3,                     4, NULL,                   HS_PUFFY2           }, // HS_PUFFY1
    { HSPR_FRB1, 4,                     4, NULL,                   HS_PUFFY3           }, // HS_PUFFY2
    { HSPR_FRB1, 5,                     4, NULL,                   HS_PUFFY4           }, // HS_PUFFY3
    { HSPR_FRB1, 6,                     4, NULL,                   HS_PUFFY5           }, // HS_PUFFY4
    { HSPR_FRB1, 7,                     4, NULL,                   HS_NULL             }, // HS_PUFFY5
    { HSPR_SNKE, 0,                    10, A_Look,                 HS_SNAKE_LOOK2      }, // HS_SNAKE_LOOK1
    { HSPR_SNKE, 1,                    10, A_Look,                 HS_SNAKE_LOOK1      }, // HS_SNAKE_LOOK2
    { HSPR_SNKE, 0,                     4, A_Chase,                HS_SNAKE_WALK2      }, // HS_SNAKE_WALK1
    { HSPR_SNKE, 1,                     4, A_Chase,                HS_SNAKE_WALK3      }, // HS_SNAKE_WALK2
    { HSPR_SNKE, 2,                     4, A_Chase,                HS_SNAKE_WALK4      }, // HS_SNAKE_WALK3
    { HSPR_SNKE, 3,                     4, A_Chase,                HS_SNAKE_WALK1      }, // HS_SNAKE_WALK4
    { HSPR_SNKE, 5,                     5, A_FaceTarget,           HS_SNAKE_ATK2       }, // HS_SNAKE_ATK1
    { HSPR_SNKE, 5,                     5, A_FaceTarget,           HS_SNAKE_ATK3       }, // HS_SNAKE_ATK2
    { HSPR_SNKE, 5,                     4, A_SnakeAttack,          HS_SNAKE_ATK4       }, // HS_SNAKE_ATK3
    { HSPR_SNKE, 5,                     4, A_SnakeAttack,          HS_SNAKE_ATK5       }, // HS_SNAKE_ATK4
    { HSPR_SNKE, 5,                     4, A_SnakeAttack,          HS_SNAKE_ATK6       }, // HS_SNAKE_ATK5
    { HSPR_SNKE, 5,                     5, A_FaceTarget,           HS_SNAKE_ATK7       }, // HS_SNAKE_ATK6
    { HSPR_SNKE, 5,                     5, A_FaceTarget,           HS_SNAKE_ATK8       }, // HS_SNAKE_ATK7
    { HSPR_SNKE, 5,                     5, A_FaceTarget,           HS_SNAKE_ATK9       }, // HS_SNAKE_ATK8
    { HSPR_SNKE, 5,                     4, A_SnakeAttack2,         HS_SNAKE_WALK1      }, // HS_SNAKE_ATK9
    { HSPR_SNKE, 4,                     3, NULL,                   HS_SNAKE_PAIN2      }, // HS_SNAKE_PAIN1
    { HSPR_SNKE, 4,                     3, A_Pain,                 HS_SNAKE_WALK1      }, // HS_SNAKE_PAIN2
    { HSPR_SNKE, 6,                     5, NULL,                   HS_SNAKE_DIE2       }, // HS_SNAKE_DIE1
    { HSPR_SNKE, 7,                     5, A_Scream,               HS_SNAKE_DIE3       }, // HS_SNAKE_DIE2
    { HSPR_SNKE, 8,                     5, NULL,                   HS_SNAKE_DIE4       }, // HS_SNAKE_DIE3
    { HSPR_SNKE, 9,                     5, NULL,                   HS_SNAKE_DIE5       }, // HS_SNAKE_DIE4
    { HSPR_SNKE, 10,                    5, NULL,                   HS_SNAKE_DIE6       }, // HS_SNAKE_DIE5
    { HSPR_SNKE, 11,                    5, NULL,                   HS_SNAKE_DIE7       }, // HS_SNAKE_DIE6
    { HSPR_SNKE, 12,                    5, A_NoBlocking,           HS_SNAKE_DIE8       }, // HS_SNAKE_DIE7
    { HSPR_SNKE, 13,                    5, NULL,                   HS_SNAKE_DIE9       }, // HS_SNAKE_DIE8
    { HSPR_SNKE, 14,                    5, NULL,                   HS_SNAKE_DIE10      }, // HS_SNAKE_DIE9
    { HSPR_SNKE, 15,                   -1, NULL,                   HS_NULL             }, // HS_SNAKE_DIE10
    { HSPR_SNFX, 0 | FF_FULLBRIGHT,     5, NULL,                   HS_SNAKEPRO_A2      }, // HS_SNAKEPRO_A1
    { HSPR_SNFX, 1 | FF_FULLBRIGHT,     5, NULL,                   HS_SNAKEPRO_A3      }, // HS_SNAKEPRO_A2
    { HSPR_SNFX, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_SNAKEPRO_A4      }, // HS_SNAKEPRO_A3
    { HSPR_SNFX, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_SNAKEPRO_A1      }, // HS_SNAKEPRO_A4
    { HSPR_SNFX, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_SNAKEPRO_AX2     }, // HS_SNAKEPRO_AX1
    { HSPR_SNFX, 5 | FF_FULLBRIGHT,     5, NULL,                   HS_SNAKEPRO_AX3     }, // HS_SNAKEPRO_AX2
    { HSPR_SNFX, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_SNAKEPRO_AX4     }, // HS_SNAKEPRO_AX3
    { HSPR_SNFX, 7 | FF_FULLBRIGHT,     3, NULL,                   HS_SNAKEPRO_AX5     }, // HS_SNAKEPRO_AX4
    { HSPR_SNFX, 8 | FF_FULLBRIGHT,     3, NULL,                   HS_NULL             }, // HS_SNAKEPRO_AX5
    { HSPR_SNFX, 9 | FF_FULLBRIGHT,     6, NULL,                   HS_SNAKEPRO_B2      }, // HS_SNAKEPRO_B1
    { HSPR_SNFX, 10 | FF_FULLBRIGHT,    6, NULL,                   HS_SNAKEPRO_B1      }, // HS_SNAKEPRO_B2
    { HSPR_SNFX, 11 | FF_FULLBRIGHT,    5, NULL,                   HS_SNAKEPRO_BX2     }, // HS_SNAKEPRO_BX1
    { HSPR_SNFX, 12 | FF_FULLBRIGHT,    5, NULL,                   HS_SNAKEPRO_BX3     }, // HS_SNAKEPRO_BX2
    { HSPR_SNFX, 13 | FF_FULLBRIGHT,    4, NULL,                   HS_SNAKEPRO_BX4     }, // HS_SNAKEPRO_BX3
    { HSPR_SNFX, 14 | FF_FULLBRIGHT,    3, NULL,                   HS_NULL             }, // HS_SNAKEPRO_BX4
    { HSPR_HEAD, 0,                    10, A_Look,                 HS_HEAD_LOOK        }, // HS_HEAD_LOOK
    { HSPR_HEAD, 0,                     4, A_Chase,                HS_HEAD_FLOAT       }, // HS_HEAD_FLOAT
    { HSPR_HEAD, 0,                     5, A_FaceTarget,           HS_HEAD_ATK2        }, // HS_HEAD_ATK1
    { HSPR_HEAD, 1,                    20, A_HeadAttack2,          HS_HEAD_FLOAT       }, // HS_HEAD_ATK2
    { HSPR_HEAD, 0,                     4, NULL,                   HS_HEAD_PAIN2       }, // HS_HEAD_PAIN1
    { HSPR_HEAD, 0,                     4, A_Pain,                 HS_HEAD_FLOAT       }, // HS_HEAD_PAIN2
    { HSPR_HEAD, 2,                     7, NULL,                   HS_HEAD_DIE2        }, // HS_HEAD_DIE1
    { HSPR_HEAD, 3,                     7, A_Scream,               HS_HEAD_DIE3        }, // HS_HEAD_DIE2
    { HSPR_HEAD, 4,                     7, NULL,                   HS_HEAD_DIE4        }, // HS_HEAD_DIE3
    { HSPR_HEAD, 5,                     7, NULL,                   HS_HEAD_DIE5        }, // HS_HEAD_DIE4
    { HSPR_HEAD, 6,                     7, A_NoBlocking,           HS_HEAD_DIE6        }, // HS_HEAD_DIE5
    { HSPR_HEAD, 7,                     7, NULL,                   HS_HEAD_DIE7        }, // HS_HEAD_DIE6
    { HSPR_HEAD, 8,                    -1, A_BossDeath2,           HS_NULL             }, // HS_HEAD_DIE7
    { HSPR_FX05, 0,                     6, NULL,                   HS_HEADFX1_2        }, // HS_HEADFX1_1
    { HSPR_FX05, 1,                     6, NULL,                   HS_HEADFX1_3        }, // HS_HEADFX1_2
    { HSPR_FX05, 2,                     6, NULL,                   HS_HEADFX1_1        }, // HS_HEADFX1_3
    { HSPR_FX05, 3,                     5, A_HeadIceImpact,        HS_HEADFXI1_2       }, // HS_HEADFXI1_1
    { HSPR_FX05, 4,                     5, NULL,                   HS_HEADFXI1_3       }, // HS_HEADFXI1_2
    { HSPR_FX05, 5,                     5, NULL,                   HS_HEADFXI1_4       }, // HS_HEADFXI1_3
    { HSPR_FX05, 6,                     5, NULL,                   HS_NULL             }, // HS_HEADFXI1_4
    { HSPR_FX05, 7,                     6, NULL,                   HS_HEADFX2_2        }, // HS_HEADFX2_1
    { HSPR_FX05, 8,                     6, NULL,                   HS_HEADFX2_3        }, // HS_HEADFX2_2
    { HSPR_FX05, 9,                     6, NULL,                   HS_HEADFX2_1        }, // HS_HEADFX2_3
    { HSPR_FX05, 3,                     5, NULL,                   HS_HEADFXI2_2       }, // HS_HEADFXI2_1
    { HSPR_FX05, 4,                     5, NULL,                   HS_HEADFXI2_3       }, // HS_HEADFXI2_2
    { HSPR_FX05, 5,                     5, NULL,                   HS_HEADFXI2_4       }, // HS_HEADFXI2_3
    { HSPR_FX05, 6,                     5, NULL,                   HS_NULL             }, // HS_HEADFXI2_4
    { HSPR_FX06, 0,                     4, A_HeadFireGrow,         HS_HEADFX3_2        }, // HS_HEADFX3_1
    { HSPR_FX06, 1,                     4, A_HeadFireGrow,         HS_HEADFX3_3        }, // HS_HEADFX3_2
    { HSPR_FX06, 2,                     4, A_HeadFireGrow,         HS_HEADFX3_1        }, // HS_HEADFX3_3
    { HSPR_FX06, 0,                     5, NULL,                   HS_HEADFX3_5        }, // HS_HEADFX3_4
    { HSPR_FX06, 1,                     5, NULL,                   HS_HEADFX3_6        }, // HS_HEADFX3_5
    { HSPR_FX06, 2,                     5, NULL,                   HS_HEADFX3_4        }, // HS_HEADFX3_6
    { HSPR_FX06, 3,                     5, NULL,                   HS_HEADFXI3_2       }, // HS_HEADFXI3_1
    { HSPR_FX06, 4,                     5, NULL,                   HS_HEADFXI3_3       }, // HS_HEADFXI3_2
    { HSPR_FX06, 5,                     5, NULL,                   HS_HEADFXI3_4       }, // HS_HEADFXI3_3
    { HSPR_FX06, 6,                     5, NULL,                   HS_NULL             }, // HS_HEADFXI3_4
    { HSPR_FX07, 3,                     3, NULL,                   HS_HEADFX4_2        }, // HS_HEADFX4_1
    { HSPR_FX07, 4,                     3, NULL,                   HS_HEADFX4_3        }, // HS_HEADFX4_2
    { HSPR_FX07, 5,                     3, NULL,                   HS_HEADFX4_4        }, // HS_HEADFX4_3
    { HSPR_FX07, 6,                     3, NULL,                   HS_HEADFX4_5        }, // HS_HEADFX4_4
    { HSPR_FX07, 0,                     3, A_WhirlwindSeek,        HS_HEADFX4_6        }, // HS_HEADFX4_5
    { HSPR_FX07, 1,                     3, A_WhirlwindSeek,        HS_HEADFX4_7        }, // HS_HEADFX4_6
    { HSPR_FX07, 2,                     3, A_WhirlwindSeek,        HS_HEADFX4_5        }, // HS_HEADFX4_7
    { HSPR_FX07, 6,                     4, NULL,                   HS_HEADFXI4_2       }, // HS_HEADFXI4_1
    { HSPR_FX07, 5,                     4, NULL,                   HS_HEADFXI4_3       }, // HS_HEADFXI4_2
    { HSPR_FX07, 4,                     4, NULL,                   HS_HEADFXI4_4       }, // HS_HEADFXI4_3
    { HSPR_FX07, 3,                     4, NULL,                   HS_NULL             }, // HS_HEADFXI4_4
    { HSPR_CLNK, 0,                    10, A_Look,                 HS_CLINK_LOOK2      }, // HS_CLINK_LOOK1
    { HSPR_CLNK, 1,                    10, A_Look,                 HS_CLINK_LOOK1      }, // HS_CLINK_LOOK2
    { HSPR_CLNK, 0,                     3, A_Chase,                HS_CLINK_WALK2      }, // HS_CLINK_WALK1
    { HSPR_CLNK, 1,                     3, A_Chase,                HS_CLINK_WALK3      }, // HS_CLINK_WALK2
    { HSPR_CLNK, 2,                     3, A_Chase,                HS_CLINK_WALK4      }, // HS_CLINK_WALK3
    { HSPR_CLNK, 3,                     3, A_Chase,                HS_CLINK_WALK1      }, // HS_CLINK_WALK4
    { HSPR_CLNK, 4,                     5, A_FaceTarget,           HS_CLINK_ATK2       }, // HS_CLINK_ATK1
    { HSPR_CLNK, 5,                     4, A_FaceTarget,           HS_CLINK_ATK3       }, // HS_CLINK_ATK2
    { HSPR_CLNK, 6,                     7, A_ClinkAttack,          HS_CLINK_WALK1      }, // HS_CLINK_ATK3
    { HSPR_CLNK, 7,                     3, NULL,                   HS_CLINK_PAIN2      }, // HS_CLINK_PAIN1
    { HSPR_CLNK, 7,                     3, A_Pain,                 HS_CLINK_WALK1      }, // HS_CLINK_PAIN2
    { HSPR_CLNK, 8,                     6, NULL,                   HS_CLINK_DIE2       }, // HS_CLINK_DIE1
    { HSPR_CLNK, 9,                     6, NULL,                   HS_CLINK_DIE3       }, // HS_CLINK_DIE2
    { HSPR_CLNK, 10,                    5, A_Scream,               HS_CLINK_DIE4       }, // HS_CLINK_DIE3
    { HSPR_CLNK, 11,                    5, A_NoBlocking,           HS_CLINK_DIE5       }, // HS_CLINK_DIE4
    { HSPR_CLNK, 12,                    5, NULL,                   HS_CLINK_DIE6       }, // HS_CLINK_DIE5
    { HSPR_CLNK, 13,                    5, NULL,                   HS_CLINK_DIE7       }, // HS_CLINK_DIE6
    { HSPR_CLNK, 14,                   -1, NULL,                   HS_NULL             }, // HS_CLINK_DIE7
    { HSPR_WZRD, 0,                    10, A_Look,                 HS_WIZARD_LOOK2     }, // HS_WIZARD_LOOK1
    { HSPR_WZRD, 1,                    10, A_Look,                 HS_WIZARD_LOOK1     }, // HS_WIZARD_LOOK2
    { HSPR_WZRD, 0,                     3, A_Chase,                HS_WIZARD_WALK2     }, // HS_WIZARD_WALK1
    { HSPR_WZRD, 0,                     4, A_Chase,                HS_WIZARD_WALK3     }, // HS_WIZARD_WALK2
    { HSPR_WZRD, 0,                     3, A_Chase,                HS_WIZARD_WALK4     }, // HS_WIZARD_WALK3
    { HSPR_WZRD, 0,                     4, A_Chase,                HS_WIZARD_WALK5     }, // HS_WIZARD_WALK4
    { HSPR_WZRD, 1,                     3, A_Chase,                HS_WIZARD_WALK6     }, // HS_WIZARD_WALK5
    { HSPR_WZRD, 1,                     4, A_Chase,                HS_WIZARD_WALK7     }, // HS_WIZARD_WALK6
    { HSPR_WZRD, 1,                     3, A_Chase,                HS_WIZARD_WALK8     }, // HS_WIZARD_WALK7
    { HSPR_WZRD, 1,                     4, A_Chase,                HS_WIZARD_WALK1     }, // HS_WIZARD_WALK8
    { HSPR_WZRD, 2,                     4, A_WizAtk1,              HS_WIZARD_ATK2      }, // HS_WIZARD_ATK1
    { HSPR_WZRD, 2,                     4, A_WizAtk2,              HS_WIZARD_ATK3      }, // HS_WIZARD_ATK2
    { HSPR_WZRD, 2,                     4, A_WizAtk1,              HS_WIZARD_ATK4      }, // HS_WIZARD_ATK3
    { HSPR_WZRD, 2,                     4, A_WizAtk2,              HS_WIZARD_ATK5      }, // HS_WIZARD_ATK4
    { HSPR_WZRD, 2,                     4, A_WizAtk1,              HS_WIZARD_ATK6      }, // HS_WIZARD_ATK5
    { HSPR_WZRD, 2,                     4, A_WizAtk2,              HS_WIZARD_ATK7      }, // HS_WIZARD_ATK6
    { HSPR_WZRD, 2,                     4, A_WizAtk1,              HS_WIZARD_ATK8      }, // HS_WIZARD_ATK7
    { HSPR_WZRD, 2,                     4, A_WizAtk2,              HS_WIZARD_ATK9      }, // HS_WIZARD_ATK8
    { HSPR_WZRD, 3,                    12, A_WizAtk3,              HS_WIZARD_WALK1     }, // HS_WIZARD_ATK9
    { HSPR_WZRD, 4,                     3, A_GhostOff,             HS_WIZARD_PAIN2     }, // HS_WIZARD_PAIN1
    { HSPR_WZRD, 4,                     3, A_Pain,                 HS_WIZARD_WALK1     }, // HS_WIZARD_PAIN2
    { HSPR_WZRD, 5,                     6, A_GhostOff,             HS_WIZARD_DIE2      }, // HS_WIZARD_DIE1
    { HSPR_WZRD, 6,                     6, A_Scream,               HS_WIZARD_DIE3      }, // HS_WIZARD_DIE2
    { HSPR_WZRD, 7,                     6, NULL,                   HS_WIZARD_DIE4      }, // HS_WIZARD_DIE3
    { HSPR_WZRD, 8,                     6, NULL,                   HS_WIZARD_DIE5      }, // HS_WIZARD_DIE4
    { HSPR_WZRD, 9,                     6, A_NoBlocking,           HS_WIZARD_DIE6      }, // HS_WIZARD_DIE5
    { HSPR_WZRD, 10,                    6, NULL,                   HS_WIZARD_DIE7      }, // HS_WIZARD_DIE6
    { HSPR_WZRD, 11,                    6, NULL,                   HS_WIZARD_DIE8      }, // HS_WIZARD_DIE7
    { HSPR_WZRD, 12,                   -1, NULL,                   HS_NULL             }, // HS_WIZARD_DIE8
    { HSPR_FX11, 0 | FF_FULLBRIGHT,     6, NULL,                   HS_WIZFX1_2         }, // HS_WIZFX1_1
    { HSPR_FX11, 1 | FF_FULLBRIGHT,     6, NULL,                   HS_WIZFX1_1         }, // HS_WIZFX1_2
    { HSPR_FX11, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_WIZFXI1_2        }, // HS_WIZFXI1_1
    { HSPR_FX11, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_WIZFXI1_3        }, // HS_WIZFXI1_2
    { HSPR_FX11, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_WIZFXI1_4        }, // HS_WIZFXI1_3
    { HSPR_FX11, 5 | FF_FULLBRIGHT,     5, NULL,                   HS_WIZFXI1_5        }, // HS_WIZFXI1_4
    { HSPR_FX11, 6 | FF_FULLBRIGHT,     5, NULL,                   HS_NULL             }, // HS_WIZFXI1_5
    { HSPR_IMPX, 0,                    10, A_Look,                 HS_IMP_LOOK2        }, // HS_IMP_LOOK1
    { HSPR_IMPX, 1,                    10, A_Look,                 HS_IMP_LOOK3        }, // HS_IMP_LOOK2
    { HSPR_IMPX, 2,                    10, A_Look,                 HS_IMP_LOOK4        }, // HS_IMP_LOOK3
    { HSPR_IMPX, 1,                    10, A_Look,                 HS_IMP_LOOK1        }, // HS_IMP_LOOK4
    { HSPR_IMPX, 0,                     3, A_Chase,                HS_IMP_FLY2         }, // HS_IMP_FLY1
    { HSPR_IMPX, 0,                     3, A_Chase,                HS_IMP_FLY3         }, // HS_IMP_FLY2
    { HSPR_IMPX, 1,                     3, A_Chase,                HS_IMP_FLY4         }, // HS_IMP_FLY3
    { HSPR_IMPX, 1,                     3, A_Chase,                HS_IMP_FLY5         }, // HS_IMP_FLY4
    { HSPR_IMPX, 2,                     3, A_Chase,                HS_IMP_FLY6         }, // HS_IMP_FLY5
    { HSPR_IMPX, 2,                     3, A_Chase,                HS_IMP_FLY7         }, // HS_IMP_FLY6
    { HSPR_IMPX, 1,                     3, A_Chase,                HS_IMP_FLY8         }, // HS_IMP_FLY7
    { HSPR_IMPX, 1,                     3, A_Chase,                HS_IMP_FLY1         }, // HS_IMP_FLY8
    { HSPR_IMPX, 3,                     6, A_FaceTarget,           HS_IMP_MEATK2       }, // HS_IMP_MEATK1
    { HSPR_IMPX, 4,                     6, A_FaceTarget,           HS_IMP_MEATK3       }, // HS_IMP_MEATK2
    { HSPR_IMPX, 5,                     6, A_ImpMeAttack,          HS_IMP_FLY1         }, // HS_IMP_MEATK3
    { HSPR_IMPX, 0,                    10, A_FaceTarget,           HS_IMP_MSATK1_2     }, // HS_IMP_MSATK1_1
    { HSPR_IMPX, 1,                     6, A_ImpMsAttack,          HS_IMP_MSATK1_3     }, // HS_IMP_MSATK1_2
    { HSPR_IMPX, 2,                     6, NULL,                   HS_IMP_MSATK1_4     }, // HS_IMP_MSATK1_3
    { HSPR_IMPX, 1,                     6, NULL,                   HS_IMP_MSATK1_5     }, // HS_IMP_MSATK1_4
    { HSPR_IMPX, 0,                     6, NULL,                   HS_IMP_MSATK1_6     }, // HS_IMP_MSATK1_5
    { HSPR_IMPX, 1,                     6, NULL,                   HS_IMP_MSATK1_3     }, // HS_IMP_MSATK1_6
    { HSPR_IMPX, 3,                     6, A_FaceTarget,           HS_IMP_MSATK2_2     }, // HS_IMP_MSATK2_1
    { HSPR_IMPX, 4,                     6, A_FaceTarget,           HS_IMP_MSATK2_3     }, // HS_IMP_MSATK2_2
    { HSPR_IMPX, 5,                     6, A_ImpMsAttack2,         HS_IMP_FLY1         }, // HS_IMP_MSATK2_3
    { HSPR_IMPX, 6,                     3, NULL,                   HS_IMP_PAIN2        }, // HS_IMP_PAIN1
    { HSPR_IMPX, 6,                     3, A_Pain,                 HS_IMP_FLY1         }, // HS_IMP_PAIN2
    { HSPR_IMPX, 6,                     4, A_ImpDeath,             HS_IMP_DIE2         }, // HS_IMP_DIE1
    { HSPR_IMPX, 7,                     5, NULL,                   HS_IMP_DIE2         }, // HS_IMP_DIE2
    { HSPR_IMPX, 18,                    5, A_ImpXDeath1,           HS_IMP_XDIE2        }, // HS_IMP_XDIE1
    { HSPR_IMPX, 19,                    5, NULL,                   HS_IMP_XDIE3        }, // HS_IMP_XDIE2
    { HSPR_IMPX, 20,                    5, NULL,                   HS_IMP_XDIE4        }, // HS_IMP_XDIE3
    { HSPR_IMPX, 21,                    5, A_ImpXDeath2,           HS_IMP_XDIE5        }, // HS_IMP_XDIE4
    { HSPR_IMPX, 22,                    5, NULL,                   HS_IMP_XDIE5        }, // HS_IMP_XDIE5
    { HSPR_IMPX, 8,                     7, A_ImpExplode,           HS_IMP_CRASH2       }, // HS_IMP_CRASH1
    { HSPR_IMPX, 9,                     7, A_Scream,               HS_IMP_CRASH3       }, // HS_IMP_CRASH2
    { HSPR_IMPX, 10,                    7, NULL,                   HS_IMP_CRASH4       }, // HS_IMP_CRASH3
    { HSPR_IMPX, 11,                   -1, NULL,                   HS_NULL             }, // HS_IMP_CRASH4
    { HSPR_IMPX, 23,                    7, NULL,                   HS_IMP_XCRASH2      }, // HS_IMP_XCRASH1
    { HSPR_IMPX, 24,                    7, NULL,                   HS_IMP_XCRASH3      }, // HS_IMP_XCRASH2
    { HSPR_IMPX, 25,                   -1, NULL,                   HS_NULL             }, // HS_IMP_XCRASH3
    { HSPR_IMPX, 12,                    5, NULL,                   HS_IMP_CHUNKA2      }, // HS_IMP_CHUNKA1
    { HSPR_IMPX, 13,                  700, NULL,                   HS_IMP_CHUNKA3      }, // HS_IMP_CHUNKA2
    { HSPR_IMPX, 14,                  700, NULL,                   HS_NULL             }, // HS_IMP_CHUNKA3
    { HSPR_IMPX, 15,                    5, NULL,                   HS_IMP_CHUNKB2      }, // HS_IMP_CHUNKB1
    { HSPR_IMPX, 16,                  700, NULL,                   HS_IMP_CHUNKB3      }, // HS_IMP_CHUNKB2
    { HSPR_IMPX, 17,                  700, NULL,                   HS_NULL             }, // HS_IMP_CHUNKB3
    { HSPR_FX10, 0 | FF_FULLBRIGHT,     6, NULL,                   HS_IMPFX2           }, // HS_IMPFX1
    { HSPR_FX10, 1 | FF_FULLBRIGHT,     6, NULL,                   HS_IMPFX3           }, // HS_IMPFX2
    { HSPR_FX10, 2 | FF_FULLBRIGHT,     6, NULL,                   HS_IMPFX1           }, // HS_IMPFX3
    { HSPR_FX10, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_IMPFXI2          }, // HS_IMPFXI1
    { HSPR_FX10, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_IMPFXI3          }, // HS_IMPFXI2
    { HSPR_FX10, 5 | FF_FULLBRIGHT,     5, NULL,                   HS_IMPFXI4          }, // HS_IMPFXI3
    { HSPR_FX10, 6 | FF_FULLBRIGHT,     5, NULL,                   HS_NULL             }, // HS_IMPFXI4
    { HSPR_KNIG, 0,                    10, A_Look,                 HS_KNIGHT_STND2     }, // HS_KNIGHT_STND1
    { HSPR_KNIG, 1,                    10, A_Look,                 HS_KNIGHT_STND1     }, // HS_KNIGHT_STND2
    { HSPR_KNIG, 0,                     4, A_Chase,                HS_KNIGHT_WALK2     }, // HS_KNIGHT_WALK1
    { HSPR_KNIG, 1,                     4, A_Chase,                HS_KNIGHT_WALK3     }, // HS_KNIGHT_WALK2
    { HSPR_KNIG, 2,                     4, A_Chase,                HS_KNIGHT_WALK4     }, // HS_KNIGHT_WALK3
    { HSPR_KNIG, 3,                     4, A_Chase,                HS_KNIGHT_WALK1     }, // HS_KNIGHT_WALK4
    { HSPR_KNIG, 4,                    10, A_FaceTarget,           HS_KNIGHT_ATK2      }, // HS_KNIGHT_ATK1
    { HSPR_KNIG, 5,                     8, A_FaceTarget,           HS_KNIGHT_ATK3      }, // HS_KNIGHT_ATK2
    { HSPR_KNIG, 6,                     8, A_KnightAttack,         HS_KNIGHT_ATK4      }, // HS_KNIGHT_ATK3
    { HSPR_KNIG, 4,                    10, A_FaceTarget,           HS_KNIGHT_ATK5      }, // HS_KNIGHT_ATK4
    { HSPR_KNIG, 5,                     8, A_FaceTarget,           HS_KNIGHT_ATK6      }, // HS_KNIGHT_ATK5
    { HSPR_KNIG, 6,                     8, A_KnightAttack,         HS_KNIGHT_WALK1     }, // HS_KNIGHT_ATK6
    { HSPR_KNIG, 7,                     3, NULL,                   HS_KNIGHT_PAIN2     }, // HS_KNIGHT_PAIN1
    { HSPR_KNIG, 7,                     3, A_Pain,                 HS_KNIGHT_WALK1     }, // HS_KNIGHT_PAIN2
    { HSPR_KNIG, 8,                     6, NULL,                   HS_KNIGHT_DIE2      }, // HS_KNIGHT_DIE1
    { HSPR_KNIG, 9,                     6, A_Scream,               HS_KNIGHT_DIE3      }, // HS_KNIGHT_DIE2
    { HSPR_KNIG, 10,                    6, NULL,                   HS_KNIGHT_DIE4      }, // HS_KNIGHT_DIE3
    { HSPR_KNIG, 11,                    6, A_NoBlocking,           HS_KNIGHT_DIE5      }, // HS_KNIGHT_DIE4
    { HSPR_KNIG, 12,                    6, NULL,                   HS_KNIGHT_DIE6      }, // HS_KNIGHT_DIE5
    { HSPR_KNIG, 13,                    6, NULL,                   HS_KNIGHT_DIE7      }, // HS_KNIGHT_DIE6
    { HSPR_KNIG, 14,                   -1, NULL,                   HS_NULL             }, // HS_KNIGHT_DIE7
    { HSPR_SPAX, 0 | FF_FULLBRIGHT,     3, A_ContMobjSound,        HS_SPINAXE2         }, // HS_SPINAXE1
    { HSPR_SPAX, 1 | FF_FULLBRIGHT,     3, NULL,                   HS_SPINAXE3         }, // HS_SPINAXE2
    { HSPR_SPAX, 2 | FF_FULLBRIGHT,     3, NULL,                   HS_SPINAXE1         }, // HS_SPINAXE3
    { HSPR_SPAX, 3 | FF_FULLBRIGHT,     6, NULL,                   HS_SPINAXEX2        }, // HS_SPINAXEX1
    { HSPR_SPAX, 4 | FF_FULLBRIGHT,     6, NULL,                   HS_SPINAXEX3        }, // HS_SPINAXEX2
    { HSPR_SPAX, 5 | FF_FULLBRIGHT,     6, NULL,                   HS_NULL             }, // HS_SPINAXEX3
    { HSPR_RAXE, 0 | FF_FULLBRIGHT,     5, A_DripBlood,            HS_REDAXE2          }, // HS_REDAXE1
    { HSPR_RAXE, 1 | FF_FULLBRIGHT,     5, A_DripBlood,            HS_REDAXE1          }, // HS_REDAXE2
    { HSPR_RAXE, 2 | FF_FULLBRIGHT,     6, NULL,                   HS_REDAXEX2         }, // HS_REDAXEX1
    { HSPR_RAXE, 3 | FF_FULLBRIGHT,     6, NULL,                   HS_REDAXEX3         }, // HS_REDAXEX2
    { HSPR_RAXE, 4 | FF_FULLBRIGHT,     6, NULL,                   HS_NULL             }, // HS_REDAXEX3
    { HSPR_SRCR, 0,                    10, A_Look,                 HS_SRCR1_LOOK2      }, // HS_SRCR1_LOOK1
    { HSPR_SRCR, 1,                    10, A_Look,                 HS_SRCR1_LOOK1      }, // HS_SRCR1_LOOK2
    { HSPR_SRCR, 0,                     5, A_Sor1Chase,            HS_SRCR1_WALK2      }, // HS_SRCR1_WALK1
    { HSPR_SRCR, 1,                     5, A_Sor1Chase,            HS_SRCR1_WALK3      }, // HS_SRCR1_WALK2
    { HSPR_SRCR, 2,                     5, A_Sor1Chase,            HS_SRCR1_WALK4      }, // HS_SRCR1_WALK3
    { HSPR_SRCR, 3,                     5, A_Sor1Chase,            HS_SRCR1_WALK1      }, // HS_SRCR1_WALK4
    { HSPR_SRCR, 16,                    6, A_Sor1Pain,             HS_SRCR1_WALK1      }, // HS_SRCR1_PAIN1
    { HSPR_SRCR, 16,                    7, A_FaceTarget,           HS_SRCR1_ATK2       }, // HS_SRCR1_ATK1
    { HSPR_SRCR, 17,                    6, A_FaceTarget,           HS_SRCR1_ATK3       }, // HS_SRCR1_ATK2
    { HSPR_SRCR, 18,                   10, A_Srcr1Attack,          HS_SRCR1_WALK1      }, // HS_SRCR1_ATK3
    { HSPR_SRCR, 18,                   10, A_FaceTarget,           HS_SRCR1_ATK5       }, // HS_SRCR1_ATK4
    { HSPR_SRCR, 16,                    7, A_FaceTarget,           HS_SRCR1_ATK6       }, // HS_SRCR1_ATK5
    { HSPR_SRCR, 17,                    6, A_FaceTarget,           HS_SRCR1_ATK7       }, // HS_SRCR1_ATK6
    { HSPR_SRCR, 18,                   10, A_Srcr1Attack,          HS_SRCR1_WALK1      }, // HS_SRCR1_ATK7
    { HSPR_SRCR, 4,                     7, NULL,                   HS_SRCR1_DIE2       }, // HS_SRCR1_DIE1
    { HSPR_SRCR, 5,                     7, A_Scream,               HS_SRCR1_DIE3       }, // HS_SRCR1_DIE2
    { HSPR_SRCR, 6,                     7, NULL,                   HS_SRCR1_DIE4       }, // HS_SRCR1_DIE3
    { HSPR_SRCR, 7,                     6, NULL,                   HS_SRCR1_DIE5       }, // HS_SRCR1_DIE4
    { HSPR_SRCR, 8,                     6, NULL,                   HS_SRCR1_DIE6       }, // HS_SRCR1_DIE5
    { HSPR_SRCR, 9,                     6, NULL,                   HS_SRCR1_DIE7       }, // HS_SRCR1_DIE6
    { HSPR_SRCR, 10,                    6, NULL,                   HS_SRCR1_DIE8       }, // HS_SRCR1_DIE7
    { HSPR_SRCR, 11,                   25, A_SorZap,               HS_SRCR1_DIE9       }, // HS_SRCR1_DIE8
    { HSPR_SRCR, 12,                    5, NULL,                   HS_SRCR1_DIE10      }, // HS_SRCR1_DIE9
    { HSPR_SRCR, 13,                    5, NULL,                   HS_SRCR1_DIE11      }, // HS_SRCR1_DIE10
    { HSPR_SRCR, 14,                    4, NULL,                   HS_SRCR1_DIE12      }, // HS_SRCR1_DIE11
    { HSPR_SRCR, 11,                   20, A_SorZap,               HS_SRCR1_DIE13      }, // HS_SRCR1_DIE12
    { HSPR_SRCR, 12,                    5, NULL,                   HS_SRCR1_DIE14      }, // HS_SRCR1_DIE13
    { HSPR_SRCR, 13,                    5, NULL,                   HS_SRCR1_DIE15      }, // HS_SRCR1_DIE14
    { HSPR_SRCR, 14,                    4, NULL,                   HS_SRCR1_DIE16      }, // HS_SRCR1_DIE15
    { HSPR_SRCR, 11,                   12, NULL,                   HS_SRCR1_DIE17      }, // HS_SRCR1_DIE16
    { HSPR_SRCR, 15,                   -1, A_SorcererRise,         HS_NULL             }, // HS_SRCR1_DIE17
    { HSPR_FX14, 0 | FF_FULLBRIGHT,     6, NULL,                   HS_SRCRFX1_2        }, // HS_SRCRFX1_1
    { HSPR_FX14, 1 | FF_FULLBRIGHT,     6, NULL,                   HS_SRCRFX1_3        }, // HS_SRCRFX1_2
    { HSPR_FX14, 2 | FF_FULLBRIGHT,     6, NULL,                   HS_SRCRFX1_1        }, // HS_SRCRFX1_3
    { HSPR_FX14, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_SRCRFXI1_2       }, // HS_SRCRFXI1_1
    { HSPR_FX14, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_SRCRFXI1_3       }, // HS_SRCRFXI1_2
    { HSPR_FX14, 5 | FF_FULLBRIGHT,     5, NULL,                   HS_SRCRFXI1_4       }, // HS_SRCRFXI1_3
    { HSPR_FX14, 6 | FF_FULLBRIGHT,     5, NULL,                   HS_SRCRFXI1_5       }, // HS_SRCRFXI1_4
    { HSPR_FX14, 7 | FF_FULLBRIGHT,     5, NULL,                   HS_NULL             }, // HS_SRCRFXI1_5
    { HSPR_SOR2, 0,                     4, NULL,                   HS_SOR2_RISE2       }, // HS_SOR2_RISE1
    { HSPR_SOR2, 1,                     4, NULL,                   HS_SOR2_RISE3       }, // HS_SOR2_RISE2
    { HSPR_SOR2, 2,                     4, A_SorRise,              HS_SOR2_RISE4       }, // HS_SOR2_RISE3
    { HSPR_SOR2, 3,                     4, NULL,                   HS_SOR2_RISE5       }, // HS_SOR2_RISE4
    { HSPR_SOR2, 4,                     4, NULL,                   HS_SOR2_RISE6       }, // HS_SOR2_RISE5
    { HSPR_SOR2, 5,                     4, NULL,                   HS_SOR2_RISE7       }, // HS_SOR2_RISE6
    { HSPR_SOR2, 6,                    12, A_SorSightSnd,          HS_SOR2_WALK1       }, // HS_SOR2_RISE7
    { HSPR_SOR2, 12,                   10, A_Look,                 HS_SOR2_LOOK2       }, // HS_SOR2_LOOK1
    { HSPR_SOR2, 13,                   10, A_Look,                 HS_SOR2_LOOK1       }, // HS_SOR2_LOOK2
    { HSPR_SOR2, 12,                    4, A_Chase,                HS_SOR2_WALK2       }, // HS_SOR2_WALK1
    { HSPR_SOR2, 13,                    4, A_Chase,                HS_SOR2_WALK3       }, // HS_SOR2_WALK2
    { HSPR_SOR2, 14,                    4, A_Chase,                HS_SOR2_WALK4       }, // HS_SOR2_WALK3
    { HSPR_SOR2, 15,                    4, A_Chase,                HS_SOR2_WALK1       }, // HS_SOR2_WALK4
    { HSPR_SOR2, 16,                    3, NULL,                   HS_SOR2_PAIN2       }, // HS_SOR2_PAIN1
    { HSPR_SOR2, 16,                    6, A_Pain,                 HS_SOR2_WALK1       }, // HS_SOR2_PAIN2
    { HSPR_SOR2, 17,                    9, A_Srcr2Decide,          HS_SOR2_ATK2        }, // HS_SOR2_ATK1
    { HSPR_SOR2, 18,                    9, A_FaceTarget,           HS_SOR2_ATK3        }, // HS_SOR2_ATK2
    { HSPR_SOR2, 19,                   20, A_Srcr2Attack,          HS_SOR2_WALK1       }, // HS_SOR2_ATK3
    { HSPR_SOR2, 11,                    6, NULL,                   HS_SOR2_TELE2       }, // HS_SOR2_TELE1
    { HSPR_SOR2, 10,                    6, NULL,                   HS_SOR2_TELE3       }, // HS_SOR2_TELE2
    { HSPR_SOR2, 9,                     6, NULL,                   HS_SOR2_TELE4       }, // HS_SOR2_TELE3
    { HSPR_SOR2, 8,                     6, NULL,                   HS_SOR2_TELE5       }, // HS_SOR2_TELE4
    { HSPR_SOR2, 7,                     6, NULL,                   HS_SOR2_TELE6       }, // HS_SOR2_TELE5
    { HSPR_SOR2, 6,                     6, NULL,                   HS_SOR2_WALK1       }, // HS_SOR2_TELE6
    { HSPR_SDTH, 0,                     8, A_Sor2DthInit,          HS_SOR2_DIE2        }, // HS_SOR2_DIE1
    { HSPR_SDTH, 1,                     8, NULL,                   HS_SOR2_DIE3        }, // HS_SOR2_DIE2
    { HSPR_SDTH, 2,                     8, A_SorDSph,              HS_SOR2_DIE4        }, // HS_SOR2_DIE3
    { HSPR_SDTH, 3,                     7, NULL,                   HS_SOR2_DIE5        }, // HS_SOR2_DIE4
    { HSPR_SDTH, 4,                     7, NULL,                   HS_SOR2_DIE6        }, // HS_SOR2_DIE5
    { HSPR_SDTH, 5,                     7, A_Sor2DthLoop,          HS_SOR2_DIE7        }, // HS_SOR2_DIE6
    { HSPR_SDTH, 6,                     6, A_SorDExp,              HS_SOR2_DIE8        }, // HS_SOR2_DIE7
    { HSPR_SDTH, 7,                     6, NULL,                   HS_SOR2_DIE9        }, // HS_SOR2_DIE8
    { HSPR_SDTH, 8,                    18, NULL,                   HS_SOR2_DIE10       }, // HS_SOR2_DIE9
    { HSPR_SDTH, 9,                     6, A_NoBlocking,           HS_SOR2_DIE11       }, // HS_SOR2_DIE10
    { HSPR_SDTH, 10,                    6, A_SorDBon,              HS_SOR2_DIE12       }, // HS_SOR2_DIE11
    { HSPR_SDTH, 11,                    6, NULL,                   HS_SOR2_DIE13       }, // HS_SOR2_DIE12
    { HSPR_SDTH, 12,                    6, NULL,                   HS_SOR2_DIE14       }, // HS_SOR2_DIE13
    { HSPR_SDTH, 13,                    6, NULL,                   HS_SOR2_DIE15       }, // HS_SOR2_DIE14
    { HSPR_SDTH, 14,                   -1, A_BossDeath2,           HS_NULL             }, // HS_SOR2_DIE15
    { HSPR_FX16, 0 | FF_FULLBRIGHT,     3, A_BlueSpark,            HS_SOR2FX1_2        }, // HS_SOR2FX1_1
    { HSPR_FX16, 1 | FF_FULLBRIGHT,     3, A_BlueSpark,            HS_SOR2FX1_3        }, // HS_SOR2FX1_2
    { HSPR_FX16, 2 | FF_FULLBRIGHT,     3, A_BlueSpark,            HS_SOR2FX1_1        }, // HS_SOR2FX1_3
    { HSPR_FX16, 6 | FF_FULLBRIGHT,     5, A_Explode,              HS_SOR2FXI1_2       }, // HS_SOR2FXI1_1
    { HSPR_FX16, 7 | FF_FULLBRIGHT,     5, NULL,                   HS_SOR2FXI1_3       }, // HS_SOR2FXI1_2
    { HSPR_FX16, 8 | FF_FULLBRIGHT,     5, NULL,                   HS_SOR2FXI1_4       }, // HS_SOR2FXI1_3
    { HSPR_FX16, 9 | FF_FULLBRIGHT,     5, NULL,                   HS_SOR2FXI1_5       }, // HS_SOR2FXI1_4
    { HSPR_FX16, 10 | FF_FULLBRIGHT,    5, NULL,                   HS_SOR2FXI1_6       }, // HS_SOR2FXI1_5
    { HSPR_FX16, 11 | FF_FULLBRIGHT,    5, NULL,                   HS_NULL             }, // HS_SOR2FXI1_6
    { HSPR_FX16, 3 | FF_FULLBRIGHT,    12, NULL,                   HS_SOR2FXSPARK2     }, // HS_SOR2FXSPARK1
    { HSPR_FX16, 4 | FF_FULLBRIGHT,    12, NULL,                   HS_SOR2FXSPARK3     }, // HS_SOR2FXSPARK2
    { HSPR_FX16, 5 | FF_FULLBRIGHT,    12, NULL,                   HS_NULL             }, // HS_SOR2FXSPARK3
    { HSPR_FX11, 0 | FF_FULLBRIGHT,    35, NULL,                   HS_SOR2FX2_2        }, // HS_SOR2FX2_1
    { HSPR_FX11, 0 | FF_FULLBRIGHT,     5, A_GenWizard,            HS_SOR2FX2_3        }, // HS_SOR2FX2_2
    { HSPR_FX11, 1 | FF_FULLBRIGHT,     5, NULL,                   HS_SOR2FX2_2        }, // HS_SOR2FX2_3
    { HSPR_FX11, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_SOR2FXI2_2       }, // HS_SOR2FXI2_1
    { HSPR_FX11, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_SOR2FXI2_3       }, // HS_SOR2FXI2_2
    { HSPR_FX11, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_SOR2FXI2_4       }, // HS_SOR2FXI2_3
    { HSPR_FX11, 5 | FF_FULLBRIGHT,     5, NULL,                   HS_SOR2FXI2_5       }, // HS_SOR2FXI2_4
    { HSPR_FX11, 6 | FF_FULLBRIGHT,     5, NULL,                   HS_NULL             }, // HS_SOR2FXI2_5
    { HSPR_SOR2, 6,                     8, NULL,                   HS_SOR2TELEFADE2    }, // HS_SOR2TELEFADE1
    { HSPR_SOR2, 7,                     6, NULL,                   HS_SOR2TELEFADE3    }, // HS_SOR2TELEFADE2
    { HSPR_SOR2, 8,                     6, NULL,                   HS_SOR2TELEFADE4    }, // HS_SOR2TELEFADE3
    { HSPR_SOR2, 9,                     6, NULL,                   HS_SOR2TELEFADE5    }, // HS_SOR2TELEFADE4
    { HSPR_SOR2, 10,                    6, NULL,                   HS_SOR2TELEFADE6    }, // HS_SOR2TELEFADE5
    { HSPR_SOR2, 11,                    6, NULL,                   HS_NULL             }, // HS_SOR2TELEFADE6
    { HSPR_MNTR, 0,                    10, A_Look,                 HS_MNTR_LOOK2       }, // HS_MNTR_LOOK1
    { HSPR_MNTR, 1,                    10, A_Look,                 HS_MNTR_LOOK1       }, // HS_MNTR_LOOK2
    { HSPR_MNTR, 0,                     5, A_Chase,                HS_MNTR_WALK2       }, // HS_MNTR_WALK1
    { HSPR_MNTR, 1,                     5, A_Chase,                HS_MNTR_WALK3       }, // HS_MNTR_WALK2
    { HSPR_MNTR, 2,                     5, A_Chase,                HS_MNTR_WALK4       }, // HS_MNTR_WALK3
    { HSPR_MNTR, 3,                     5, A_Chase,                HS_MNTR_WALK1       }, // HS_MNTR_WALK4
    { HSPR_MNTR, 21,                   10, A_FaceTarget,           HS_MNTR_ATK1_2      }, // HS_MNTR_ATK1_1
    { HSPR_MNTR, 22,                    7, A_FaceTarget,           HS_MNTR_ATK1_3      }, // HS_MNTR_ATK1_2
    { HSPR_MNTR, 23,                   12, A_MinotaurAtk1,         HS_MNTR_WALK1       }, // HS_MNTR_ATK1_3
    { HSPR_MNTR, 21,                   10, A_MinotaurDecide,       HS_MNTR_ATK2_2      }, // HS_MNTR_ATK2_1
    { HSPR_MNTR, 24,                    4, A_FaceTarget,           HS_MNTR_ATK2_3      }, // HS_MNTR_ATK2_2
    { HSPR_MNTR, 25,                    9, A_MinotaurAtk2,         HS_MNTR_WALK1       }, // HS_MNTR_ATK2_3
    { HSPR_MNTR, 21,                   10, A_FaceTarget,           HS_MNTR_ATK3_2      }, // HS_MNTR_ATK3_1
    { HSPR_MNTR, 22,                    7, A_FaceTarget,           HS_MNTR_ATK3_3      }, // HS_MNTR_ATK3_2
    { HSPR_MNTR, 23,                   12, A_MinotaurAtk3,         HS_MNTR_WALK1       }, // HS_MNTR_ATK3_3
    { HSPR_MNTR, 23,                   12, NULL,                   HS_MNTR_ATK3_1      }, // HS_MNTR_ATK3_4
    { HSPR_MNTR, 20,                    2, A_MinotaurCharge,       HS_MNTR_ATK4_1      }, // HS_MNTR_ATK4_1
    { HSPR_MNTR, 4,                     3, NULL,                   HS_MNTR_PAIN2       }, // HS_MNTR_PAIN1
    { HSPR_MNTR, 4,                     6, A_Pain,                 HS_MNTR_WALK1       }, // HS_MNTR_PAIN2
    { HSPR_MNTR, 5,                     6, NULL,                   HS_MNTR_DIE2        }, // HS_MNTR_DIE1
    { HSPR_MNTR, 6,                     5, NULL,                   HS_MNTR_DIE3        }, // HS_MNTR_DIE2
    { HSPR_MNTR, 7,                     6, A_Scream,               HS_MNTR_DIE4        }, // HS_MNTR_DIE3
    { HSPR_MNTR, 8,                     5, NULL,                   HS_MNTR_DIE5        }, // HS_MNTR_DIE4
    { HSPR_MNTR, 9,                     6, NULL,                   HS_MNTR_DIE6        }, // HS_MNTR_DIE5
    { HSPR_MNTR, 10,                    5, NULL,                   HS_MNTR_DIE7        }, // HS_MNTR_DIE6
    { HSPR_MNTR, 11,                    6, NULL,                   HS_MNTR_DIE8        }, // HS_MNTR_DIE7
    { HSPR_MNTR, 12,                    5, A_NoBlocking,           HS_MNTR_DIE9        }, // HS_MNTR_DIE8
    { HSPR_MNTR, 13,                    6, NULL,                   HS_MNTR_DIE10       }, // HS_MNTR_DIE9
    { HSPR_MNTR, 14,                    5, NULL,                   HS_MNTR_DIE11       }, // HS_MNTR_DIE10
    { HSPR_MNTR, 15,                    6, NULL,                   HS_MNTR_DIE12       }, // HS_MNTR_DIE11
    { HSPR_MNTR, 16,                    5, NULL,                   HS_MNTR_DIE13       }, // HS_MNTR_DIE12
    { HSPR_MNTR, 17,                    6, NULL,                   HS_MNTR_DIE14       }, // HS_MNTR_DIE13
    { HSPR_MNTR, 18,                    5, NULL,                   HS_MNTR_DIE15       }, // HS_MNTR_DIE14
    { HSPR_MNTR, 19,                   -1, A_BossDeath2,           HS_NULL             }, // HS_MNTR_DIE15
    { HSPR_FX12, 0 | FF_FULLBRIGHT,     6, NULL,                   HS_MNTRFX1_2        }, // HS_MNTRFX1_1
    { HSPR_FX12, 1 | FF_FULLBRIGHT,     6, NULL,                   HS_MNTRFX1_1        }, // HS_MNTRFX1_2
    { HSPR_FX12, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFXI1_2       }, // HS_MNTRFXI1_1
    { HSPR_FX12, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFXI1_3       }, // HS_MNTRFXI1_2
    { HSPR_FX12, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFXI1_4       }, // HS_MNTRFXI1_3
    { HSPR_FX12, 5 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFXI1_5       }, // HS_MNTRFXI1_4
    { HSPR_FX12, 6 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFXI1_6       }, // HS_MNTRFXI1_5
    { HSPR_FX12, 7 | FF_FULLBRIGHT,     5, NULL,                   HS_NULL             }, // HS_MNTRFXI1_6
    { HSPR_FX13, 0,                     2, A_MntrFloorFire,        HS_MNTRFX2_1        }, // HS_MNTRFX2_1
    { HSPR_FX13, 8 | FF_FULLBRIGHT,     4, A_Explode,              HS_MNTRFXI2_2       }, // HS_MNTRFXI2_1
    { HSPR_FX13, 9 | FF_FULLBRIGHT,     4, NULL,                   HS_MNTRFXI2_3       }, // HS_MNTRFXI2_2
    { HSPR_FX13, 10 | FF_FULLBRIGHT,    4, NULL,                   HS_MNTRFXI2_4       }, // HS_MNTRFXI2_3
    { HSPR_FX13, 11 | FF_FULLBRIGHT,    4, NULL,                   HS_MNTRFXI2_5       }, // HS_MNTRFXI2_4
    { HSPR_FX13, 12 | FF_FULLBRIGHT,    4, NULL,                   HS_NULL             }, // HS_MNTRFXI2_5
    { HSPR_FX13, 3 | FF_FULLBRIGHT,     4, NULL,                   HS_MNTRFX3_2        }, // HS_MNTRFX3_1
    { HSPR_FX13, 2 | FF_FULLBRIGHT,     4, NULL,                   HS_MNTRFX3_3        }, // HS_MNTRFX3_2
    { HSPR_FX13, 1 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFX3_4        }, // HS_MNTRFX3_3
    { HSPR_FX13, 2 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFX3_5        }, // HS_MNTRFX3_4
    { HSPR_FX13, 3 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFX3_6        }, // HS_MNTRFX3_5
    { HSPR_FX13, 4 | FF_FULLBRIGHT,     5, NULL,                   HS_MNTRFX3_7        }, // HS_MNTRFX3_6
    { HSPR_FX13, 5 | FF_FULLBRIGHT,     4, NULL,                   HS_MNTRFX3_8        }, // HS_MNTRFX3_7
    { HSPR_FX13, 6 | FF_FULLBRIGHT,     4, NULL,                   HS_MNTRFX3_9        }, // HS_MNTRFX3_8
    { HSPR_FX13, 7 | FF_FULLBRIGHT,     4, NULL,                   HS_NULL             }, // HS_MNTRFX3_9
    { HSPR_AKYY, 0 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY2            }, // HS_AKYY1
    { HSPR_AKYY, 1 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY3            }, // HS_AKYY2
    { HSPR_AKYY, 2 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY4            }, // HS_AKYY3
    { HSPR_AKYY, 3 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY5            }, // HS_AKYY4
    { HSPR_AKYY, 4 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY6            }, // HS_AKYY5
    { HSPR_AKYY, 5 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY7            }, // HS_AKYY6
    { HSPR_AKYY, 6 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY8            }, // HS_AKYY7
    { HSPR_AKYY, 7 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY9            }, // HS_AKYY8
    { HSPR_AKYY, 8 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY10           }, // HS_AKYY9
    { HSPR_AKYY, 9 | FF_FULLBRIGHT,     3, NULL,                   HS_AKYY1            }, // HS_AKYY10
    { HSPR_BKYY, 0 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY2            }, // HS_BKYY1
    { HSPR_BKYY, 1 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY3            }, // HS_BKYY2
    { HSPR_BKYY, 2 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY4            }, // HS_BKYY3
    { HSPR_BKYY, 3 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY5            }, // HS_BKYY4
    { HSPR_BKYY, 4 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY6            }, // HS_BKYY5
    { HSPR_BKYY, 5 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY7            }, // HS_BKYY6
    { HSPR_BKYY, 6 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY8            }, // HS_BKYY7
    { HSPR_BKYY, 7 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY9            }, // HS_BKYY8
    { HSPR_BKYY, 8 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY10           }, // HS_BKYY9
    { HSPR_BKYY, 9 | FF_FULLBRIGHT,     3, NULL,                   HS_BKYY1            }, // HS_BKYY10
    { HSPR_CKYY, 0 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY2            }, // HS_CKYY1
    { HSPR_CKYY, 1 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY3            }, // HS_CKYY2
    { HSPR_CKYY, 2 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY4            }, // HS_CKYY3
    { HSPR_CKYY, 3 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY5            }, // HS_CKYY4
    { HSPR_CKYY, 4 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY6            }, // HS_CKYY5
    { HSPR_CKYY, 5 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY7            }, // HS_CKYY6
    { HSPR_CKYY, 6 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY8            }, // HS_CKYY7
    { HSPR_CKYY, 7 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY9            }, // HS_CKYY8
    { HSPR_CKYY, 8 | FF_FULLBRIGHT,     3, NULL,                   HS_CKYY1            }, // HS_CKYY9
    { HSPR_AMG1, 0,                    -1, NULL,                   HS_NULL             }, // HS_AMG1
    { HSPR_AMG2, 0,                     4, NULL,                   HS_AMG2_2           }, // HS_AMG2_1
    { HSPR_AMG2, 1,                     4, NULL,                   HS_AMG2_3           }, // HS_AMG2_2
    { HSPR_AMG2, 2,                     4, NULL,                   HS_AMG2_1           }, // HS_AMG2_3
    { HSPR_AMM1, 0,                    -1, NULL,                   HS_NULL             }, // HS_AMM1
    { HSPR_AMM2, 0,                    -1, NULL,                   HS_NULL             }, // HS_AMM2
    { HSPR_AMC1, 0,                    -1, NULL,                   HS_NULL             }, // HS_AMC1
    { HSPR_AMC2, 0,                     5, NULL,                   HS_AMC2_2           }, // HS_AMC2_1
    { HSPR_AMC2, 1,                     5, NULL,                   HS_AMC2_3           }, // HS_AMC2_2
    { HSPR_AMC2, 2,                     5, NULL,                   HS_AMC2_1           }, // HS_AMC2_3
    { HSPR_AMS1, 0,                     5, NULL,                   HS_AMS1_2           }, // HS_AMS1_1
    { HSPR_AMS1, 1,                     5, NULL,                   HS_AMS1_1           }, // HS_AMS1_2
    { HSPR_AMS2, 0,                     5, NULL,                   HS_AMS2_2           }, // HS_AMS2_1
    { HSPR_AMS2, 1,                     5, NULL,                   HS_AMS2_1           }, // HS_AMS2_2
    { HSPR_AMP1, 0,                     4, NULL,                   HS_AMP1_2           }, // HS_AMP1_1
    { HSPR_AMP1, 1,                     4, NULL,                   HS_AMP1_3           }, // HS_AMP1_2
    { HSPR_AMP1, 2,                     4, NULL,                   HS_AMP1_1           }, // HS_AMP1_3
    { HSPR_AMP2, 0,                     4, NULL,                   HS_AMP2_2           }, // HS_AMP2_1
    { HSPR_AMP2, 1,                     4, NULL,                   HS_AMP2_3           }, // HS_AMP2_2
    { HSPR_AMP2, 2,                     4, NULL,                   HS_AMP2_1           }, // HS_AMP2_3
    { HSPR_AMB1, 0,                     4, NULL,                   HS_AMB1_2           }, // HS_AMB1_1
    { HSPR_AMB1, 1,                     4, NULL,                   HS_AMB1_3           }, // HS_AMB1_2
    { HSPR_AMB1, 2,                     4, NULL,                   HS_AMB1_1           }, // HS_AMB1_3
    { HSPR_AMB2, 0,                     4, NULL,                   HS_AMB2_2           }, // HS_AMB2_1
    { HSPR_AMB2, 1,                     4, NULL,                   HS_AMB2_3           }, // HS_AMB2_2
    { HSPR_AMB2, 2,                     4, NULL,                   HS_AMB2_1           }, // HS_AMB2_3
    { HSPR_AMG1, 0,                   100, A_ESound,               HS_SND_WIND         }, // HS_SND_WIND
    { HSPR_AMG1, 0,                    85, A_ESound,               HS_SND_WATERFALL    }  // HS_SND_WATERFALL
};

mobjinfo_t hereticmobjinfo[] =
{
    // HMT_MISC0
    {
        /* doomednum            */ 81,
        /* spawnstate           */ HS_ITEM_PTN1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_ITEMSHIELD1
    {
        /* doomednum            */ 85,
        /* spawnstate           */ HS_ITEM_SHLD1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
     },

    // HMT_ITEMSHIELD2
    {
        /* doomednum            */ 31,
        /* spawnstate           */ HS_ITEM_SHD2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC1
    {
        /* doomednum            */ 8,
        /* spawnstate           */ HS_ITEM_BAGH1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC2
    {
        /* doomednum            */ 35,
        /* spawnstate           */ HS_ITEM_SPMP1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_ARTIINVISIBILITY
    {
        /* doomednum            */ 75,
        /* spawnstate           */ HS_ARTI_INVS1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | /*MF_SHADOW | */MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC3
    {
        /* doomednum            */ 82,
        /* spawnstate           */ HS_ARTI_PTN2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_ARTIFLY
    {
        /* doomednum            */ 83,
        /* spawnstate           */ HS_ARTI_SOAR1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_ARTIINVULNERABILITY
    {
        /* doomednum            */ 84,
        /* spawnstate           */ HS_ARTI_INVU1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_ARTITOMEOFPOWER
    {
        /* doomednum            */ 86,
        /* spawnstate           */ HS_ARTI_PWBK1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_ARTIEGG
    {
        /* doomednum            */ 30,
        /* spawnstate           */ HS_ARTI_EGGC1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_EGGFX
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_EGGFX1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_EGGFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 18 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_ARTISUPERHEAL
    {
        /* doomednum            */ 32,
        /* spawnstate           */ HS_ARTI_SPHL1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC4
    {
        /* doomednum            */ 33,
        /* spawnstate           */ HS_ARTI_TRCH1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC5
    {
        /* doomednum            */ 34,
        /* spawnstate           */ HS_ARTI_FBMB1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_FIREBOMB
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_FIREBOMB1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_phohit,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOGRAVITY/* | MF_SHADOW*/,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_ARTITELEPORT
    {
        /* doomednum            */ 36,
        /* spawnstate           */ HS_ARTI_ATLP1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_COUNTITEM,
        /* flags2               */ MF2_FLOATBOB,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_POD
    {
        /* doomednum            */ 2035,
        /* spawnstate           */ HS_POD_WAIT1,
        /* spawnhealth          */ 45,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_POD_PAIN1,
        /* painchance           */ 255,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_POD_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_podexp,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 54 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID | MF_NOBLOOD | MF_SHOOTABLE | MF_DROPOFF,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ MF3_WINDTHRUST | MF3_PUSHABLE | MF3_SLIDE | MF3_TELESTOMP,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_PODGOO
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_PODGOO1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_PODGOOX,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 2 * FRACUNIT,
        /* pickupradius         */ 2 * FRACUNIT,
        /* height               */ 4 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_LOGRAV | MF3_CANNOTPUSH,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_PODGENERATOR
    {
        /* doomednum            */ 43,
        /* spawnstate           */ HS_PODGENERATOR,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOSECTOR,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SPLASH
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SPLASH1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SPLASHX,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 2 * FRACUNIT,
        /* pickupradius         */ 2 * FRACUNIT,
        /* height               */ 4 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_LOGRAV | MF3_CANNOTPUSH,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },
    // HMT_SPLASHBASE
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SPLASHBASE1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },
    // HMT_LAVASPLASH
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_LAVASPLASH1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },
    // HMT_LAVASMOKE
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_LAVASMOKE1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SLUDGECHUNK
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SLUDGECHUNK1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SLUDGECHUNKX,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 2 * FRACUNIT,
        /* pickupradius         */ 2 * FRACUNIT,
        /* height               */ 4 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_LOGRAV | MF3_CANNOTPUSH,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SLUDGESPLASH
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SLUDGESPLASH1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SKULLHANG70
    {
        /* doomednum            */ 17,
        /* spawnstate           */ HS_SKULLHANG70_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */  0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 70 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SKULLHANG60
    {
        /* doomednum            */ 24,
        /* spawnstate           */ HS_SKULLHANG60_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 60 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SKULLHANG45
    {
        /* doomednum            */ 25,
        /* spawnstate           */ HS_SKULLHANG45_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 45 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SKULLHANG35
    {
        /* doomednum            */ 26,
        /* spawnstate           */ HS_SKULLHANG35_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 35 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CHANDELIER
    {
        /* doomednum            */ 28,
        /* spawnstate           */ HS_CHANDELIER1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 60 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SERPTORCH
    {
        /* doomednum            */ 27,
        /* spawnstate           */ HS_SERPTORCH1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 54 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SMALLPILLAR
    {
        /* doomednum            */ 29,
        /* spawnstate           */ HS_SMALLPILLAR,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 34 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_STALAGMITESMALL
    {
        /* doomednum            */ 37,
        /* spawnstate           */ HS_STALAGMITESMALL,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 32 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0
    },

    // HMT_STALAGMITELARGE
    {
        /* doomednum            */ 38,
        /* spawnstate           */ HS_STALAGMITELARGE,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 64 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_STALACTITESMALL
    {
        /* doomednum            */ 39,
        /* spawnstate           */ HS_STALACTITESMALL,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 36 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_STALACTITELARGE
    {
        /* doomednum            */ 40,
        /* spawnstate           */ HS_STALACTITELARGE,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 68 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC6
    {
        /* doomednum            */ 76,
        /* spawnstate           */ HS_FIREBRAZIER1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 44 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BARREL
    {
        /* doomednum            */ 44,
        /* spawnstate           */ HS_BARREL,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 32 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC7
    {
        /* doomednum            */ 47,
        /* spawnstate           */ HS_BRPILLAR,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 14 * FRACUNIT,
        /* pickupradius         */ 14 * FRACUNIT,
        /* height               */ 128 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC8
    {
        /* doomednum            */ 48,
        /* spawnstate           */ HS_MOSS1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 23 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC9
    {
        /* doomednum            */ 49,
        /* spawnstate           */ HS_MOSS2,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 27 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC10
    {
        /* doomednum            */ 50,
        /* spawnstate           */ HS_WALLTORCH1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC11
    {
        /* doomednum            */ 51,
        /* spawnstate           */ HS_HANGINGCORPSE,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 104 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID | MF_SPAWNCEILING | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_KEYGIZMOBLUE
    {
        /* doomednum            */ 94,
        /* spawnstate           */ HS_KEYGIZMO1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 50 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_KEYGIZMOGREEN
    {
        /* doomednum            */ 95,
        /* spawnstate           */ HS_KEYGIZMO1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 50 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_KEYGIZMOYELLOW
    {
        /* doomednum            */ 96,
        /* spawnstate           */ HS_KEYGIZMO1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 50 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_KEYGIZMOFLOAT
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_KGZ_START,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC12
    {
        /* doomednum            */ 87,
        /* spawnstate           */ HS_VOLCANO1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 20 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_VOLCANOBLAST
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_VOLCANOBALL1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_VOLCANOBALLX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_volhit,
        /* speed                */ 2 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_LOGRAV | MF3_NOTELEPORT | MF3_FIREDAMAGE,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_VOLCANOTBLAST
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_VOLCANOTBALL1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_VOLCANOTBALLX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 2 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_LOGRAV | MF3_NOTELEPORT | MF3_FIREDAMAGE,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_TELEGLITGEN
    {
        /* doomednum            */ 74,
        /* spawnstate           */ HS_TELEGLITGEN1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY | MF_NOSECTOR,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_TELEGLITGEN2
    {
        /* doomednum            */ 52,
        /* spawnstate           */ HS_TELEGLITGEN2,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY | MF_NOSECTOR,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_TELEGLITTER
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_TELEGLITTER1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_TELEGLITTER2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_TELEGLITTER2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_TFOG
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_TFOG1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_TELEPORTMAN
    {
        /* doomednum            */ 14,
        /* spawnstate           */ HS_NULL,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOSECTOR,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_STAFFPUFF
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_STAFFPUFF1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_stfhit,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_STAFFPUFF2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_STAFFPUFF2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_stfpow,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BEAKPUFF
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_STAFFPUFF1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_chicatk,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC13
    {
        /* doomednum            */ 2005,
        /* spawnstate           */ HS_WGNT,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_GAUNTLETPUFF1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_GAUNTLETPUFF1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_GAUNTLETPUFF2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_GAUNTLETPUFF2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC14
    {
        /* doomednum            */ 53,
        /* spawnstate           */ HS_BLSR,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BLASTERFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BLASTERFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_BLASTERFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_blshit,
        /* speed                */ 184 * FRACUNIT,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT
    },

    // HMT_BLASTERSMOKE
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BLASTERSMOKE1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_CANNOTPUSH
    },

    // HMT_RIPPER
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_RIPPER1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_RIPPERX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_hrnhit,
        /* speed                */ 14 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_RIP,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BLASTERPUFF1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BLASTERPUFF1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BLASTERPUFF2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BLASTERPUFF2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_WMACE
    {
        /* doomednum            */ 2002,
        /* spawnstate           */ HS_WMCE,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MACEFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MACEFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_lobsht,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MACEFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 20 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_FLOORBOUNCE | MF3_THRUGHOST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MACEFX2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MACEFX2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MACEFXI2_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 10 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 6,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_LOGRAV | MF3_FLOORBOUNCE | MF3_THRUGHOST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MACEFX3
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MACEFX3_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MACEFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 7 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 4,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_LOGRAV | MF3_FLOORBOUNCE | MF3_THRUGHOST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MACEFX4
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MACEFX4_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MACEFXI4_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 7 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 18,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_LOGRAV | MF3_FLOORBOUNCE | MF3_THRUGHOST | MF3_TELESTOMP,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_WSKULLROD
    {
        /* doomednum            */ 2004,
        /* spawnstate           */ HS_WSKL,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_HORNRODFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_HRODFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_hrnsht,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_HRODFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_hrnhit,
        /* speed                */ 22 * FRACUNIT,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 3,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_WINDTHRUST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_HORNRODFX2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_HRODFX2_1,
        /* spawnhealth          */ 4 * 35,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_hrnsht,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_HRODFXI2_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_ramphit,
        /* speed                */ 22 * FRACUNIT,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 10,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_RAINPLR1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_RAINPLR1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_RAINPLR1X_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 12 * FRACUNIT,
        /* radius               */ 5 * FRACUNIT,
        /* pickupradius         */ 5 * FRACUNIT,
        /* height               */ 12 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 5,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_RAINPLR2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_RAINPLR2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_RAINPLR2X_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 12 * FRACUNIT,
        /* radius               */ 5 * FRACUNIT,
        /* pickupradius         */ 5 * FRACUNIT,
        /* height               */ 12 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 5,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_RAINPLR3
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_RAINPLR3_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_RAINPLR3X_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 12 * FRACUNIT,
        /* radius               */ 5 * FRACUNIT,
        /* pickupradius         */ 5 * FRACUNIT,
        /* height               */ 12 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 5,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_RAINPLR4
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_RAINPLR4_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_RAINPLR4X_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 12 * FRACUNIT,
        /* radius               */ 5 * FRACUNIT,
        /* pickupradius         */ 5 * FRACUNIT,
        /* height               */ 12 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 5,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_GOLDWANDFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_GWANDFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_GWANDFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_gldhit,
        /* speed                */ 22 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_GOLDWANDFX2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_GWANDFX2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_GWANDFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 18 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_GOLDWANDPUFF1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_GWANDPUFF1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_GOLDWANDPUFF2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_GWANDFXI1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_WPHOENIXROD
    {
        /* doomednum            */ 2003,
        /* spawnstate           */ HS_WPHX,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_PHOENIXFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_PHOENIXFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_phosht,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_PHOENIXFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_phohit,
        /* speed                */ 20 * FRACUNIT,
        /* radius               */ 11 * FRACUNIT,
        /* pickupradius         */ 11 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 20,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_THRUGHOST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_PHOENIXFX_REMOVED
     {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_PHOENIXFXIX_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_PHOENIXFXIX_3,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 2 * FRACUNIT,
        /* pickupradius         */ 2 * FRACUNIT,
        /* height               */ 4 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_PHOENIXPUFF
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_PHOENIXPUFF1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY/* | MF_SHADOW*/,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_CANNOTPUSH,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_PHOENIXFX2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_PHOENIXFX2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_PHOENIXFXI2_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 10 * FRACUNIT,
        /* radius               */ 6 * FRACUNIT,
        /* pickupradius         */ 6 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_FIREDAMAGE,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MISC15
    {
        /* doomednum            */ 2001,
        /* spawnstate           */ HS_WBOW,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CRBOWFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_CRBOWFX1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_bowsht,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_CRBOWFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_hrnhit,
        /* speed                */ 30 * FRACUNIT,
        /* radius               */ 11 * FRACUNIT,
        /* pickupradius         */ 11 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 10,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CRBOWFX2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_CRBOWFX2,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_bowsht,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_CRBOWFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_hrnhit,
        /* speed                */ 32 * FRACUNIT,
        /* radius               */ 11 * FRACUNIT,
        /* pickupradius         */ 11 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 6,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CRBOWFX3
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_CRBOWFX3,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_CRBOWFXI3_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_hrnhit,
        /* speed                */ 20 * FRACUNIT,
        /* radius               */ 11 * FRACUNIT,
        /* pickupradius         */ 11 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_WINDTHRUST | MF3_THRUGHOST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CRBOWFX4
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_CRBOWFX4_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP,
        /* flags2               */ 0,
        /* flags3               */ MF3_LOGRAV,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BLOOD
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BLOOD1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP,
        /* flags2               */ MF2_BLOOD | MF2_NOFOOTCLIP | MF2_TRANSLUCENT_50,
        /* flags3               */ 0,
        /* raisestate           */ S_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ REDBLOOD2,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BLOODSPLATTER
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BLOODSPLATTER1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_BLOODSPLATTERX,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 2 * FRACUNIT,
        /* pickupradius         */ 2 * FRACUNIT,
        /* height               */ 4 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ MF2_BLOOD,
        /* flags3               */ MF3_NOTELEPORT | MF3_CANNOTPUSH,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ REDBLOOD2,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_PLAYER
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_PLAY,
        /* spawnhealth          */ 100,
        /* gibhealth            */ 0,
        /* seestate             */ HS_PLAY_RUN1,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 0,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_PLAY_PAIN,
        /* painchance           */ 255,
        /* painsound            */ hsfx_plrpai,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_PLAY_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_PLAY_DIE1,
        /* xdeathstate          */ HS_PLAY_XDIE1,
        /* deathsound           */ hsfx_plrdth,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 56 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_PICKUP | MF_NOTDMATCH,
        /* flags2               */ MF2_CASTSHADOW | MF2_PASSMOBJ | MF2_DONTMAP | MF2_CRUSHABLE | MF2_NOLIQUIDBOB,
        /* flags3               */ MF3_WINDTHRUST | MF3_SLIDE | MF3_TELESTOMP,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BLOODYSKULL
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BLOODYSKULL1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 4 * FRACUNIT,
        /* pickupradius         */ 4 * FRACUNIT,
        /* height               */ 4 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_LOGRAV | MF3_CANNOTPUSH,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CHICPLAYER
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_CHICPLAY,
        /* spawnhealth          */ 100,
        /* gibhealth            */ 0,
        /* seestate             */ HS_CHICPLAY_RUN1,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 0,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_CHICPLAY_PAIN,
        /* painchance           */ 255,
        /* painsound            */ hsfx_chicpai,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_CHICPLAY_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_CHICKEN_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_chicdth,
        /* speed                */ 0,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 24 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_DROPOFF | MF_NOTDMATCH,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ MF3_WINDTHRUST | MF3_SLIDE | MF3_LOGRAV | MF3_TELESTOMP,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CHICKEN
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_CHICKEN_LOOK1,
        /* spawnhealth          */ 10,
        /* gibhealth            */ 0,
        /* seestate             */ HS_CHICKEN_WALK1,
        /* seesound             */ hsfx_chicpai,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_chicatk,
        /* painstate            */ HS_CHICKEN_PAIN1,
        /* painchance           */ 200,
        /* painsound            */ hsfx_chicpai,
        /* meleestate           */ HS_CHICKEN_ATK1,
        /* missilestate         */ 0,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_CHICKEN_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_chicdth,
        /* speed                */ 4,
        /* radius               */ 9 * FRACUNIT,
        /* pickupradius         */ 9 * FRACUNIT,
        /* height               */ 22 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 40,
        /* damage               */ 0,
        /* activesound          */ hsfx_chicact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ MF3_WINDTHRUST,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_FEATHER
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_FEATHER1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_FEATHERX,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 2 * FRACUNIT,
        /* pickupradius         */ 2 * FRACUNIT,
        /* height               */ 4 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_LOGRAV | MF3_CANNOTPUSH | MF3_WINDTHRUST,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MUMMY
    {
        /* doomednum            */ 68,
        /* spawnstate           */ HS_MUMMY_LOOK1,
        /* spawnhealth          */ 80,
        /* gibhealth            */ 0,
        /* seestate             */ HS_MUMMY_WALK1,
        /* seesound             */ hsfx_mumsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_mumat1,
        /* painstate            */ HS_MUMMY_PAIN1,
        /* painchance           */ 128,
        /* painsound            */ hsfx_mumpai,
        /* meleestate           */ HS_MUMMY_ATK1,
        /* missilestate         */ 0,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MUMMY_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_mumdth,
        /* speed                */ 12,
        /* radius               */ 22 * FRACUNIT,
        /* pickupradius         */ 22 * FRACUNIT,
        /* height               */ 62 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 75,
        /* damage               */ 0,
        /* activesound          */ hsfx_mumact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MUMMYLEADER
    {
        /* doomednum            */ 45,
        /* spawnstate           */ HS_MUMMY_LOOK1,
        /* spawnhealth          */ 100,
        /* gibhealth            */ 0,
        /* seestate             */ HS_MUMMY_WALK1,
        /* seesound             */ hsfx_mumsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_mumat1,
        /* painstate            */ HS_MUMMY_PAIN1,
        /* painchance           */ 64,
        /* painsound            */ hsfx_mumpai,
        /* meleestate           */ HS_MUMMY_ATK1,
        /* missilestate         */ HS_MUMMYL_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MUMMY_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_mumdth,
        /* speed                */ 12,
        /* radius               */ 22 * FRACUNIT,
        /* pickupradius         */ 22 * FRACUNIT,
        /* height               */ 62 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 75,
        /* damage               */ 0,
        /* activesound          */ hsfx_mumact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MUMMYGHOST
    {
        /* doomednum            */ 69,
        /* spawnstate           */ HS_MUMMY_LOOK1,
        /* spawnhealth          */ 80,
        /* gibhealth            */ 0,
        /* seestate             */ HS_MUMMY_WALK1,
        /* seesound             */ hsfx_mumsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_mumat1,
        /* painstate            */ HS_MUMMY_PAIN1,
        /* painchance           */ 128,
        /* painsound            */ hsfx_mumpai,
        /* meleestate           */ HS_MUMMY_ATK1,
        /* missilestate         */ 0,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MUMMY_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_mumdth,
        /* speed                */ 12,
        /* radius               */ 22 * FRACUNIT,
        /* pickupradius         */ 22 * FRACUNIT,
        /* height               */ 62 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 75,
        /* damage               */ 0,
        /* activesound          */ hsfx_mumact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL/* | MF_SHADOW*/,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MUMMYLEADERGHOST
    {
        /* doomednum            */ 46,
        /* spawnstate           */ HS_MUMMY_LOOK1,
        /* spawnhealth          */ 100,
        /* gibhealth            */ 0,
        /* seestate             */ HS_MUMMY_WALK1,
        /* seesound             */ hsfx_mumsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_mumat1,
        /* painstate            */ HS_MUMMY_PAIN1,
        /* painchance           */ 64,
        /* painsound            */ hsfx_mumpai,
        /* meleestate           */ HS_MUMMY_ATK1,
        /* missilestate         */ HS_MUMMYL_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MUMMY_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_mumdth,
        /* speed                */ 12,
        /* radius               */ 22 * FRACUNIT,
        /* pickupradius         */ 22 * FRACUNIT,
        /* height               */ 62 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 75,
        /* damage               */ 0,
        /* activesound          */ hsfx_mumact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL/* | MF_SHADOW*/,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MUMMYSOUL
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MUMMY_SOUL1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MUMMYFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MUMMYFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MUMMYFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 9 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 14 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 4,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BEAST
    {
        /* doomednum            */ 70,
        /* spawnstate           */ HS_BEAST_LOOK1,
        /* spawnhealth          */ 220,
        /* gibhealth            */ 0,
        /* seestate             */ HS_BEAST_WALK1,
        /* seesound             */ hsfx_bstsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_bstatk,
        /* painstate            */ HS_BEAST_PAIN1,
        /* painchance           */ 100,
        /* painsound            */ hsfx_bstpai,
        /* meleestate           */ 0,
        /* missilestate         */ HS_BEAST_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_BEAST_DIE1,
        /* xdeathstate          */ HS_BEAST_XDIE1,
        /* deathsound           */ hsfx_bstdth,
        /* speed                */ 14,
        /* radius               */ 32 * FRACUNIT,
        /* pickupradius         */ 32 * FRACUNIT,
        /* height               */ 74 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 200,
        /* damage               */ 0,
        /* activesound          */ hsfx_bstact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BEASTBALL
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BEASTBALL1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_BEASTBALLX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 12 * FRACUNIT,
        /* radius               */ 9 * FRACUNIT,
        /* pickupradius         */ 9 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 4,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_WINDTHRUST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BURNBALL
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BURNBALL1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_BEASTBALLX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 10 * FRACUNIT,
        /* radius               */ 6 * FRACUNIT,
        /* pickupradius         */ 6 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BURNBALLFB
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_BURNBALLFB1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_BEASTBALLX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 10 * FRACUNIT,
        /* radius               */ 6 * FRACUNIT,
        /* pickupradius         */ 6 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_PUFFY
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_PUFFY1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_PUFFY1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 10 * FRACUNIT,
        /* radius               */ 6 * FRACUNIT,
        /* pickupradius         */ 6 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY | MF_MISSILE,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SNAKE
    {
        /* doomednum            */ 92,
        /* spawnstate           */ HS_SNAKE_LOOK1,
        /* spawnhealth          */ 280,
        /* gibhealth            */ 0,
        /* seestate             */ HS_SNAKE_WALK1,
        /* seesound             */ hsfx_snksit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_snkatk,
        /* painstate            */ HS_SNAKE_PAIN1,
        /* painchance           */ 48,
        /* painsound            */ hsfx_snkpai,
        /* meleestate           */ 0,
        /* missilestate         */ HS_SNAKE_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SNAKE_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_snkdth,
        /* speed                */ 10,
        /* radius               */ 22 * FRACUNIT,
        /* pickupradius         */ 22 * FRACUNIT,
        /* height               */ 70 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_snkact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SNAKEPRO_A
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SNAKEPRO_A1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SNAKEPRO_AX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 14 * FRACUNIT,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_WINDTHRUST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SNAKEPRO_B
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SNAKEPRO_B1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SNAKEPRO_BX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 14 * FRACUNIT,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 3,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_HEAD
    {
        /* doomednum            */ 6,
        /* spawnstate           */ HS_HEAD_LOOK,
        /* spawnhealth          */ 700,
        /* gibhealth            */ 0,
        /* seestate             */ HS_HEAD_FLOAT,
        /* seesound             */ hsfx_hedsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_hedat1,
        /* painstate            */ HS_HEAD_PAIN1,
        /* painchance           */ 32,
        /* painsound            */ hsfx_hedpai,
        /* meleestate           */ 0,
        /* missilestate         */ HS_HEAD_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_HEAD_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_heddth,
        /* speed                */ 6,
        /* radius               */ 40 * FRACUNIT,
        /* pickupradius         */ 40 * FRACUNIT,
        /* height               */ 72 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 325,
        /* damage               */ 0,
        /* activesound          */ hsfx_hedact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_NOBLOOD,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_HEADFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_HEADFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_HEADFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 13 * FRACUNIT,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_THRUGHOST,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_HEADFX2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_HEADFX2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_HEADFXI2_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 8 * FRACUNIT,
        /* radius               */ 12 * FRACUNIT,
        /* pickupradius         */ 12 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 3,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_HEADFX3
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_HEADFX3_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_HEADFXI3_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 10 * FRACUNIT,
        /* radius               */ 14 * FRACUNIT,
        /* pickupradius         */ 14 * FRACUNIT,
        /* height               */ 12 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 5,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_WINDTHRUST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_WHIRLWIND
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_HEADFX4_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_HEADFXI4_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 10 * FRACUNIT,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 74 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY/* | MF_SHADOW*/,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CLINK
    {
        /* doomednum            */ 90,
        /* spawnstate           */ HS_CLINK_LOOK1,
        /* spawnhealth          */ 150,
        /* gibhealth            */ 0,
        /* seestate             */ HS_CLINK_WALK1,
        /* seesound             */ hsfx_clksit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_clkatk,
        /* painstate            */ HS_CLINK_PAIN1,
        /* painchance           */ 32,
        /* painsound            */ hsfx_clkpai,
        /* meleestate           */ HS_CLINK_ATK1,
        /* missilestate         */ 0,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_CLINK_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_clkdth,
        /* speed                */ 14,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 64 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 75,
        /* damage               */ 0,
        /* activesound          */ hsfx_clkact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_NOBLOOD,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

// HMT_WIZARD
    {
        /* doomednum            */ 15,
        /* spawnstate           */ HS_WIZARD_LOOK1,
        /* spawnhealth          */ 180,
        /* gibhealth            */ 0,
        /* seestate             */ HS_WIZARD_WALK1,
        /* seesound             */ hsfx_wizsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_wizatk,
        /* painstate            */ HS_WIZARD_PAIN1,
        /* painchance           */ 64,
        /* painsound            */ hsfx_wizpai,
        /* meleestate           */ 0,
        /* missilestate         */ HS_WIZARD_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_WIZARD_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_wizdth,
        /* speed                */ 12,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 68 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_wizact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_FLOAT | MF_NOGRAVITY,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_WIZFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_WIZFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_WIZFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 18 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 3,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_IMP
    {
        /* doomednum            */ 66,
        /* spawnstate           */ HS_IMP_LOOK1,
        /* spawnhealth          */ 40,
        /* gibhealth            */ 0,
        /* seestate             */ HS_IMP_FLY1,
        /* seesound             */ hsfx_impsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_impat1,
        /* painstate            */ HS_IMP_PAIN1,
        /* painchance           */ 200,
        /* painsound            */ hsfx_imppai,
        /* meleestate           */ HS_IMP_MEATK1,
        /* missilestate         */ HS_IMP_MSATK1_1,
        /* crashstate           */ HS_IMP_CRASH1,
        /* deathstate           */ HS_IMP_DIE1,
        /* xdeathstate          */ HS_IMP_XDIE1,
        /* deathsound           */ hsfx_impdth,
        /* speed                */ 10,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 36 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 50,
        /* damage               */ 0,
        /* activesound          */ hsfx_impact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL,
        /* flags2               */ MF2_PASSMOBJ | MF2_CASTSHADOW,
        /* flags3               */ MF3_SPAWNFLOAT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ HMT_BLOOD,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_IMPLEADER
    {
        /* doomednum            */ 5,
        /* spawnstate           */ HS_IMP_LOOK1,
        /* spawnhealth          */ 80,
        /* gibhealth            */ 0,
        /* seestate             */ HS_IMP_FLY1,
        /* seesound             */ hsfx_impsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_impat2,
        /* painstate            */ HS_IMP_PAIN1,
        /* painchance           */ 200,
        /* painsound            */ hsfx_imppai,
        /* meleestate           */ 0,
        /* missilestate         */ HS_IMP_MSATK2_1,
        /* crashstate           */ HS_IMP_CRASH1,
        /* deathstate           */ HS_IMP_DIE1,
        /* xdeathstate          */ HS_IMP_XDIE1,
        /* deathsound           */ hsfx_impdth,
        /* speed                */ 10,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 36 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 50,
        /* damage               */ 0,
        /* activesound          */ hsfx_impact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_FLOAT | MF_NOGRAVITY | MF_COUNTKILL,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ MF3_SPAWNFLOAT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ HMT_BLOOD,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_IMPCHUNK1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_IMP_CHUNKA1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_IMPCHUNK2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_IMP_CHUNKB1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_IMPBALL
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_IMPFX1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_IMPFXI1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 10 * FRACUNIT,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_WINDTHRUST | MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_KNIGHT
    {
        /* doomednum            */ 64,
        /* spawnstate           */ HS_KNIGHT_STND1,
        /* spawnhealth          */ 200,
        /* gibhealth            */ 0,
        /* seestate             */ HS_KNIGHT_WALK1,
        /* seesound             */ hsfx_kgtsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_kgtatk,
        /* painstate            */ HS_KNIGHT_PAIN1,
        /* painchance           */ 100,
        /* painsound            */ hsfx_kgtpai,
        /* meleestate           */ HS_KNIGHT_ATK1,
        /* missilestate         */ HS_KNIGHT_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_KNIGHT_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_kgtdth,
        /* speed                */ 12,
        /* radius               */ 24 * FRACUNIT,
        /* pickupradius         */ 24 * FRACUNIT,
        /* height               */ 78 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 150,
        /* damage               */ 0,
        /* activesound          */ hsfx_kgtact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_KNIGHTGHOST
    {
        /* doomednum            */ 65,
        /* spawnstate           */ HS_KNIGHT_STND1,
        /* spawnhealth          */ 200,
        /* gibhealth            */ 0,
        /* seestate             */ HS_KNIGHT_WALK1,
        /* seesound             */ hsfx_kgtsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_kgtatk,
        /* painstate            */ HS_KNIGHT_PAIN1,
        /* painchance           */ 100,
        /* painsound            */ hsfx_kgtpai,
        /* meleestate           */ HS_KNIGHT_ATK1,
        /* missilestate         */ HS_KNIGHT_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_KNIGHT_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_kgtdth,
        /* speed                */ 12,
        /* radius               */ 24 * FRACUNIT,
        /* pickupradius         */ 24 * FRACUNIT,
        /* height               */ 78 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 150,
        /* damage               */ 0,
        /* activesound          */ hsfx_kgtact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL/* | MF_SHADOW*/,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_KNIGHTAXE
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SPINAXE1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SPINAXEX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_hrnhit,
        /* speed                */ 9 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 2,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_WINDTHRUST | MF3_NOTELEPORT | MF3_THRUGHOST,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_REDAXE
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_REDAXE1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_REDAXEX1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_hrnhit,
        /* speed                */ 9 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 8 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 7,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_THRUGHOST,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SORCERER1
    {
        /* doomednum            */ 7,
        /* spawnstate           */ HS_SRCR1_LOOK1,
        /* spawnhealth          */ 2000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_SRCR1_WALK1,
        /* seesound             */ hsfx_sbtsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_sbtatk,
        /* painstate            */ HS_SRCR1_PAIN1,
        /* painchance           */ 56,
        /* painsound            */ hsfx_sbtpai,
        /* meleestate           */ 0,
        /* missilestate         */ HS_SRCR1_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SRCR1_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_sbtdth,
        /* speed                */ 16,
        /* radius               */ 28 * FRACUNIT,
        /* pickupradius         */ 28 * FRACUNIT,
        /* height               */ 100 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 800,
        /* damage               */ 0,
        /* activesound          */ hsfx_sbtact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ MF3_BOSS,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SRCRFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SRCRFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SRCRFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 20 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 10 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 10,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_FIREDAMAGE,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SORCERER2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SOR2_LOOK1,
        /* spawnhealth          */ 3500,
        /* gibhealth            */ 0,
        /* seestate             */ HS_SOR2_WALK1,
        /* seesound             */ hsfx_sorsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_soratk,
        /* painstate            */ HS_SOR2_PAIN1,
        /* painchance           */ 32,
        /* painsound            */ hsfx_sorpai,
        /* meleestate           */ 0,
        /* missilestate         */ HS_SOR2_ATK1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SOR2_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 14,
        /* radius               */ 16 * FRACUNIT,
        /* pickupradius         */ 16 * FRACUNIT,
        /* height               */ 70 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 300,
        /* damage               */ 0,
        /* activesound          */ hsfx_soract,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ MF3_BOSS,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SOR2FX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SOR2FX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SOR2FXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 20 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 1,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SOR2FXSPARK
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SOR2FXSPARK1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_CANNOTPUSH,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SOR2FX2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SOR2FX2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_SOR2FXI2_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 6 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 10,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SOR2TELEFADE
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_SOR2TELEFADE1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MINOTAUR
    {
        /* doomednum            */ 9,
        /* spawnstate           */ HS_MNTR_LOOK1,
        /* spawnhealth          */ 3000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_MNTR_WALK1,
        /* seesound             */ hsfx_minsit,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_minat1,
        /* painstate            */ HS_MNTR_PAIN1,
        /* painchance           */ 25,
        /* painsound            */ hsfx_minpai,
        /* meleestate           */ HS_MNTR_ATK1_1,
        /* missilestate         */ HS_MNTR_ATK2_1,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MNTR_DIE1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_mindth,
        /* speed                */ 16,
        /* radius               */ 28 * FRACUNIT,
        /* pickupradius         */ 28 * FRACUNIT,
        /* height               */ 100 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 800,
        /* damage               */ 7,
        /* activesound          */ hsfx_minact,
        /* flags                */ MF_SOLID | MF_SHOOTABLE | MF_COUNTKILL | MF_DROPOFF,
        /* flags2               */ MF2_PASSMOBJ,
        /* flags3               */ MF3_BOSS,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MNTRFX1
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MNTRFX1_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MNTRFXI1_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ 0,
        /* speed                */ 20 * FRACUNIT,
        /* radius               */ 10 * FRACUNIT,
        /* pickupradius         */ 10 * FRACUNIT,
        /* height               */ 6 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 3,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_FIREDAMAGE,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MNTRFX2
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MNTRFX2_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MNTRFXI2_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_phohit,
        /* speed                */ 14 * FRACUNIT,
        /* radius               */ 5 * FRACUNIT,
        /* pickupradius         */ 5 * FRACUNIT,
        /* height               */ 12 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 4,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_FIREDAMAGE,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_MNTRFX3
    {
        /* doomednum            */ -1,
        /* spawnstate           */ HS_MNTRFX3_1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ 0,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_MNTRFXI2_1,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_phohit,
        /* speed                */ 0,
        /* radius               */ 8 * FRACUNIT,
        /* pickupradius         */ 8 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 4,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_MISSILE | MF_DROPOFF | MF_NOGRAVITY,
        /* flags2               */ 0,
        /* flags3               */ MF3_NOTELEPORT | MF3_FIREDAMAGE,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AKYY
    {
        /* doomednum            */ 73,
        /* spawnstate           */ HS_AKYY1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_NOTDMATCH,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_BKYY
    {
        /* doomednum            */ 79,
        /* spawnstate           */ HS_BKYY1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_NOTDMATCH,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_CKEY
    {
        /* doomednum            */ 80,
        /* spawnstate           */ HS_CKYY1,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL | MF_NOTDMATCH,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMGWNDWIMPY
    {
        /* doomednum            */ 10,
        /* spawnstate           */ HS_AMG1,
        /* spawnhealth          */ AMMO_GWND_WIMPY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMGWNDHEFTY
    {
        /* doomednum            */ 12,
        /* spawnstate           */ HS_AMG2_1,
        /* spawnhealth          */ AMMO_GWND_HEFTY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMMACEWIMPY
    {
        /* doomednum            */ 13,
        /* spawnstate           */ HS_AMM1,
        /* spawnhealth          */ AMMO_MACE_WIMPY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMMACEHEFTY
    {
        /* doomednum            */ 16,
        /* spawnstate           */ HS_AMM2,
        /* spawnhealth          */ AMMO_MACE_HEFTY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMCBOWWIMPY
    {
        /* doomednum            */ 18,
        /* spawnstate           */ HS_AMC1,
        /* spawnhealth          */ AMMO_CBOW_WIMPY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMCBOWHEFTY
    {
        /* doomednum            */ 19,
        /* spawnstate           */ HS_AMC2_1,
        /* spawnhealth          */ AMMO_CBOW_HEFTY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMSKRDWIMPY
    {
        /* doomednum            */ 20,
        /* spawnstate           */ HS_AMS1_1,
        /* spawnhealth          */ AMMO_SKRD_WIMPY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMSKRDHEFTY
    {
        /* doomednum            */ 21,
        /* spawnstate           */ HS_AMS2_1,
        /* spawnhealth          */ AMMO_SKRD_HEFTY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMPHRDWIMPY
    {
        /* doomednum            */ 22,
        /* spawnstate           */ HS_AMP1_1,
        /* spawnhealth          */ AMMO_PHRD_WIMPY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMPHRDHEFTY
    {
        /* doomednum            */ 23,
        /* spawnstate           */ HS_AMP2_1,
        /* spawnhealth          */ AMMO_PHRD_HEFTY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMBLSRWIMPY
    {
        /* doomednum            */ 54,
        /* spawnstate           */ HS_AMB1_1,
        /* spawnhealth          */ AMMO_BLSR_WIMPY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_AMBLSRHEFTY
    {
        /* doomednum            */ 55,
        /* spawnstate           */ HS_AMB2_1,
        /* spawnhealth          */ AMMO_BLSR_HEFTY,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_SPECIAL,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SOUNDWIND
    {
        /* doomednum            */ 42,
        /* spawnstate           */ HS_SND_WIND,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOSECTOR,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    },

    // HMT_SOUNDWATERFALL
    {
        /* doomednum            */ 41,
        /* spawnstate           */ HS_SND_WATERFALL,
        /* spawnhealth          */ 1000,
        /* gibhealth            */ 0,
        /* seestate             */ HS_NULL,
        /* seesound             */ hsfx_None,
        /* reactiontime         */ 8,
        /* attacksound          */ hsfx_None,
        /* painstate            */ HS_NULL,
        /* painchance           */ 0,
        /* painsound            */ hsfx_None,
        /* meleestate           */ HS_NULL,
        /* missilestate         */ HS_NULL,
        /* crashstate           */ HS_NULL,
        /* deathstate           */ HS_NULL,
        /* xdeathstate          */ HS_NULL,
        /* deathsound           */ hsfx_None,
        /* speed                */ 0,
        /* radius               */ 20 * FRACUNIT,
        /* pickupradius         */ 20 * FRACUNIT,
        /* height               */ 16 * FRACUNIT,
        /* projectilepassheight */ 0,
        /* mass                 */ 100,
        /* damage               */ 0,
        /* activesound          */ hsfx_None,
        /* flags                */ MF_NOBLOCKMAP | MF_NOSECTOR,
        /* flags2               */ 0,
        /* flags3               */ 0,
        /* raisestate           */ HS_NULL,
        /* frames               */ 0,
        /* fullbright           */ false,
        /* blood                */ 0,
        /* shadowoffset         */ 0,
        /* name1                */ "",
        /* plural1              */ "",
        /* name2                */ "",
        /* plural2              */ "",
        /* name3                */ "",
        /* plural3              */ ""
    }
};
