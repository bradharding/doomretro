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
#include "states.h"

void A_AccTeleGlitter(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_AddPlayerRain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BabyMetal(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeakAttackPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeakAttackPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeakRaise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeakReady(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeastAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BeastPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BetaSkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BFGSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BFGSpray(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BlueSpark(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BoltSpark(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BossDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BossDeath2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainAwake(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainExplode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainSpit(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BruisAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BspiAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckBurnGone(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckReload(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckSkullDone(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckSkullFloor(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ChicAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ChicChase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ChicLook(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ChicPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ClinkAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CloseShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ContMobjSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CPosRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CyberAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_DeathBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Detonate(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Die(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_DripBlood(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ESound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Explode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack3(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatRaise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Feathers(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireBFG(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireBlasterPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireBlasterPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCGun(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCrackle(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCrossbowPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCrossbowPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireGoldWandPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireGoldWandPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireMacePL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireMacePL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireMissile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireOldBFG(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePhoenixPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePistol(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePlasma(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireShotgun(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireSkullRodPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireSkullRodPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FlameEnd(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FlameSnd(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FloatPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FreeTargMobj(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GauntletAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GenWizard(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GhostOff(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GunFlash(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadFireGrow(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadIceImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HideInCeiling(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HideThing(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Hoof(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpExplode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpMeAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpMsAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpMsAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpXDeath1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ImpXDeath2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_InitKeyGizmo(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_InitPhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_KeenDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_KnightAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light0(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_LoadShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MaceBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MaceBallImpact2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MacePL1Check(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MakePod(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Metal(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurAtk1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurAtk2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurAtk3(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurCharge(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MinotaurDecide(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MntrFloorFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MummyAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MummyAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MummyFX1Seek(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_MummySoul(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Mushroom(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_NoBlocking(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_OpenShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PainAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PainDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PhoenixPuff(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PlayerScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PodPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Punch(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RainImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RemovePod(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RestoreArtifact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RestoreSpecialThing1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_RestoreSpecialThing2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SargAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Saw(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ShutdownPhoenixPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelFist(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelMissile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelWhoosh(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullPop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullRodPL2Seek(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullRodStorm(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SnakeAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SnakeAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Sor1Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Sor1Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Sor2DthInit(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Sor2DthLoop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorcererRise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorDBon(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorDExp(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorDSph(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorRise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorSightSnd(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SorZap(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnFly(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnRippers(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnTeleGlitter(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnTeleGlitter2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpidRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Srcr1Attack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Srcr2Attack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Srcr2Decide(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_StaffAttackPL1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_StaffAttackPL2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_StartFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Stop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Tracer(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_TroopAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_UnHideThing(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileChase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileStart(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VolcanoBlast(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VolcanoSet(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VolcBallImpact(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WhirlwindSeek(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WizAtk1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WizAtk2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WizAtk3(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_XScream(mobj_t *actor, player_t *player, pspdef_t *psp);

state_t states[NUMSTATES] =
{
  //  sprite      frame                 tics  action                  nextstate                 state
    { SPR_TROO,   0,                      -1, NULL,                   S_NULL              }, // S_NULL

    { SPR_SHTG,   4,                       0, A_Light0,               S_NULL              }, // S_LIGHTDONE

    // Fist
    { SPR_PUNG,   0,                       1, A_WeaponReady,          S_PUNCH             }, // S_PUNCH
    { SPR_PUNG,   0,                       1, A_Lower,                S_PUNCHDOWN         }, // S_PUNCHDOWN
    { SPR_PUNG,   0,                       1, A_Raise,                S_PUNCHUP           }, // S_PUNCHUP
    { SPR_PUNG,   1,                       4, NULL,                   S_PUNCH2            }, // S_PUNCH1
    { SPR_PUNG,   2,                       4, A_Punch,                S_PUNCH3            }, // S_PUNCH2
    { SPR_PUNG,   3,                       5, NULL,                   S_PUNCH4            }, // S_PUNCH3
    { SPR_PUNG,   2,                       4, NULL,                   S_PUNCH5            }, // S_PUNCH4
    { SPR_PUNG,   1,                       5, A_ReFire,               S_PUNCH             }, // S_PUNCH5

    // Pistol
    { SPR_PISG,   0,                       1, A_WeaponReady,          S_PISTOL            }, // S_PISTOL
    { SPR_PISG,   0,                       1, A_Lower,                S_PISTOLDOWN        }, // S_PISTOLDOWN
    { SPR_PISG,   0,                       1, A_Raise,                S_PISTOLUP          }, // S_PISTOLUP
    { SPR_PISG,   0,                       4, NULL,                   S_PISTOL2           }, // S_PISTOL1
    { SPR_PISG,   1,                       6, A_FirePistol,           S_PISTOL3           }, // S_PISTOL2
    { SPR_PISG,   2,                       4, NULL,                   S_PISTOL4           }, // S_PISTOL3
    { SPR_PISG,   1,                       5, A_ReFire,               S_PISTOL            }, // S_PISTOL4
    { SPR_PISF,  (0 | FF_FULLBRIGHT),      7, A_Light1,               S_LIGHTDONE         }, // S_PISTOLFLASH

    // Shotgun
    { SPR_SHTG,   0,                       1, A_WeaponReady,          S_SGUN              }, // S_SGUN
    { SPR_SHTG,   0,                       1, A_Lower,                S_SGUNDOWN          }, // S_SGUNDOWN
    { SPR_SHTG,   0,                       1, A_Raise,                S_SGUNUP            }, // S_SGUNUP
    { SPR_SHTG,   0,                       3, NULL,                   S_SGUN2             }, // S_SGUN1
    { SPR_SHTG,   0,                       7, A_FireShotgun,          S_SGUN3             }, // S_SGUN2
    { SPR_SHTG,   1,                       5, NULL,                   S_SGUN4             }, // S_SGUN3
    { SPR_SHTG,   2,                       5, NULL,                   S_SGUN5             }, // S_SGUN4
    { SPR_SHTG,   3,                       4, NULL,                   S_SGUN6             }, // S_SGUN5
    { SPR_SHTG,   2,                       5, NULL,                   S_SGUN7             }, // S_SGUN6
    { SPR_SHTG,   1,                       5, NULL,                   S_SGUN8             }, // S_SGUN7
    { SPR_SHTG,   0,                       3, NULL,                   S_SGUN9             }, // S_SGUN8
    { SPR_SHTG,   0,                       7, A_ReFire,               S_SGUN              }, // S_SGUN9
    { SPR_SHTF,  (0 | FF_FULLBRIGHT),      4, A_Light1,               S_SGUNFLASH2        }, // S_SGUNFLASH1
    { SPR_SHTF,  (1 | FF_FULLBRIGHT),      3, A_Light2,               S_LIGHTDONE         }, // S_SGUNFLASH2

    // Super Shotgun
    { SPR_SHT2,    0,                      1, A_WeaponReady,          S_DSGUN             }, // S_DSGUN
    { SPR_SHT2,    0,                      1, A_Lower,                S_DSGUNDOWN         }, // S_DSGUNDOWN
    { SPR_SHT2,    0,                      1, A_Raise,                S_DSGUNUP           }, // S_DSGUNUP
    { SPR_SHT2,    0,                      3, NULL,                   S_DSGUN2            }, // S_DSGUN1
    { SPR_SHT2,   (0 | FF_FULLBRIGHT),     7, A_FireShotgun2,         S_DSGUN3            }, // S_DSGUN2
    { SPR_SHT2,    1,                      7, NULL,                   S_DSGUN4            }, // S_DSGUN3
    { SPR_SHT2,    2,                      7, A_CheckReload,          S_DSGUN5            }, // S_DSGUN4
    { SPR_SHT2,    3,                      7, A_OpenShotgun2,         S_DSGUN6            }, // S_DSGUN5
    { SPR_SHT2,    4,                      7, NULL,                   S_DSGUN7            }, // S_DSGUN6
    { SPR_SHT2,    5,                      7, A_LoadShotgun2,         S_DSGUN8            }, // S_DSGUN7
    { SPR_SHT2,    6,                      6, NULL,                   S_DSGUN9            }, // S_DSGUN8
    { SPR_SHT2,    7,                      6, A_CloseShotgun2,        S_DSGUN10           }, // S_DSGUN9
    { SPR_SHT2,    0,                      5, A_ReFire,               S_DSGUN             }, // S_DSGUN10
    { SPR_SHT2,    1,                      7, NULL,                   S_DSNR2             }, // S_DSNR1
    { SPR_SHT2,    0,                      3, NULL,                   S_DSGUNDOWN         }, // S_DSNR2
    { SPR_SHT2,   (8 | FF_FULLBRIGHT),     4, A_Light1,               S_DSGUNFLASH2       }, // S_DSGUNFLASH1
    { SPR_SHT2,   (9 | FF_FULLBRIGHT),     3, A_Light2,               S_LIGHTDONE         }, // S_DSGUNFLASH2

    // Chaingun
    { SPR_CHGG,    0,                     1, A_WeaponReady,          S_CHAIN             }, // S_CHAIN
    { SPR_CHGG,    0,                     1, A_Lower,                S_CHAINDOWN         }, // S_CHAINDOWN
    { SPR_CHGG,    0,                     1, A_Raise,                S_CHAINUP           }, // S_CHAINUP
    { SPR_CHGG,    0,                     4, A_FireCGun,             S_CHAIN2            }, // S_CHAIN1
    { SPR_CHGG,    1,                     4, A_FireCGun,             S_CHAIN3            }, // S_CHAIN2
    { SPR_CHGG,    1,                     0, A_ReFire,               S_CHAIN             }, // S_CHAIN3
    { SPR_CHGF,   (0 | FF_FULLBRIGHT),    5, A_Light1,               S_LIGHTDONE         }, // S_CHAINFLASH1
    { SPR_CHGF,   (1 | FF_FULLBRIGHT),    5, A_Light2,               S_LIGHTDONE         }, // S_CHAINFLASH2

    // Rocket Launcher
    { SPR_MISG,    0,                     1, A_WeaponReady,          S_MISSILE           }, // S_MISSILE
    { SPR_MISG,    0,                     1, A_Lower,                S_MISSILEDOWN       }, // S_MISSILEDOWN
    { SPR_MISG,    0,                     1, A_Raise,                S_MISSILEUP         }, // S_MISSILEUP
    { SPR_MISG,    1,                     8, A_GunFlash,             S_MISSILE2          }, // S_MISSILE1
    { SPR_MISG,    1,                    12, A_FireMissile,          S_MISSILE3          }, // S_MISSILE2
    { SPR_MISG,    1,                     0, A_ReFire,               S_MISSILE           }, // S_MISSILE3
    { SPR_MISF,   (0 | FF_FULLBRIGHT),    3, A_Light1,               S_MISSILEFLASH2     }, // S_MISSILEFLASH1
    { SPR_MISF,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_MISSILEFLASH3     }, // S_MISSILEFLASH2
    { SPR_MISF,   (2 | FF_FULLBRIGHT),    4, A_Light2,               S_MISSILEFLASH4     }, // S_MISSILEFLASH3
    { SPR_MISF,   (3 | FF_FULLBRIGHT),    4, A_Light2,               S_LIGHTDONE         }, // S_MISSILEFLASH4

    // Chainsaw
    { SPR_SAWG,    2,                     4, A_WeaponReady,          S_SAWB              }, // S_SAW
    { SPR_SAWG,    3,                     4, A_WeaponReady,          S_SAW               }, // S_SAWB
    { SPR_SAWG,    2,                     1, A_Lower,                S_SAWDOWN           }, // S_SAWDOWN
    { SPR_SAWG,    2,                     1, A_Raise,                S_SAWUP             }, // S_SAWUP
    { SPR_SAWG,    0,                     4, A_Saw,                  S_SAW2              }, // S_SAW1
    { SPR_SAWG,    1,                     4, A_Saw,                  S_SAW3              }, // S_SAW2
    { SPR_SAWG,    1,                     0, A_ReFire,               S_SAW               }, // S_SAW3

    // Plasma Rifle
    { SPR_PLSG,    0,                     1, A_WeaponReady,          S_PLASMA            }, // S_PLASMA
    { SPR_PLSG,    0,                     1, A_Lower,                S_PLASMADOWN        }, // S_PLASMADOWN
    { SPR_PLSG,    0,                     1, A_Raise,                S_PLASMAUP          }, // S_PLASMAUP
    { SPR_PLSG,    0,                     3, A_FirePlasma,           S_PLASMA2           }, // S_PLASMA1
    { SPR_PLSG,    1,                    20, A_ReFire,               S_PLASMA            }, // S_PLASMA2
    { SPR_PLSF,   (0 | FF_FULLBRIGHT),    4, A_Light1,               S_LIGHTDONE         }, // S_PLASMAFLASH1
    { SPR_PLSF,   (1 | FF_FULLBRIGHT),    4, A_Light1,               S_LIGHTDONE         }, // S_PLASMAFLASH2

    // BFG-9000
    { SPR_BFGG,    0,                     1, A_WeaponReady,          S_BFG               }, // S_BFG
    { SPR_BFGG,    0,                     1, A_Lower,                S_BFGDOWN           }, // S_BFGDOWN
    { SPR_BFGG,    0,                     1, A_Raise,                S_BFGUP             }, // S_BFGUP
    { SPR_BFGG,    0,                    20, A_BFGSound,             S_BFG2              }, // S_BFG1
    { SPR_BFGG,    1,                    10, A_GunFlash,             S_BFG3              }, // S_BFG2
    { SPR_BFGG,    1,                    10, A_FireBFG,              S_BFG4              }, // S_BFG3
    { SPR_BFGG,    1,                    20, A_ReFire,               S_BFG               }, // S_BFG4
    { SPR_BFGF,   (0 | FF_FULLBRIGHT),   11, A_Light1,               S_BFGFLASH2         }, // S_BFGFLASH1
    { SPR_BFGF,   (1 | FF_FULLBRIGHT),    6, A_Light2,               S_LIGHTDONE         }, // S_BFGFLASH2

    // Blood (MT_BLOOD)
    { SPR_BLUD,    2,                     8, NULL,                   S_BLOOD2            }, // S_BLOOD1
    { SPR_BLUD,    1,                     8, NULL,                   S_BLOOD3            }, // S_BLOOD2
    { SPR_BLUD,    0,                    -1, NULL,                   S_NULL              }, // S_BLOOD3

    // Bullet Puff (MT_PUFF)
    { SPR_PUFF,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_PUFF2             }, // S_PUFF1
    { SPR_PUFF,    1,                     4, NULL,                   S_PUFF3             }, // S_PUFF2
    { SPR_PUFF,    2,                     4, NULL,                   S_PUFF4             }, // S_PUFF3
    { SPR_PUFF,    3,                     4, NULL,                   S_NULL              }, // S_PUFF4

    // Imp Projectile (MT_TROOPSHOT)
    { SPR_BAL1,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_TBALL2            }, // S_TBALL1
    { SPR_BAL1,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_TBALL1            }, // S_TBALL2
    { SPR_BAL1,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_TBALLX2           }, // S_TBALLX1
    { SPR_BAL1,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_TBALLX3           }, // S_TBALLX2
    { SPR_BAL1,   (4 | FF_FULLBRIGHT),    6, NULL,                   S_NULL              }, // S_TBALLX3

    // Cacodemon Projectile (MT_HEADSHOT)
    { SPR_BAL2,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_RBALL2            }, // S_RBALL1
    { SPR_BAL2,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_RBALL1            }, // S_RBALL2
    { SPR_BAL2,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_RBALLX2           }, // S_RBALLX1
    { SPR_BAL2,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_RBALLX3           }, // S_RBALLX2
    { SPR_BAL2,   (4 | FF_FULLBRIGHT),    6, NULL,                   S_NULL              }, // S_RBALLX3

    // Plasma Rifle Projectile (MT_PLASMA)
    { SPR_PLSS,   (0 | FF_FULLBRIGHT),     6, NULL,                   S_PLASBALL2         }, // S_PLASBALL
    { SPR_PLSS,   (1 | FF_FULLBRIGHT),     6, NULL,                   S_PLASBALL          }, // S_PLASBALL2
    { SPR_PLSE,   (0 | FF_FULLBRIGHT),     4, NULL,                   S_PLASEXP2          }, // S_PLASEXP
    { SPR_PLSE,   (1 | FF_FULLBRIGHT),     4, NULL,                   S_PLASEXP3          }, // S_PLASEXP2
    { SPR_PLSE,   (2 | FF_FULLBRIGHT),     4, NULL,                   S_PLASEXP4          }, // S_PLASEXP3
    { SPR_PLSE,   (3 | FF_FULLBRIGHT),     4, NULL,                   S_PLASEXP5          }, // S_PLASEXP4
    { SPR_PLSE,   (4 | FF_FULLBRIGHT),     4, NULL,                   S_NULL              }, // S_PLASEXP5

    // Rocket Launcher Projectile (MT_ROCKET)
    { SPR_MISL,   (0 | FF_FULLBRIGHT),     1, NULL,                   S_ROCKET            }, // S_ROCKET

    // BFG-9000 Projectile (MT_BFG)
    { SPR_BFS1,   (0 | FF_FULLBRIGHT),     4, NULL,                   S_BFGSHOT2          }, // S_BFGSHOT
    { SPR_BFS1,   (1 | FF_FULLBRIGHT),     4, NULL,                   S_BFGSHOT           }, // S_BFGSHOT2
    { SPR_BFE1,   (0 | FF_FULLBRIGHT),     8, NULL,                   S_BFGLAND2          }, // S_BFGLAND
    { SPR_BFE1,   (1 | FF_FULLBRIGHT),     8, NULL,                   S_BFGLAND3          }, // S_BFGLAND2
    { SPR_BFE1,   (2 | FF_FULLBRIGHT),     8, A_BFGSpray,             S_BFGLAND4          }, // S_BFGLAND3
    { SPR_BFE1,   (3 | FF_FULLBRIGHT),     8, NULL,                   S_BFGLAND5          }, // S_BFGLAND4
    { SPR_BFE1,   (4 | FF_FULLBRIGHT),     8, NULL,                   S_BFGLAND6          }, // S_BFGLAND5
    { SPR_BFE1,   (5 | FF_FULLBRIGHT),     8, NULL,                   S_NULL              }, // S_BFGLAND6

    // BFG-9000 Secondary Projectile (MT_EXTRABFG)
    { SPR_BFE2,   (0 | FF_FULLBRIGHT),     8, NULL,                   S_BFGEXP2           }, // S_BFGEXP
    { SPR_BFE2,   (1 | FF_FULLBRIGHT),     8, NULL,                   S_BFGEXP3           }, // S_BFGEXP2
    { SPR_BFE2,   (2 | FF_FULLBRIGHT),     8, NULL,                   S_BFGEXP4           }, // S_BFGEXP3
    { SPR_BFE2,   (3 | FF_FULLBRIGHT),     8, NULL,                   S_NULL              }, // S_BFGEXP4
    { SPR_MISL,   (1 | FF_FULLBRIGHT),     8, A_Explode,              S_EXPLODE2          }, // S_EXPLODE1
    { SPR_MISL,   (2 | FF_FULLBRIGHT),     6, NULL,                   S_EXPLODE3          }, // S_EXPLODE2
    { SPR_MISL,   (3 | FF_FULLBRIGHT),     4, NULL,                   S_NULL              }, // S_EXPLODE3

    // Teleport Fog (MT_TFOG)
    { SPR_TFOG,   (0 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG01            }, // S_TFOG
    { SPR_TFOG,   (1 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG02            }, // S_TFOG01
    { SPR_TFOG,   (0 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG2             }, // S_TFOG02
    { SPR_TFOG,   (1 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG3             }, // S_TFOG2
    { SPR_TFOG,   (2 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG4             }, // S_TFOG3
    { SPR_TFOG,   (3 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG5             }, // S_TFOG4
    { SPR_TFOG,   (4 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG6             }, // S_TFOG5
    { SPR_TFOG,   (5 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG7             }, // S_TFOG6
    { SPR_TFOG,   (6 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG8             }, // S_TFOG7
    { SPR_TFOG,   (7 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG9             }, // S_TFOG8
    { SPR_TFOG,   (8 | FF_FULLBRIGHT),     6, NULL,                   S_TFOG10            }, // S_TFOG9
    { SPR_TFOG,   (9 | FF_FULLBRIGHT),     6, NULL,                   S_NULL              }, // S_TFOG10

    // Item Fog  Item Fog (MT_IFOG)
    { SPR_IFOG,   (0 | FF_FULLBRIGHT),     6, NULL,                   S_IFOG01            }, // S_IFOG
    { SPR_IFOG,   (1 | FF_FULLBRIGHT),     6, NULL,                   S_IFOG02            }, // S_IFOG01
    { SPR_IFOG,   (0 | FF_FULLBRIGHT),     6, NULL,                   S_IFOG2             }, // S_IFOG02
    { SPR_IFOG,   (1 | FF_FULLBRIGHT),     6, NULL,                   S_IFOG3             }, // S_IFOG2
    { SPR_IFOG,   (2 | FF_FULLBRIGHT),     6, NULL,                   S_IFOG4             }, // S_IFOG3
    { SPR_IFOG,   (3 | FF_FULLBRIGHT),     6, NULL,                   S_IFOG5             }, // S_IFOG4
    { SPR_IFOG,   (4 | FF_FULLBRIGHT),     6, NULL,                   S_NULL              }, // S_IFOG5

    // Player (MT_PLAYER)
    { SPR_PLAY,    0,                    -1, NULL,                   S_NULL              }, // S_PLAY
    { SPR_PLAY,    0,                     4, NULL,                   S_PLAY_RUN2         }, // S_PLAY_RUN1
    { SPR_PLAY,    1,                     4, NULL,                   S_PLAY_RUN3         }, // S_PLAY_RUN2
    { SPR_PLAY,    2,                     4, NULL,                   S_PLAY_RUN4         }, // S_PLAY_RUN3
    { SPR_PLAY,    3,                     4, NULL,                   S_PLAY_RUN1         }, // S_PLAY_RUN4
    { SPR_PLAY,    4,                    12, NULL,                   S_PLAY              }, // S_PLAY_ATK1
    { SPR_PLAY,   (5 | FF_FULLBRIGHT),    6, NULL,                   S_PLAY_ATK1         }, // S_PLAY_ATK2
    { SPR_PLAY,    6,                     4, NULL,                   S_PLAY_PAIN2        }, // S_PLAY_PAIN
    { SPR_PLAY,    6,                     4, A_Pain,                 S_PLAY              }, // S_PLAY_PAIN2
    { SPR_PLAY,    7,                    10, NULL,                   S_PLAY_DIE2         }, // S_PLAY_DIE1
    { SPR_PLAY,    8,                    10, A_PlayerScream,         S_PLAY_DIE3         }, // S_PLAY_DIE2
    { SPR_PLAY,    9,                    10, A_Fall,                 S_PLAY_DIE4         }, // S_PLAY_DIE3
    { SPR_PLAY,   10,                    10, NULL,                   S_PLAY_DIE5         }, // S_PLAY_DIE4
    { SPR_PLAY,   11,                    10, NULL,                   S_PLAY_DIE6         }, // S_PLAY_DIE5
    { SPR_PLAY,   12,                    10, NULL,                   S_PLAY_DIE7         }, // S_PLAY_DIE6

    // Player Death (MT_MISC62)
    { SPR_PLAY,   13,                    -1, NULL,                   S_NULL              }, // S_PLAY_DIE7
    { SPR_PLAY,   14,                     5, NULL,                   S_PLAY_XDIE2        }, // S_PLAY_XDIE1
    { SPR_PLAY,   15,                     5, A_XScream,              S_PLAY_XDIE3        }, // S_PLAY_XDIE2
    { SPR_PLAY,   16,                     5, A_Fall,                 S_PLAY_XDIE4        }, // S_PLAY_XDIE3
    { SPR_PLAY,   17,                     5, NULL,                   S_PLAY_XDIE5        }, // S_PLAY_XDIE4
    { SPR_PLAY,   18,                     5, NULL,                   S_PLAY_XDIE6        }, // S_PLAY_XDIE5
    { SPR_PLAY,   19,                     5, NULL,                   S_PLAY_XDIE7        }, // S_PLAY_XDIE6
    { SPR_PLAY,   20,                     5, NULL,                   S_PLAY_XDIE8        }, // S_PLAY_XDIE7
    { SPR_PLAY,   21,                     5, NULL,                   S_PLAY_XDIE9        }, // S_PLAY_XDIE8

    // Player Corpse (MT_MISC68 and MT_MISC69)
    { SPR_PLAY,   22,                    -1, NULL,                   S_NULL              }, // S_PLAY_XDIE9

    // Zombieman (MT_POSSESSED)
    { SPR_POSS,    0,                    10, A_Look,                 S_POSS_STND2        }, // S_POSS_STND
    { SPR_POSS,    1,                    10, A_Look,                 S_POSS_STND         }, // S_POSS_STND2
    { SPR_POSS,    0,                     4, A_Chase,                S_POSS_RUN2         }, // S_POSS_RUN1
    { SPR_POSS,    0,                     4, A_Chase,                S_POSS_RUN3         }, // S_POSS_RUN2
    { SPR_POSS,    1,                     4, A_Chase,                S_POSS_RUN4         }, // S_POSS_RUN3
    { SPR_POSS,    1,                     4, A_Chase,                S_POSS_RUN5         }, // S_POSS_RUN4
    { SPR_POSS,    2,                     4, A_Chase,                S_POSS_RUN6         }, // S_POSS_RUN5
    { SPR_POSS,    2,                     4, A_Chase,                S_POSS_RUN7         }, // S_POSS_RUN6
    { SPR_POSS,    3,                     4, A_Chase,                S_POSS_RUN8         }, // S_POSS_RUN7
    { SPR_POSS,    3,                     4, A_Chase,                S_POSS_RUN1         }, // S_POSS_RUN8
    { SPR_POSS,    4,                    10, A_FaceTarget,           S_POSS_ATK2         }, // S_POSS_ATK1
    { SPR_POSS,   (5 | FF_FULLBRIGHT),    8, A_PosAttack,            S_POSS_ATK3         }, // S_POSS_ATK2
    { SPR_POSS,    4,                     8, NULL,                   S_POSS_RUN1         }, // S_POSS_ATK3
    { SPR_POSS,    6,                     3, NULL,                   S_POSS_PAIN2        }, // S_POSS_PAIN
    { SPR_POSS,    6,                     3, A_Pain,                 S_POSS_RUN1         }, // S_POSS_PAIN2
    { SPR_POSS,    7,                     5, NULL,                   S_POSS_DIE2         }, // S_POSS_DIE1
    { SPR_POSS,    8,                     5, A_Scream,               S_POSS_DIE3         }, // S_POSS_DIE2
    { SPR_POSS,    9,                     5, A_Fall,                 S_POSS_DIE4         }, // S_POSS_DIE3
    { SPR_POSS,   10,                     5, NULL,                   S_POSS_DIE5         }, // S_POSS_DIE4

    // Zombieman Death (MT_MISC63)
    { SPR_POSS,   11,                    -1, NULL,                   S_NULL              }, // S_POSS_DIE5
    { SPR_POSS,   12,                     5, NULL,                   S_POSS_XDIE2        }, // S_POSS_XDIE1
    { SPR_POSS,   13,                     5, A_XScream,              S_POSS_XDIE3        }, // S_POSS_XDIE2
    { SPR_POSS,   14,                     5, A_Fall,                 S_POSS_XDIE4        }, // S_POSS_XDIE3
    { SPR_POSS,   15,                     5, NULL,                   S_POSS_XDIE5        }, // S_POSS_XDIE4
    { SPR_POSS,   16,                     5, NULL,                   S_POSS_XDIE6        }, // S_POSS_XDIE5
    { SPR_POSS,   17,                     5, NULL,                   S_POSS_XDIE7        }, // S_POSS_XDIE6
    { SPR_POSS,   18,                     5, NULL,                   S_POSS_XDIE8        }, // S_POSS_XDIE7
    { SPR_POSS,   19,                     5, NULL,                   S_POSS_XDIE9        }, // S_POSS_XDIE8
    { SPR_POSS,   20,                    -1, NULL,                   S_NULL              }, // S_POSS_XDIE9
    { SPR_POSS,   10,                     5, NULL,                   S_POSS_RAISE2       }, // S_POSS_RAISE1
    { SPR_POSS,    9,                     5, NULL,                   S_POSS_RAISE3       }, // S_POSS_RAISE2
    { SPR_POSS,    8,                     5, NULL,                   S_POSS_RAISE4       }, // S_POSS_RAISE3
    { SPR_POSS,    7,                     5, NULL,                   S_POSS_RUN1         }, // S_POSS_RAISE4

    // Shotgun Guy (MT_SHOTGUY)
    { SPR_SPOS,    0,                    10, A_Look,                 S_SPOS_STND2        }, // S_SPOS_STND
    { SPR_SPOS,    1,                    10, A_Look,                 S_SPOS_STND         }, // S_SPOS_STND2
    { SPR_SPOS,    0,                     3, A_Chase,                S_SPOS_RUN2         }, // S_SPOS_RUN1
    { SPR_SPOS,    0,                     3, A_Chase,                S_SPOS_RUN3         }, // S_SPOS_RUN2
    { SPR_SPOS,    1,                     3, A_Chase,                S_SPOS_RUN4         }, // S_SPOS_RUN3
    { SPR_SPOS,    1,                     3, A_Chase,                S_SPOS_RUN5         }, // S_SPOS_RUN4
    { SPR_SPOS,    2,                     3, A_Chase,                S_SPOS_RUN6         }, // S_SPOS_RUN5
    { SPR_SPOS,    2,                     3, A_Chase,                S_SPOS_RUN7         }, // S_SPOS_RUN6
    { SPR_SPOS,    3,                     3, A_Chase,                S_SPOS_RUN8         }, // S_SPOS_RUN7
    { SPR_SPOS,    3,                     3, A_Chase,                S_SPOS_RUN1         }, // S_SPOS_RUN8
    { SPR_SPOS,    4,                    10, A_FaceTarget,           S_SPOS_ATK2         }, // S_SPOS_ATK1
    { SPR_SPOS,   (5 | FF_FULLBRIGHT),   10, A_SPosAttack,           S_SPOS_ATK3         }, // S_SPOS_ATK2
    { SPR_SPOS,    4,                    10, NULL,                   S_SPOS_RUN1         }, // S_SPOS_ATK3
    { SPR_SPOS,    6,                     3, NULL,                   S_SPOS_PAIN2        }, // S_SPOS_PAIN
    { SPR_SPOS,    6,                     3, A_Pain,                 S_SPOS_RUN1         }, // S_SPOS_PAIN2
    { SPR_SPOS,    7,                     5, NULL,                   S_SPOS_DIE2         }, // S_SPOS_DIE1
    { SPR_SPOS,    8,                     5, A_Scream,               S_SPOS_DIE3         }, // S_SPOS_DIE2
    { SPR_SPOS,    9,                     5, A_Fall,                 S_SPOS_DIE4         }, // S_SPOS_DIE3
    { SPR_SPOS,   10,                     5, NULL,                   S_SPOS_DIE5         }, // S_SPOS_DIE4

    // Shotgun Guy Death (MT_MISC67)
    { SPR_SPOS,   11,                    -1, NULL,                   S_NULL              }, // S_SPOS_DIE5
    { SPR_SPOS,   12,                     5, NULL,                   S_SPOS_XDIE2        }, // S_SPOS_XDIE1
    { SPR_SPOS,   13,                     5, A_XScream,              S_SPOS_XDIE3        }, // S_SPOS_XDIE2
    { SPR_SPOS,   14,                     5, A_Fall,                 S_SPOS_XDIE4        }, // S_SPOS_XDIE3
    { SPR_SPOS,   15,                     5, NULL,                   S_SPOS_XDIE5        }, // S_SPOS_XDIE4
    { SPR_SPOS,   16,                     5, NULL,                   S_SPOS_XDIE6        }, // S_SPOS_XDIE5
    { SPR_SPOS,   17,                     5, NULL,                   S_SPOS_XDIE7        }, // S_SPOS_XDIE6
    { SPR_SPOS,   18,                     5, NULL,                   S_SPOS_XDIE8        }, // S_SPOS_XDIE7
    { SPR_SPOS,   19,                     5, NULL,                   S_SPOS_XDIE9        }, // S_SPOS_XDIE8
    { SPR_SPOS,   20,                    -1, NULL,                   S_NULL              }, // S_SPOS_XDIE9
    { SPR_SPOS,   11,                     5, NULL,                   S_SPOS_RAISE2       }, // S_SPOS_RAISE1
    { SPR_SPOS,   10,                     5, NULL,                   S_SPOS_RAISE3       }, // S_SPOS_RAISE2
    { SPR_SPOS,    9,                     5, NULL,                   S_SPOS_RAISE4       }, // S_SPOS_RAISE3
    { SPR_SPOS,    8,                     5, NULL,                   S_SPOS_RAISE5       }, // S_SPOS_RAISE4
    { SPR_SPOS,    7,                     5, NULL,                   S_SPOS_RUN1         }, // S_SPOS_RAISE5

    // Arch-vile (MT_VILE)
    { SPR_VILE,    0,                    10, A_Look,                 S_VILE_STND2        }, // S_VILE_STND
    { SPR_VILE,    1,                    10, A_Look,                 S_VILE_STND         }, // S_VILE_STND2
    { SPR_VILE,    0,                     2, A_VileChase,            S_VILE_RUN2         }, // S_VILE_RUN1
    { SPR_VILE,    0,                     2, A_VileChase,            S_VILE_RUN3         }, // S_VILE_RUN2
    { SPR_VILE,    1,                     2, A_VileChase,            S_VILE_RUN4         }, // S_VILE_RUN3
    { SPR_VILE,    1,                     2, A_VileChase,            S_VILE_RUN5         }, // S_VILE_RUN4
    { SPR_VILE,    2,                     2, A_VileChase,            S_VILE_RUN6         }, // S_VILE_RUN5
    { SPR_VILE,    2,                     2, A_VileChase,            S_VILE_RUN7         }, // S_VILE_RUN6
    { SPR_VILE,    3,                     2, A_VileChase,            S_VILE_RUN8         }, // S_VILE_RUN7
    { SPR_VILE,    3,                     2, A_VileChase,            S_VILE_RUN9         }, // S_VILE_RUN8
    { SPR_VILE,    4,                     2, A_VileChase,            S_VILE_RUN10        }, // S_VILE_RUN9
    { SPR_VILE,    4,                     2, A_VileChase,            S_VILE_RUN11        }, // S_VILE_RUN10
    { SPR_VILE,    5,                     2, A_VileChase,            S_VILE_RUN12        }, // S_VILE_RUN11
    { SPR_VILE,    5,                     2, A_VileChase,            S_VILE_RUN1         }, // S_VILE_RUN12
    { SPR_VILE,   (6 | FF_FULLBRIGHT),    0, A_VileStart,            S_VILE_ATK2         }, // S_VILE_ATK1
    { SPR_VILE,   (6 | FF_FULLBRIGHT),   10, A_FaceTarget,           S_VILE_ATK3         }, // S_VILE_ATK2
    { SPR_VILE,   (7 | FF_FULLBRIGHT),    8, A_VileTarget,           S_VILE_ATK4         }, // S_VILE_ATK3
    { SPR_VILE,   (8 | FF_FULLBRIGHT),    8, A_FaceTarget,           S_VILE_ATK5         }, // S_VILE_ATK4
    { SPR_VILE,   (9 | FF_FULLBRIGHT),    8, A_FaceTarget,           S_VILE_ATK6         }, // S_VILE_ATK5
    { SPR_VILE,  (10 | FF_FULLBRIGHT),    8, A_FaceTarget,           S_VILE_ATK7         }, // S_VILE_ATK6
    { SPR_VILE,  (11 | FF_FULLBRIGHT),    8, A_FaceTarget,           S_VILE_ATK8         }, // S_VILE_ATK7
    { SPR_VILE,  (12 | FF_FULLBRIGHT),    8, A_FaceTarget,           S_VILE_ATK9         }, // S_VILE_ATK8
    { SPR_VILE,  (13 | FF_FULLBRIGHT),    8, A_FaceTarget,           S_VILE_ATK10        }, // S_VILE_ATK9
    { SPR_VILE,  (14 | FF_FULLBRIGHT),    8, A_VileAttack,           S_VILE_ATK11        }, // S_VILE_ATK10
    { SPR_VILE,  (15 | FF_FULLBRIGHT),   20, NULL,                   S_VILE_RUN1         }, // S_VILE_ATK11
    { SPR_VILE,  (26 | FF_FULLBRIGHT),   10, NULL,                   S_VILE_HEAL2        }, // S_VILE_HEAL1
    { SPR_VILE,  (27 | FF_FULLBRIGHT),   10, NULL,                   S_VILE_HEAL3        }, // S_VILE_HEAL2
    { SPR_VILE,  (28 | FF_FULLBRIGHT),   10, NULL,                   S_VILE_RUN1         }, // S_VILE_HEAL3
    { SPR_VILE,   16,                     5, NULL,                   S_VILE_PAIN2        }, // S_VILE_PAIN
    { SPR_VILE,   16,                     5, A_Pain,                 S_VILE_RUN1         }, // S_VILE_PAIN2
    { SPR_VILE,   16,                     7, NULL,                   S_VILE_DIE2         }, // S_VILE_DIE1
    { SPR_VILE,   17,                     7, A_Scream,               S_VILE_DIE3         }, // S_VILE_DIE2
    { SPR_VILE,   18,                     7, A_Fall,                 S_VILE_DIE4         }, // S_VILE_DIE3
    { SPR_VILE,   19,                     7, NULL,                   S_VILE_DIE5         }, // S_VILE_DIE4
    { SPR_VILE,   20,                     7, NULL,                   S_VILE_DIE6         }, // S_VILE_DIE5
    { SPR_VILE,   21,                     7, NULL,                   S_VILE_DIE7         }, // S_VILE_DIE6
    { SPR_VILE,   22,                     7, NULL,                   S_VILE_DIE8         }, // S_VILE_DIE7
    { SPR_VILE,   23,                     5, NULL,                   S_VILE_DIE9         }, // S_VILE_DIE8
    { SPR_VILE,   24,                     5, NULL,                   S_VILE_DIE10        }, // S_VILE_DIE9
    { SPR_VILE,   25,                    -1, NULL,                   S_NULL              }, // S_VILE_DIE10

    // Arch-vile Fire Attack (MT_FIRE)
    { SPR_FIRE,   (0 | FF_FULLBRIGHT),    2, A_StartFire,            S_FIRE2             }, // S_FIRE1
    { SPR_FIRE,   (1 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE3             }, // S_FIRE2
    { SPR_FIRE,   (0 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE4             }, // S_FIRE3
    { SPR_FIRE,   (1 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE5             }, // S_FIRE4
    { SPR_FIRE,   (2 | FF_FULLBRIGHT),    2, A_FireCrackle,          S_FIRE6             }, // S_FIRE5
    { SPR_FIRE,   (1 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE7             }, // S_FIRE6
    { SPR_FIRE,   (2 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE8             }, // S_FIRE7
    { SPR_FIRE,   (1 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE9             }, // S_FIRE8
    { SPR_FIRE,   (2 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE10            }, // S_FIRE9
    { SPR_FIRE,   (3 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE11            }, // S_FIRE10
    { SPR_FIRE,   (2 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE12            }, // S_FIRE11
    { SPR_FIRE,   (3 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE13            }, // S_FIRE12
    { SPR_FIRE,   (2 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE14            }, // S_FIRE13
    { SPR_FIRE,   (3 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE15            }, // S_FIRE14
    { SPR_FIRE,   (4 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE16            }, // S_FIRE15
    { SPR_FIRE,   (3 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE17            }, // S_FIRE16
    { SPR_FIRE,   (4 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE18            }, // S_FIRE17
    { SPR_FIRE,   (3 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE19            }, // S_FIRE18
    { SPR_FIRE,   (4 | FF_FULLBRIGHT),    2, A_FireCrackle,          S_FIRE20            }, // S_FIRE19
    { SPR_FIRE,   (5 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE21            }, // S_FIRE20
    { SPR_FIRE,   (4 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE22            }, // S_FIRE21
    { SPR_FIRE,   (5 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE23            }, // S_FIRE22
    { SPR_FIRE,   (4 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE24            }, // S_FIRE23
    { SPR_FIRE,   (5 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE25            }, // S_FIRE24
    { SPR_FIRE,   (6 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE26            }, // S_FIRE25
    { SPR_FIRE,   (7 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE27            }, // S_FIRE26
    { SPR_FIRE,   (6 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE28            }, // S_FIRE27
    { SPR_FIRE,   (7 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE29            }, // S_FIRE28
    { SPR_FIRE,   (6 | FF_FULLBRIGHT),    2, A_Fire,                 S_FIRE30            }, // S_FIRE29
    { SPR_FIRE,   (7 | FF_FULLBRIGHT),    2, A_Fire,                 S_NULL              }, // S_FIRE30

    // Smoke (MT_SMOKE)
    { SPR_PUFF,    1,                     4, NULL,                   S_SMOKE2            }, // S_SMOKE1
    { SPR_PUFF,    2,                     4, NULL,                   S_SMOKE3            }, // S_SMOKE2
    { SPR_PUFF,    1,                     4, NULL,                   S_SMOKE4            }, // S_SMOKE3
    { SPR_PUFF,    2,                     4, NULL,                   S_SMOKE5            }, // S_SMOKE4
    { SPR_PUFF,    3,                     4, NULL,                   S_NULL              }, // S_SMOKE5

    // Revenant Projectile (MT_TRACER)
    { SPR_FATB,   (0 | FF_FULLBRIGHT),    2, A_Tracer,               S_TRACER2           }, // S_TRACER
    { SPR_FATB,   (1 | FF_FULLBRIGHT),    2, A_Tracer,               S_TRACER            }, // S_TRACER2
    { SPR_FBXP,   (0 | FF_FULLBRIGHT),    8, NULL,                   S_TRACEEXP2         }, // S_TRACEEXP1
    { SPR_FBXP,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_TRACEEXP3         }, // S_TRACEEXP2
    { SPR_FBXP,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_NULL              }, // S_TRACEEXP3

    // Revenant (MT_UNDEAD)
    { SPR_SKEL,    0,                    10, A_Look,                 S_SKEL_STND2        }, // S_SKEL_STND
    { SPR_SKEL,    1,                    10, A_Look,                 S_SKEL_STND         }, // S_SKEL_STND2
    { SPR_SKEL,    0,                     2, A_Chase,                S_SKEL_RUN2         }, // S_SKEL_RUN1
    { SPR_SKEL,    0,                     2, A_Chase,                S_SKEL_RUN3         }, // S_SKEL_RUN2
    { SPR_SKEL,    1,                     2, A_Chase,                S_SKEL_RUN4         }, // S_SKEL_RUN3
    { SPR_SKEL,    1,                     2, A_Chase,                S_SKEL_RUN5         }, // S_SKEL_RUN4
    { SPR_SKEL,    2,                     2, A_Chase,                S_SKEL_RUN6         }, // S_SKEL_RUN5
    { SPR_SKEL,    2,                     2, A_Chase,                S_SKEL_RUN7         }, // S_SKEL_RUN6
    { SPR_SKEL,    3,                     2, A_Chase,                S_SKEL_RUN8         }, // S_SKEL_RUN7
    { SPR_SKEL,    3,                     2, A_Chase,                S_SKEL_RUN9         }, // S_SKEL_RUN8
    { SPR_SKEL,    4,                     2, A_Chase,                S_SKEL_RUN10        }, // S_SKEL_RUN9
    { SPR_SKEL,    4,                     2, A_Chase,                S_SKEL_RUN11        }, // S_SKEL_RUN10
    { SPR_SKEL,    5,                     2, A_Chase,                S_SKEL_RUN12        }, // S_SKEL_RUN11
    { SPR_SKEL,    5,                     2, A_Chase,                S_SKEL_RUN1         }, // S_SKEL_RUN12
    { SPR_SKEL,    6,                     0, A_FaceTarget,           S_SKEL_FIST2        }, // S_SKEL_FIST1
    { SPR_SKEL,    6,                     6, A_SkelWhoosh,           S_SKEL_FIST3        }, // S_SKEL_FIST2
    { SPR_SKEL,    7,                     6, A_FaceTarget,           S_SKEL_FIST4        }, // S_SKEL_FIST3
    { SPR_SKEL,    8,                     6, A_SkelFist,             S_SKEL_RUN1         }, // S_SKEL_FIST4
    { SPR_SKEL,   (9 | FF_FULLBRIGHT),    0, A_FaceTarget,           S_SKEL_MISS2        }, // S_SKEL_MISS1
    { SPR_SKEL,   (9 | FF_FULLBRIGHT),   10, A_FaceTarget,           S_SKEL_MISS3        }, // S_SKEL_MISS2
    { SPR_SKEL,   10,                    10, A_SkelMissile,          S_SKEL_MISS4        }, // S_SKEL_MISS3
    { SPR_SKEL,   10,                    10, A_FaceTarget,           S_SKEL_RUN1         }, // S_SKEL_MISS4
    { SPR_SKEL,   11,                     5, NULL,                   S_SKEL_PAIN2        }, // S_SKEL_PAIN
    { SPR_SKEL,   11,                     5, A_Pain,                 S_SKEL_RUN1         }, // S_SKEL_PAIN2
    { SPR_SKEL,   11,                     7, NULL,                   S_SKEL_DIE2         }, // S_SKEL_DIE1
    { SPR_SKEL,   12,                     7, NULL,                   S_SKEL_DIE3         }, // S_SKEL_DIE2
    { SPR_SKEL,   13,                     7, A_Scream,               S_SKEL_DIE4         }, // S_SKEL_DIE3
    { SPR_SKEL,   14,                     7, A_Fall,                 S_SKEL_DIE5         }, // S_SKEL_DIE4
    { SPR_SKEL,   15,                     7, NULL,                   S_SKEL_DIE6         }, // S_SKEL_DIE5
    { SPR_SKEL,   16,                    -1, NULL,                   S_NULL              }, // S_SKEL_DIE6
    { SPR_SKEL,   16,                     5, NULL,                   S_SKEL_RAISE2       }, // S_SKEL_RAISE1
    { SPR_SKEL,   15,                     5, NULL,                   S_SKEL_RAISE3       }, // S_SKEL_RAISE2
    { SPR_SKEL,   14,                     5, NULL,                   S_SKEL_RAISE4       }, // S_SKEL_RAISE3
    { SPR_SKEL,   13,                     5, NULL,                   S_SKEL_RAISE5       }, // S_SKEL_RAISE4
    { SPR_SKEL,   12,                     5, NULL,                   S_SKEL_RAISE6       }, // S_SKEL_RAISE5
    { SPR_SKEL,   11,                     5, NULL,                   S_SKEL_RUN1         }, // S_SKEL_RAISE6

    // Mancubus Projectile (MT_FATSHOT)
    { SPR_MANF,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_FATSHOT2          }, // S_FATSHOT1
    { SPR_MANF,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_FATSHOT1          }, // S_FATSHOT2
    { SPR_MISL,   (1 | FF_FULLBRIGHT),    8, NULL,                   S_FATSHOTX2         }, // S_FATSHOTX1
    { SPR_MISL,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_FATSHOTX3         }, // S_FATSHOTX2
    { SPR_MISL,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_NULL              }, // S_FATSHOTX3

    // Mancubus (MT_FATSO)
    { SPR_FATT,    0,                    15, A_Look,                 S_FATT_STND2        }, // S_FATT_STND
    { SPR_FATT,    1,                    15, A_Look,                 S_FATT_STND         }, // S_FATT_STND2
    { SPR_FATT,    0,                     4, A_Chase,                S_FATT_RUN2         }, // S_FATT_RUN1
    { SPR_FATT,    0,                     4, A_Chase,                S_FATT_RUN3         }, // S_FATT_RUN2
    { SPR_FATT,    1,                     4, A_Chase,                S_FATT_RUN4         }, // S_FATT_RUN3
    { SPR_FATT,    1,                     4, A_Chase,                S_FATT_RUN5         }, // S_FATT_RUN4
    { SPR_FATT,    2,                     4, A_Chase,                S_FATT_RUN6         }, // S_FATT_RUN5
    { SPR_FATT,    2,                     4, A_Chase,                S_FATT_RUN7         }, // S_FATT_RUN6
    { SPR_FATT,    3,                     4, A_Chase,                S_FATT_RUN8         }, // S_FATT_RUN7
    { SPR_FATT,    3,                     4, A_Chase,                S_FATT_RUN9         }, // S_FATT_RUN8
    { SPR_FATT,    4,                     4, A_Chase,                S_FATT_RUN10        }, // S_FATT_RUN9
    { SPR_FATT,    4,                     4, A_Chase,                S_FATT_RUN11        }, // S_FATT_RUN10
    { SPR_FATT,    5,                     4, A_Chase,                S_FATT_RUN12        }, // S_FATT_RUN11
    { SPR_FATT,    5,                     4, A_Chase,                S_FATT_RUN1         }, // S_FATT_RUN12
    { SPR_FATT,    6,                    20, A_FatRaise,             S_FATT_ATK2         }, // S_FATT_ATK1
    { SPR_FATT,   (7 | FF_FULLBRIGHT),   10, A_FatAttack1,           S_FATT_ATK3         }, // S_FATT_ATK2
    { SPR_FATT,    8,                     5, A_FaceTarget,           S_FATT_ATK4         }, // S_FATT_ATK3
    { SPR_FATT,    6,                     5, A_FaceTarget,           S_FATT_ATK5         }, // S_FATT_ATK4
    { SPR_FATT,   (7 | FF_FULLBRIGHT),   10, A_FatAttack2,           S_FATT_ATK6         }, // S_FATT_ATK5
    { SPR_FATT,    8,                     5, A_FaceTarget,           S_FATT_ATK7         }, // S_FATT_ATK6
    { SPR_FATT,    6,                     5, A_FaceTarget,           S_FATT_ATK8         }, // S_FATT_ATK7
    { SPR_FATT,   (7 | FF_FULLBRIGHT),   10, A_FatAttack3,           S_FATT_ATK9         }, // S_FATT_ATK8
    { SPR_FATT,    8,                     5, A_FaceTarget,           S_FATT_ATK10        }, // S_FATT_ATK9
    { SPR_FATT,    6,                     5, A_FaceTarget,           S_FATT_RUN1         }, // S_FATT_ATK10
    { SPR_FATT,    9,                     3, NULL,                   S_FATT_PAIN2        }, // S_FATT_PAIN
    { SPR_FATT,    9,                     3, A_Pain,                 S_FATT_RUN1         }, // S_FATT_PAIN2
    { SPR_FATT,   10,                     6, NULL,                   S_FATT_DIE2         }, // S_FATT_DIE1
    { SPR_FATT,   11,                     6, A_Scream,               S_FATT_DIE3         }, // S_FATT_DIE2
    { SPR_FATT,   12,                     6, A_Fall,                 S_FATT_DIE4         }, // S_FATT_DIE3
    { SPR_FATT,   13,                     6, NULL,                   S_FATT_DIE5         }, // S_FATT_DIE4
    { SPR_FATT,   14,                     6, NULL,                   S_FATT_DIE6         }, // S_FATT_DIE5
    { SPR_FATT,   15,                     6, NULL,                   S_FATT_DIE7         }, // S_FATT_DIE6
    { SPR_FATT,   16,                     6, NULL,                   S_FATT_DIE8         }, // S_FATT_DIE7
    { SPR_FATT,   17,                     6, NULL,                   S_FATT_DIE9         }, // S_FATT_DIE8
    { SPR_FATT,   18,                     6, NULL,                   S_FATT_DIE10        }, // S_FATT_DIE9
    { SPR_FATT,   19,                    -1, A_BossDeath,            S_NULL              }, // S_FATT_DIE10
    { SPR_FATT,   17,                     5, NULL,                   S_FATT_RAISE2       }, // S_FATT_RAISE1
    { SPR_FATT,   16,                     5, NULL,                   S_FATT_RAISE3       }, // S_FATT_RAISE2
    { SPR_FATT,   15,                     5, NULL,                   S_FATT_RAISE4       }, // S_FATT_RAISE3
    { SPR_FATT,   14,                     5, NULL,                   S_FATT_RAISE5       }, // S_FATT_RAISE4
    { SPR_FATT,   13,                     5, NULL,                   S_FATT_RAISE6       }, // S_FATT_RAISE5
    { SPR_FATT,   12,                     5, NULL,                   S_FATT_RAISE7       }, // S_FATT_RAISE6
    { SPR_FATT,   11,                     5, NULL,                   S_FATT_RAISE8       }, // S_FATT_RAISE7
    { SPR_FATT,   10,                     5, NULL,                   S_FATT_RUN1         }, // S_FATT_RAISE8

    // Chaingunner (MT_CHAINGUY)
    { SPR_CPOS,    0,                    10, A_Look,                 S_CPOS_STND2        }, // S_CPOS_STND
    { SPR_CPOS,    1,                    10, A_Look,                 S_CPOS_STND         }, // S_CPOS_STND2
    { SPR_CPOS,    0,                     3, A_Chase,                S_CPOS_RUN2         }, // S_CPOS_RUN1
    { SPR_CPOS,    0,                     3, A_Chase,                S_CPOS_RUN3         }, // S_CPOS_RUN2
    { SPR_CPOS,    1,                     3, A_Chase,                S_CPOS_RUN4         }, // S_CPOS_RUN3
    { SPR_CPOS,    1,                     3, A_Chase,                S_CPOS_RUN5         }, // S_CPOS_RUN4
    { SPR_CPOS,    2,                     3, A_Chase,                S_CPOS_RUN6         }, // S_CPOS_RUN5
    { SPR_CPOS,    2,                     3, A_Chase,                S_CPOS_RUN7         }, // S_CPOS_RUN6
    { SPR_CPOS,    3,                     3, A_Chase,                S_CPOS_RUN8         }, // S_CPOS_RUN7
    { SPR_CPOS,    3,                     3, A_Chase,                S_CPOS_RUN1         }, // S_CPOS_RUN8
    { SPR_CPOS,    4,                    10, A_FaceTarget,           S_CPOS_ATK2         }, // S_CPOS_ATK1
    { SPR_CPOS,   (5 | FF_FULLBRIGHT),    4, A_CPosAttack,           S_CPOS_ATK3         }, // S_CPOS_ATK2
    { SPR_CPOS,    4,                     4, A_CPosAttack,           S_CPOS_ATK4         }, // S_CPOS_ATK3
    { SPR_CPOS,   (5 | FF_FULLBRIGHT),    1, A_CPosRefire,           S_CPOS_ATK2         }, // S_CPOS_ATK4
    { SPR_CPOS,    6,                     3, NULL,                   S_CPOS_PAIN2        }, // S_CPOS_PAIN
    { SPR_CPOS,    6,                     3, A_Pain,                 S_CPOS_RUN1         }, // S_CPOS_PAIN2
    { SPR_CPOS,    7,                     5, NULL,                   S_CPOS_DIE2         }, // S_CPOS_DIE1
    { SPR_CPOS,    8,                     5, A_Scream,               S_CPOS_DIE3         }, // S_CPOS_DIE2
    { SPR_CPOS,    9,                     5, A_Fall,                 S_CPOS_DIE4         }, // S_CPOS_DIE3
    { SPR_CPOS,   10,                     5, NULL,                   S_CPOS_DIE5         }, // S_CPOS_DIE4
    { SPR_CPOS,   11,                     5, NULL,                   S_CPOS_DIE6         }, // S_CPOS_DIE5
    { SPR_CPOS,   12,                     5, NULL,                   S_CPOS_DIE7         }, // S_CPOS_DIE6
    { SPR_CPOS,   13,                    -1, NULL,                   S_NULL              }, // S_CPOS_DIE7
    { SPR_CPOS,   14,                     5, NULL,                   S_CPOS_XDIE2        }, // S_CPOS_XDIE1
    { SPR_CPOS,   15,                     5, A_XScream,              S_CPOS_XDIE3        }, // S_CPOS_XDIE2
    { SPR_CPOS,   16,                     5, A_Fall,                 S_CPOS_XDIE4        }, // S_CPOS_XDIE3
    { SPR_CPOS,   17,                     5, NULL,                   S_CPOS_XDIE5        }, // S_CPOS_XDIE4
    { SPR_CPOS,   18,                     5, NULL,                   S_CPOS_XDIE6        }, // S_CPOS_XDIE5
    { SPR_CPOS,   19,                    -1, NULL,                   S_NULL              }, // S_CPOS_XDIE6
    { SPR_CPOS,   13,                     5, NULL,                   S_CPOS_RAISE2       }, // S_CPOS_RAISE1
    { SPR_CPOS,   12,                     5, NULL,                   S_CPOS_RAISE3       }, // S_CPOS_RAISE2
    { SPR_CPOS,   11,                     5, NULL,                   S_CPOS_RAISE4       }, // S_CPOS_RAISE3
    { SPR_CPOS,   10,                     5, NULL,                   S_CPOS_RAISE5       }, // S_CPOS_RAISE4
    { SPR_CPOS,    9,                     5, NULL,                   S_CPOS_RAISE6       }, // S_CPOS_RAISE5
    { SPR_CPOS,    8,                     5, NULL,                   S_CPOS_RAISE7       }, // S_CPOS_RAISE6
    { SPR_CPOS,    7,                     5, NULL,                   S_CPOS_RUN1         }, // S_CPOS_RAISE7

    // Imp (MT_TROOP)
    { SPR_TROO,    0,                    10, A_Look,                 S_TROO_STND2        }, // S_TROO_STND
    { SPR_TROO,    1,                    10, A_Look,                 S_TROO_STND         }, // S_TROO_STND2
    { SPR_TROO,    0,                     3, A_Chase,                S_TROO_RUN2         }, // S_TROO_RUN1
    { SPR_TROO,    0,                     3, A_Chase,                S_TROO_RUN3         }, // S_TROO_RUN2
    { SPR_TROO,    1,                     3, A_Chase,                S_TROO_RUN4         }, // S_TROO_RUN3
    { SPR_TROO,    1,                     3, A_Chase,                S_TROO_RUN5         }, // S_TROO_RUN4
    { SPR_TROO,    2,                     3, A_Chase,                S_TROO_RUN6         }, // S_TROO_RUN5
    { SPR_TROO,    2,                     3, A_Chase,                S_TROO_RUN7         }, // S_TROO_RUN6
    { SPR_TROO,    3,                     3, A_Chase,                S_TROO_RUN8         }, // S_TROO_RUN7
    { SPR_TROO,    3,                     3, A_Chase,                S_TROO_RUN1         }, // S_TROO_RUN8
    { SPR_TROO,    4,                     8, A_FaceTarget,           S_TROO_ATK2         }, // S_TROO_ATK1
    { SPR_TROO,    5,                     8, A_FaceTarget,           S_TROO_ATK3         }, // S_TROO_ATK2
    { SPR_TROO,    6,                     6, A_TroopAttack,          S_TROO_RUN1         }, // S_TROO_ATK3
    { SPR_TROO,    7,                     2, NULL,                   S_TROO_PAIN2        }, // S_TROO_PAIN
    { SPR_TROO,    7,                     2, A_Pain,                 S_TROO_RUN1         }, // S_TROO_PAIN2
    { SPR_TROO,    8,                     8, NULL,                   S_TROO_DIE2         }, // S_TROO_DIE1
    { SPR_TROO,    9,                     8, A_Scream,               S_TROO_DIE3         }, // S_TROO_DIE2
    { SPR_TROO,   10,                     6, NULL,                   S_TROO_DIE4         }, // S_TROO_DIE3
    { SPR_TROO,   11,                     6, A_Fall,                 S_TROO_DIE5         }, // S_TROO_DIE4

    // Imp Death (MT_MISC66)
    { SPR_TROO,   12,                    -1, NULL,                   S_NULL              }, // S_TROO_DIE5
    { SPR_TROO,   13,                     5, NULL,                   S_TROO_XDIE2        }, // S_TROO_XDIE1
    { SPR_TROO,   14,                     5, A_XScream,              S_TROO_XDIE3        }, // S_TROO_XDIE2
    { SPR_TROO,   15,                     5, NULL,                   S_TROO_XDIE4        }, // S_TROO_XDIE3
    { SPR_TROO,   16,                     5, A_Fall,                 S_TROO_XDIE5        }, // S_TROO_XDIE4
    { SPR_TROO,   17,                     5, NULL,                   S_TROO_XDIE6        }, // S_TROO_XDIE5
    { SPR_TROO,   18,                     5, NULL,                   S_TROO_XDIE7        }, // S_TROO_XDIE6
    { SPR_TROO,   19,                     5, NULL,                   S_TROO_XDIE8        }, // S_TROO_XDIE7
    { SPR_TROO,   20,                    -1, NULL,                   S_NULL              }, // S_TROO_XDIE8
    { SPR_TROO,   12,                     8, NULL,                   S_TROO_RAISE2       }, // S_TROO_RAISE1
    { SPR_TROO,   11,                     8, NULL,                   S_TROO_RAISE3       }, // S_TROO_RAISE2
    { SPR_TROO,   10,                     6, NULL,                   S_TROO_RAISE4       }, // S_TROO_RAISE3
    { SPR_TROO,    9,                     6, NULL,                   S_TROO_RAISE5       }, // S_TROO_RAISE4
    { SPR_TROO,    8,                     6, NULL,                   S_TROO_RUN1         }, // S_TROO_RAISE5

    // Demon (MT_SERGEANT) and Spectre (MT_SHADOWS)
    { SPR_SARG,    0,                    10, A_Look,                 S_SARG_STND2        }, // S_SARG_STND
    { SPR_SARG,    1,                    10, A_Look,                 S_SARG_STND         }, // S_SARG_STND2
    { SPR_SARG,    0,                     2, A_Chase,                S_SARG_RUN2         }, // S_SARG_RUN1
    { SPR_SARG,    0,                     2, A_Chase,                S_SARG_RUN3         }, // S_SARG_RUN2
    { SPR_SARG,    1,                     2, A_Chase,                S_SARG_RUN4         }, // S_SARG_RUN3
    { SPR_SARG,    1,                     2, A_Chase,                S_SARG_RUN5         }, // S_SARG_RUN4
    { SPR_SARG,    2,                     2, A_Chase,                S_SARG_RUN6         }, // S_SARG_RUN5
    { SPR_SARG,    2,                     2, A_Chase,                S_SARG_RUN7         }, // S_SARG_RUN6
    { SPR_SARG,    3,                     2, A_Chase,                S_SARG_RUN8         }, // S_SARG_RUN7
    { SPR_SARG,    3,                     2, A_Chase,                S_SARG_RUN1         }, // S_SARG_RUN8
    { SPR_SARG,    4,                     8, A_FaceTarget,           S_SARG_ATK2         }, // S_SARG_ATK1
    { SPR_SARG,    5,                     8, A_FaceTarget,           S_SARG_ATK3         }, // S_SARG_ATK2
    { SPR_SARG,    6,                     8, A_SargAttack,           S_SARG_RUN1         }, // S_SARG_ATK3
    { SPR_SARG,    7,                     2, NULL,                   S_SARG_PAIN2        }, // S_SARG_PAIN
    { SPR_SARG,    7,                     2, A_Pain,                 S_SARG_RUN1         }, // S_SARG_PAIN2
    { SPR_SARG,    8,                     8, NULL,                   S_SARG_DIE2         }, // S_SARG_DIE1
    { SPR_SARG,    9,                     8, A_Scream,               S_SARG_DIE3         }, // S_SARG_DIE2
    { SPR_SARG,   10,                     4, NULL,                   S_SARG_DIE4         }, // S_SARG_DIE3
    { SPR_SARG,   11,                     4, A_Fall,                 S_SARG_DIE5         }, // S_SARG_DIE4
    { SPR_SARG,   12,                     4, NULL,                   S_SARG_DIE6         }, // S_SARG_DIE5

    // Demon Death (MT_MISC64)
    { SPR_SARG,   13,                    -1, NULL,                   S_NULL              }, // S_SARG_DIE6
    { SPR_SARG,   13,                     5, NULL,                   S_SARG_RAISE2       }, // S_SARG_RAISE1
    { SPR_SARG,   12,                     5, NULL,                   S_SARG_RAISE3       }, // S_SARG_RAISE2
    { SPR_SARG,   11,                     5, NULL,                   S_SARG_RAISE4       }, // S_SARG_RAISE3
    { SPR_SARG,   10,                     5, NULL,                   S_SARG_RAISE5       }, // S_SARG_RAISE4
    { SPR_SARG,    9,                     5, NULL,                   S_SARG_RAISE6       }, // S_SARG_RAISE5
    { SPR_SARG,    8,                     5, NULL,                   S_SARG_RUN1         }, // S_SARG_RAISE6

    // Cacodemon (MT_HEAD)
    { SPR_HEAD,    0,                    10, A_Look,                 S_HEAD_STND         }, // S_HEAD_STND
    { SPR_HEAD,    0,                     3, A_Chase,                S_HEAD_RUN1         }, // S_HEAD_RUN1
    { SPR_HEAD,    1,                     5, A_FaceTarget,           S_HEAD_ATK2         }, // S_HEAD_ATK1
    { SPR_HEAD,    2,                     5, A_FaceTarget,           S_HEAD_ATK3         }, // S_HEAD_ATK2
    { SPR_HEAD,    3,                     5, A_HeadAttack,           S_HEAD_RUN1         }, // S_HEAD_ATK3
    { SPR_HEAD,    4,                     3, NULL,                   S_HEAD_PAIN2        }, // S_HEAD_PAIN
    { SPR_HEAD,    4,                     3, A_Pain,                 S_HEAD_PAIN3        }, // S_HEAD_PAIN2
    { SPR_HEAD,    5,                     6, NULL,                   S_HEAD_RUN1         }, // S_HEAD_PAIN3
    { SPR_HEAD,    6,                     8, NULL,                   S_HEAD_DIE2         }, // S_HEAD_DIE1
    { SPR_HEAD,    7,                     8, A_Scream,               S_HEAD_DIE3         }, // S_HEAD_DIE2
    { SPR_HEAD,    8,                     8, NULL,                   S_HEAD_DIE4         }, // S_HEAD_DIE3
    { SPR_HEAD,    9,                     8, NULL,                   S_HEAD_DIE5         }, // S_HEAD_DIE4
    { SPR_HEAD,   10,                     8, A_Fall,                 S_HEAD_DIE6         }, // S_HEAD_DIE5

    // Cacodemon Death (MT_MISC61)
    { SPR_HEAD,   11,                    -1, NULL,                   S_NULL              }, // S_HEAD_DIE6
    { SPR_HEAD,   11,                     8, NULL,                   S_HEAD_RAISE2       }, // S_HEAD_RAISE1
    { SPR_HEAD,   10,                     8, NULL,                   S_HEAD_RAISE3       }, // S_HEAD_RAISE2
    { SPR_HEAD,    9,                     8, NULL,                   S_HEAD_RAISE4       }, // S_HEAD_RAISE3
    { SPR_HEAD,    8,                     8, NULL,                   S_HEAD_RAISE5       }, // S_HEAD_RAISE4
    { SPR_HEAD,    7,                     8, NULL,                   S_HEAD_RAISE6       }, // S_HEAD_RAISE5
    { SPR_HEAD,    6,                     8, NULL,                   S_HEAD_RUN1         }, // S_HEAD_RAISE6

    // Baron of Hell and Hell Knight Projectiles (MT_BRUISERSHOT)
    { SPR_BAL7,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_BRBALL2           }, // S_BRBALL1
    { SPR_BAL7,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_BRBALL1           }, // S_BRBALL2
    { SPR_BAL7,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_BRBALLX2          }, // S_BRBALLX1
    { SPR_BAL7,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_BRBALLX3          }, // S_BRBALLX2
    { SPR_BAL7,   (4 | FF_FULLBRIGHT),    6, NULL,                   S_NULL              }, // S_BRBALLX3

    // Baron of Hell (MT_BRUISER)
    { SPR_BOSS,    0,                    10, A_Look,                 S_BOSS_STND2        }, // S_BOSS_STND
    { SPR_BOSS,    1,                    10, A_Look,                 S_BOSS_STND         }, // S_BOSS_STND2
    { SPR_BOSS,    0,                     3, A_Chase,                S_BOSS_RUN2         }, // S_BOSS_RUN1
    { SPR_BOSS,    0,                     3, A_Chase,                S_BOSS_RUN3         }, // S_BOSS_RUN2
    { SPR_BOSS,    1,                     3, A_Chase,                S_BOSS_RUN4         }, // S_BOSS_RUN3
    { SPR_BOSS,    1,                     3, A_Chase,                S_BOSS_RUN5         }, // S_BOSS_RUN4
    { SPR_BOSS,    2,                     3, A_Chase,                S_BOSS_RUN6         }, // S_BOSS_RUN5
    { SPR_BOSS,    2,                     3, A_Chase,                S_BOSS_RUN7         }, // S_BOSS_RUN6
    { SPR_BOSS,    3,                     3, A_Chase,                S_BOSS_RUN8         }, // S_BOSS_RUN7
    { SPR_BOSS,    3,                     3, A_Chase,                S_BOSS_RUN1         }, // S_BOSS_RUN8
    { SPR_BOSS,    4,                     8, A_FaceTarget,           S_BOSS_ATK2         }, // S_BOSS_ATK1
    { SPR_BOSS,    5,                     8, A_FaceTarget,           S_BOSS_ATK3         }, // S_BOSS_ATK2
    { SPR_BOSS,    6,                     8, A_BruisAttack,          S_BOSS_RUN1         }, // S_BOSS_ATK3
    { SPR_BOSS,    7,                     2, NULL,                   S_BOSS_PAIN2        }, // S_BOSS_PAIN
    { SPR_BOSS,    7,                     2, A_Pain,                 S_BOSS_RUN1         }, // S_BOSS_PAIN2
    { SPR_BOSS,    8,                     8, NULL,                   S_BOSS_DIE2         }, // S_BOSS_DIE1
    { SPR_BOSS,    9,                     8, A_Scream,               S_BOSS_DIE3         }, // S_BOSS_DIE2
    { SPR_BOSS,   10,                     8, NULL,                   S_BOSS_DIE4         }, // S_BOSS_DIE3
    { SPR_BOSS,   11,                     8, A_Fall,                 S_BOSS_DIE5         }, // S_BOSS_DIE4
    { SPR_BOSS,   12,                     8, NULL,                   S_BOSS_DIE6         }, // S_BOSS_DIE5
    { SPR_BOSS,   13,                     8, NULL,                   S_BOSS_DIE7         }, // S_BOSS_DIE6
    { SPR_BOSS,   14,                    -1, A_BossDeath,            S_NULL              }, // S_BOSS_DIE7
    { SPR_BOSS,   14,                     8, NULL,                   S_BOSS_RAISE2       }, // S_BOSS_RAISE1
    { SPR_BOSS,   13,                     8, NULL,                   S_BOSS_RAISE3       }, // S_BOSS_RAISE2
    { SPR_BOSS,   12,                     8, NULL,                   S_BOSS_RAISE4       }, // S_BOSS_RAISE3
    { SPR_BOSS,   11,                     8, NULL,                   S_BOSS_RAISE5       }, // S_BOSS_RAISE4
    { SPR_BOSS,   10,                     8, NULL,                   S_BOSS_RAISE6       }, // S_BOSS_RAISE5
    { SPR_BOSS,    9,                     8, NULL,                   S_BOSS_RAISE7       }, // S_BOSS_RAISE6
    { SPR_BOSS,    8,                     8, NULL,                   S_BOSS_RUN1         }, // S_BOSS_RAISE7

    // Hell Knight (MT_KNIGHT)
    { SPR_BOS2,    0,                    10, A_Look,                 S_BOS2_STND2        }, // S_BOS2_STND
    { SPR_BOS2,    1,                    10, A_Look,                 S_BOS2_STND         }, // S_BOS2_STND2
    { SPR_BOS2,    0,                     3, A_Chase,                S_BOS2_RUN2         }, // S_BOS2_RUN1
    { SPR_BOS2,    0,                     3, A_Chase,                S_BOS2_RUN3         }, // S_BOS2_RUN2
    { SPR_BOS2,    1,                     3, A_Chase,                S_BOS2_RUN4         }, // S_BOS2_RUN3
    { SPR_BOS2,    1,                     3, A_Chase,                S_BOS2_RUN5         }, // S_BOS2_RUN4
    { SPR_BOS2,    2,                     3, A_Chase,                S_BOS2_RUN6         }, // S_BOS2_RUN5
    { SPR_BOS2,    2,                     3, A_Chase,                S_BOS2_RUN7         }, // S_BOS2_RUN6
    { SPR_BOS2,    3,                     3, A_Chase,                S_BOS2_RUN8         }, // S_BOS2_RUN7
    { SPR_BOS2,    3,                     3, A_Chase,                S_BOS2_RUN1         }, // S_BOS2_RUN8
    { SPR_BOS2,    4,                     8, A_FaceTarget,           S_BOS2_ATK2         }, // S_BOS2_ATK1
    { SPR_BOS2,    5,                     8, A_FaceTarget,           S_BOS2_ATK3         }, // S_BOS2_ATK2
    { SPR_BOS2,    6,                     8, A_BruisAttack,          S_BOS2_RUN1         }, // S_BOS2_ATK3
    { SPR_BOS2,    7,                     2, NULL,                   S_BOS2_PAIN2        }, // S_BOS2_PAIN
    { SPR_BOS2,    7,                     2, A_Pain,                 S_BOS2_RUN1         }, // S_BOS2_PAIN2
    { SPR_BOS2,    8,                     8, NULL,                   S_BOS2_DIE2         }, // S_BOS2_DIE1
    { SPR_BOS2,    9,                     8, A_Scream,               S_BOS2_DIE3         }, // S_BOS2_DIE2
    { SPR_BOS2,   10,                     8, NULL,                   S_BOS2_DIE4         }, // S_BOS2_DIE3
    { SPR_BOS2,   11,                     8, A_Fall,                 S_BOS2_DIE5         }, // S_BOS2_DIE4
    { SPR_BOS2,   12,                     8, NULL,                   S_BOS2_DIE6         }, // S_BOS2_DIE5
    { SPR_BOS2,   13,                     8, NULL,                   S_BOS2_DIE7         }, // S_BOS2_DIE6
    { SPR_BOS2,   14,                    -1, NULL,                   S_NULL              }, // S_BOS2_DIE7
    { SPR_BOS2,   14,                     8, NULL,                   S_BOS2_RAISE2       }, // S_BOS2_RAISE1
    { SPR_BOS2,   13,                     8, NULL,                   S_BOS2_RAISE3       }, // S_BOS2_RAISE2
    { SPR_BOS2,   12,                     8, NULL,                   S_BOS2_RAISE4       }, // S_BOS2_RAISE3
    { SPR_BOS2,   11,                     8, NULL,                   S_BOS2_RAISE5       }, // S_BOS2_RAISE4
    { SPR_BOS2,   10,                     8, NULL,                   S_BOS2_RAISE6       }, // S_BOS2_RAISE5
    { SPR_BOS2,    9,                     8, NULL,                   S_BOS2_RAISE7       }, // S_BOS2_RAISE6
    { SPR_BOS2,    8,                     8, NULL,                   S_BOS2_RUN1         }, // S_BOS2_RAISE7

    // Lost Soul (MT_SKULL)
    { SPR_SKUL,   (0 | FF_FULLBRIGHT),   10, A_Look,                 S_SKULL_STND2       }, // S_SKULL_STND
    { SPR_SKUL,   (1 | FF_FULLBRIGHT),   10, A_Look,                 S_SKULL_STND        }, // S_SKULL_STND2
    { SPR_SKUL,   (0 | FF_FULLBRIGHT),    6, A_Chase,                S_SKULL_RUN2        }, // S_SKULL_RUN1
    { SPR_SKUL,   (1 | FF_FULLBRIGHT),    6, A_Chase,                S_SKULL_RUN1        }, // S_SKULL_RUN2
    { SPR_SKUL,   (2 | FF_FULLBRIGHT),   10, A_FaceTarget,           S_SKULL_ATK2        }, // S_SKULL_ATK1
    { SPR_SKUL,   (3 | FF_FULLBRIGHT),    4, A_SkullAttack,          S_SKULL_ATK3        }, // S_SKULL_ATK2
    { SPR_SKUL,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_SKULL_ATK4        }, // S_SKULL_ATK3
    { SPR_SKUL,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_SKULL_ATK3        }, // S_SKULL_ATK4
    { SPR_SKUL,   (4 | FF_FULLBRIGHT),    3, NULL,                   S_SKULL_PAIN2       }, // S_SKULL_PAIN
    { SPR_SKUL,   (4 | FF_FULLBRIGHT),    3, A_Pain,                 S_SKULL_RUN1        }, // S_SKULL_PAIN2
    { SPR_SKUL,   (5 | FF_FULLBRIGHT),    6, NULL,                   S_SKULL_DIE2        }, // S_SKULL_DIE1
    { SPR_SKUL,   (6 | FF_FULLBRIGHT),    6, A_Scream,               S_SKULL_DIE3        }, // S_SKULL_DIE2
    { SPR_SKUL,   (7 | FF_FULLBRIGHT),    6, NULL,                   S_SKULL_DIE4        }, // S_SKULL_DIE3
    { SPR_SKUL,   (8 | FF_FULLBRIGHT),    6, A_Fall,                 S_SKULL_DIE5        }, // S_SKULL_DIE4
    { SPR_SKUL,    9,                     6, NULL,                   S_SKULL_DIE6        }, // S_SKULL_DIE5

    // Lost Soul Death (MT_MISC65)
    { SPR_SKUL,   10,                     6, NULL,                   S_NULL              }, // S_SKULL_DIE6

    // Spiderdemon (MT_SPIDER)
    { SPR_SPID,    0,                    10, A_Look,                 S_SPID_STND2        }, // S_SPID_STND
    { SPR_SPID,    1,                    10, A_Look,                 S_SPID_STND         }, // S_SPID_STND2
    { SPR_SPID,    0,                     3, A_Metal,                S_SPID_RUN2         }, // S_SPID_RUN1
    { SPR_SPID,    0,                     3, A_Chase,                S_SPID_RUN3         }, // S_SPID_RUN2
    { SPR_SPID,    1,                     3, A_Chase,                S_SPID_RUN4         }, // S_SPID_RUN3
    { SPR_SPID,    1,                     3, A_Chase,                S_SPID_RUN5         }, // S_SPID_RUN4
    { SPR_SPID,    2,                     3, A_Metal,                S_SPID_RUN6         }, // S_SPID_RUN5
    { SPR_SPID,    2,                     3, A_Chase,                S_SPID_RUN7         }, // S_SPID_RUN6
    { SPR_SPID,    3,                     3, A_Chase,                S_SPID_RUN8         }, // S_SPID_RUN7
    { SPR_SPID,    3,                     3, A_Chase,                S_SPID_RUN9         }, // S_SPID_RUN8
    { SPR_SPID,    4,                     3, A_Metal,                S_SPID_RUN10        }, // S_SPID_RUN9
    { SPR_SPID,    4,                     3, A_Chase,                S_SPID_RUN11        }, // S_SPID_RUN10
    { SPR_SPID,    5,                     3, A_Chase,                S_SPID_RUN12        }, // S_SPID_RUN11
    { SPR_SPID,    5,                     3, A_Chase,                S_SPID_RUN1         }, // S_SPID_RUN12
    { SPR_SPID,    0,                    20, A_FaceTarget,           S_SPID_ATK2         }, // S_SPID_ATK1
    { SPR_SPID,   (6 | FF_FULLBRIGHT),    4, A_SPosAttack,           S_SPID_ATK3         }, // S_SPID_ATK2
    { SPR_SPID,    7,                     4, A_SPosAttack,           S_SPID_ATK4         }, // S_SPID_ATK3
    { SPR_SPID,   (7 | FF_FULLBRIGHT),    1, A_SpidRefire,           S_SPID_ATK2         }, // S_SPID_ATK4
    { SPR_SPID,    8,                     3, NULL,                   S_SPID_PAIN2        }, // S_SPID_PAIN
    { SPR_SPID,    8,                     3, A_Pain,                 S_SPID_RUN1         }, // S_SPID_PAIN2
    { SPR_SPID,    9,                    20, A_Scream,               S_SPID_DIE2         }, // S_SPID_DIE1
    { SPR_SPID,   10,                    10, A_Fall,                 S_SPID_DIE3         }, // S_SPID_DIE2
    { SPR_SPID,   11,                    10, NULL,                   S_SPID_DIE4         }, // S_SPID_DIE3
    { SPR_SPID,  (12 | FF_FULLBRIGHT),   10, NULL,                   S_SPID_DIE5         }, // S_SPID_DIE4
    { SPR_SPID,  (13 | FF_FULLBRIGHT),   10, NULL,                   S_SPID_DIE6         }, // S_SPID_DIE5
    { SPR_SPID,  (14 | FF_FULLBRIGHT),   10, NULL,                   S_SPID_DIE7         }, // S_SPID_DIE6
    { SPR_SPID,  (15 | FF_FULLBRIGHT),   10, NULL,                   S_SPID_DIE8         }, // S_SPID_DIE7
    { SPR_SPID,  (16 | FF_FULLBRIGHT),   10, NULL,                   S_SPID_DIE9         }, // S_SPID_DIE8
    { SPR_SPID,  (17 | FF_FULLBRIGHT),   10, NULL,                   S_SPID_DIE10        }, // S_SPID_DIE9
    { SPR_SPID,   18,                    30, NULL,                   S_SPID_DIE11        }, // S_SPID_DIE10
    { SPR_SPID,   18,                    -1, A_BossDeath,            S_NULL              }, // S_SPID_DIE11

    // Arachnotron (MT_BABY)
    { SPR_BSPI,    0,                    10, A_Look,                 S_BSPI_STND2        }, // S_BSPI_STND
    { SPR_BSPI,    1,                    10, A_Look,                 S_BSPI_STND         }, // S_BSPI_STND2
    { SPR_BSPI,    0,                    20, NULL,                   S_BSPI_RUN1         }, // S_BSPI_SIGHT
    { SPR_BSPI,    0,                     3, A_BabyMetal,            S_BSPI_RUN2         }, // S_BSPI_RUN1
    { SPR_BSPI,    0,                     3, A_Chase,                S_BSPI_RUN3         }, // S_BSPI_RUN2
    { SPR_BSPI,    1,                     3, A_Chase,                S_BSPI_RUN4         }, // S_BSPI_RUN3
    { SPR_BSPI,    1,                     3, A_Chase,                S_BSPI_RUN5         }, // S_BSPI_RUN4
    { SPR_BSPI,    2,                     3, A_Chase,                S_BSPI_RUN6         }, // S_BSPI_RUN5
    { SPR_BSPI,    2,                     3, A_Chase,                S_BSPI_RUN7         }, // S_BSPI_RUN6
    { SPR_BSPI,    3,                     3, A_BabyMetal,            S_BSPI_RUN8         }, // S_BSPI_RUN7
    { SPR_BSPI,    3,                     3, A_Chase,                S_BSPI_RUN9         }, // S_BSPI_RUN8
    { SPR_BSPI,    4,                     3, A_Chase,                S_BSPI_RUN10        }, // S_BSPI_RUN9
    { SPR_BSPI,    4,                     3, A_Chase,                S_BSPI_RUN11        }, // S_BSPI_RUN10
    { SPR_BSPI,    5,                     3, A_Chase,                S_BSPI_RUN12        }, // S_BSPI_RUN11
    { SPR_BSPI,    5,                     3, A_Chase,                S_BSPI_RUN1         }, // S_BSPI_RUN12
    { SPR_BSPI,    0,                    20, A_FaceTarget,           S_BSPI_ATK2         }, // S_BSPI_ATK1
    { SPR_BSPI,   (6 | FF_FULLBRIGHT),    4, A_BspiAttack,           S_BSPI_ATK3         }, // S_BSPI_ATK2
    { SPR_BSPI,    7,                     4, NULL,                   S_BSPI_ATK4         }, // S_BSPI_ATK3
    { SPR_BSPI,   (7 | FF_FULLBRIGHT),    1, A_SpidRefire,           S_BSPI_ATK2         }, // S_BSPI_ATK4
    { SPR_BSPI,    8,                     3, NULL,                   S_BSPI_PAIN2        }, // S_BSPI_PAIN
    { SPR_BSPI,    8,                     3, A_Pain,                 S_BSPI_RUN1         }, // S_BSPI_PAIN2
    { SPR_BSPI,    9,                    20, A_Scream,               S_BSPI_DIE2         }, // S_BSPI_DIE1
    { SPR_BSPI,   10,                     7, A_Fall,                 S_BSPI_DIE3         }, // S_BSPI_DIE2
    { SPR_BSPI,   11,                     7, NULL,                   S_BSPI_DIE4         }, // S_BSPI_DIE3
    { SPR_BSPI,   12,                     7, NULL,                   S_BSPI_DIE5         }, // S_BSPI_DIE4
    { SPR_BSPI,   13,                     7, NULL,                   S_BSPI_DIE6         }, // S_BSPI_DIE5
    { SPR_BSPI,   14,                     7, NULL,                   S_BSPI_DIE7         }, // S_BSPI_DIE6
    { SPR_BSPI,   15,                    -1, A_BossDeath,            S_NULL              }, // S_BSPI_DIE7
    { SPR_BSPI,   15,                     5, NULL,                   S_BSPI_RAISE2       }, // S_BSPI_RAISE1
    { SPR_BSPI,   14,                     5, NULL,                   S_BSPI_RAISE3       }, // S_BSPI_RAISE2
    { SPR_BSPI,   13,                     5, NULL,                   S_BSPI_RAISE4       }, // S_BSPI_RAISE3
    { SPR_BSPI,   12,                     5, NULL,                   S_BSPI_RAISE5       }, // S_BSPI_RAISE4
    { SPR_BSPI,   11,                     5, NULL,                   S_BSPI_RAISE6       }, // S_BSPI_RAISE5
    { SPR_BSPI,   10,                     5, NULL,                   S_BSPI_RAISE7       }, // S_BSPI_RAISE6
    { SPR_BSPI,    9,                     5, NULL,                   S_BSPI_RUN1         }, // S_BSPI_RAISE7

    // Arachnotron Projectile (MT_ARACHPLAZ)
    { SPR_APLS,   (0 | FF_FULLBRIGHT),    5, NULL,                   S_ARACH_PLAZ2       }, // S_ARACH_PLAZ
    { SPR_APLS,   (1 | FF_FULLBRIGHT),    5, NULL,                   S_ARACH_PLAZ        }, // S_ARACH_PLAZ2
    { SPR_APBX,   (0 | FF_FULLBRIGHT),    5, NULL,                   S_ARACH_PLEX2       }, // S_ARACH_PLEX
    { SPR_APBX,   (1 | FF_FULLBRIGHT),    5, NULL,                   S_ARACH_PLEX3       }, // S_ARACH_PLEX2
    { SPR_APBX,   (2 | FF_FULLBRIGHT),    5, NULL,                   S_ARACH_PLEX4       }, // S_ARACH_PLEX3
    { SPR_APBX,   (3 | FF_FULLBRIGHT),    5, NULL,                   S_ARACH_PLEX5       }, // S_ARACH_PLEX4
    { SPR_APBX,   (4 | FF_FULLBRIGHT),    5, NULL,                   S_NULL              }, // S_ARACH_PLEX5

    // Cyberdemon (MT_CYBORG)
    { SPR_CYBR,    0,                    10, A_Look,                 S_CYBER_STND2       }, // S_CYBER_STND
    { SPR_CYBR,    1,                    10, A_Look,                 S_CYBER_STND        }, // S_CYBER_STND2
    { SPR_CYBR,    0,                     3, A_Hoof,                 S_CYBER_RUN2        }, // S_CYBER_RUN1
    { SPR_CYBR,    0,                     3, A_Chase,                S_CYBER_RUN3        }, // S_CYBER_RUN2
    { SPR_CYBR,    1,                     3, A_Chase,                S_CYBER_RUN4        }, // S_CYBER_RUN3
    { SPR_CYBR,    1,                     3, A_Chase,                S_CYBER_RUN5        }, // S_CYBER_RUN4
    { SPR_CYBR,    2,                     3, A_Chase,                S_CYBER_RUN6        }, // S_CYBER_RUN5
    { SPR_CYBR,    2,                     3, A_Chase,                S_CYBER_RUN7        }, // S_CYBER_RUN6
    { SPR_CYBR,    3,                     3, A_Metal,                S_CYBER_RUN8        }, // S_CYBER_RUN7
    { SPR_CYBR,    3,                     3, A_Chase,                S_CYBER_RUN1        }, // S_CYBER_RUN8
    { SPR_CYBR,    4,                     6, A_FaceTarget,           S_CYBER_ATK2        }, // S_CYBER_ATK1
    { SPR_CYBR,   (5 | FF_FULLBRIGHT),   12, A_CyberAttack,          S_CYBER_ATK3        }, // S_CYBER_ATK2
    { SPR_CYBR,    4,                    12, A_FaceTarget,           S_CYBER_ATK4        }, // S_CYBER_ATK3
    { SPR_CYBR,   (5 | FF_FULLBRIGHT),   12, A_CyberAttack,          S_CYBER_ATK5        }, // S_CYBER_ATK4
    { SPR_CYBR,    4,                    12, A_FaceTarget,           S_CYBER_ATK6        }, // S_CYBER_ATK5
    { SPR_CYBR,   (5 | FF_FULLBRIGHT),   12, A_CyberAttack,          S_CYBER_RUN1        }, // S_CYBER_ATK6
    { SPR_CYBR,    6,                    10, A_Pain,                 S_CYBER_RUN1        }, // S_CYBER_PAIN
    { SPR_CYBR,    7,                    10, NULL,                   S_CYBER_DIE2        }, // S_CYBER_DIE1
    { SPR_CYBR,    8,                    10, A_Scream,               S_CYBER_DIE3        }, // S_CYBER_DIE2
    { SPR_CYBR,   (9 | FF_FULLBRIGHT),   10, NULL,                   S_CYBER_DIE4        }, // S_CYBER_DIE3
    { SPR_CYBR,  (10 | FF_FULLBRIGHT),   10, NULL,                   S_CYBER_DIE5        }, // S_CYBER_DIE4
    { SPR_CYBR,  (11 | FF_FULLBRIGHT),   10, NULL,                   S_CYBER_DIE6        }, // S_CYBER_DIE5
    { SPR_CYBR,  (12 | FF_FULLBRIGHT),   10, A_Fall,                 S_CYBER_DIE7        }, // S_CYBER_DIE6
    { SPR_CYBR,  (13 | FF_FULLBRIGHT),   10, NULL,                   S_CYBER_DIE8        }, // S_CYBER_DIE7
    { SPR_CYBR,   14,                    10, NULL,                   S_CYBER_DIE9        }, // S_CYBER_DIE8
    { SPR_CYBR,   15,                    30, NULL,                   S_CYBER_DIE10       }, // S_CYBER_DIE9
    { SPR_CYBR,   15,                    -1, A_BossDeath,            S_NULL              }, // S_CYBER_DIE10

    // Pain Elemental (MT_PAIN)
    { SPR_PAIN,    0,                    10, A_Look,                 S_PAIN_STND         }, // S_PAIN_STND
    { SPR_PAIN,    0,                     3, A_Chase,                S_PAIN_RUN2         }, // S_PAIN_RUN1
    { SPR_PAIN,    0,                     3, A_Chase,                S_PAIN_RUN3         }, // S_PAIN_RUN2
    { SPR_PAIN,    1,                     3, A_Chase,                S_PAIN_RUN4         }, // S_PAIN_RUN3
    { SPR_PAIN,    1,                     3, A_Chase,                S_PAIN_RUN5         }, // S_PAIN_RUN4
    { SPR_PAIN,    2,                     3, A_Chase,                S_PAIN_RUN6         }, // S_PAIN_RUN5
    { SPR_PAIN,    2,                     3, A_Chase,                S_PAIN_RUN1         }, // S_PAIN_RUN6
    { SPR_PAIN,    3,                     5, A_FaceTarget,           S_PAIN_ATK2         }, // S_PAIN_ATK1
    { SPR_PAIN,    4,                     5, A_FaceTarget,           S_PAIN_ATK3         }, // S_PAIN_ATK2
    { SPR_PAIN,   (5 | FF_FULLBRIGHT),    5, A_FaceTarget,           S_PAIN_ATK4         }, // S_PAIN_ATK3
    { SPR_PAIN,   (5 | FF_FULLBRIGHT),    0, A_PainAttack,           S_PAIN_RUN1         }, // S_PAIN_ATK4
    { SPR_PAIN,    6,                     6, NULL,                   S_PAIN_PAIN2        }, // S_PAIN_PAIN
    { SPR_PAIN,    6,                     6, A_Pain,                 S_PAIN_RUN1         }, // S_PAIN_PAIN2
    { SPR_PAIN,   (7 | FF_FULLBRIGHT),    8, NULL,                   S_PAIN_DIE2         }, // S_PAIN_DIE1
    { SPR_PAIN,   (8 | FF_FULLBRIGHT),    8, A_Scream,               S_PAIN_DIE3         }, // S_PAIN_DIE2
    { SPR_PAIN,   (9 | FF_FULLBRIGHT),    8, NULL,                   S_PAIN_DIE4         }, // S_PAIN_DIE3
    { SPR_PAIN,  (10 | FF_FULLBRIGHT),    8, NULL,                   S_PAIN_DIE5         }, // S_PAIN_DIE4
    { SPR_PAIN,  (11 | FF_FULLBRIGHT),    8, A_PainDie,              S_PAIN_DIE6         }, // S_PAIN_DIE5
    { SPR_PAIN,  (12 | FF_FULLBRIGHT),    8, NULL,                   S_NULL              }, // S_PAIN_DIE6
    { SPR_PAIN,   12,                     8, NULL,                   S_PAIN_RAISE2       }, // S_PAIN_RAISE1
    { SPR_PAIN,   11,                     8, NULL,                   S_PAIN_RAISE3       }, // S_PAIN_RAISE2
    { SPR_PAIN,   10,                     8, NULL,                   S_PAIN_RAISE4       }, // S_PAIN_RAISE3
    { SPR_PAIN,    9,                     8, NULL,                   S_PAIN_RAISE5       }, // S_PAIN_RAISE4
    { SPR_PAIN,    8,                     8, NULL,                   S_PAIN_RAISE6       }, // S_PAIN_RAISE5
    { SPR_PAIN,    7,                     8, NULL,                   S_PAIN_RUN1         }, // S_PAIN_RAISE6

    // Wolfenstein SS (MT_WOLFSS)
    { SPR_SSWV,    0,                    10, A_Look,                 S_SSWV_STND2        }, // S_SSWV_STND
    { SPR_SSWV,    1,                    10, A_Look,                 S_SSWV_STND         }, // S_SSWV_STND2
    { SPR_SSWV,    0,                     3, A_Chase,                S_SSWV_RUN2         }, // S_SSWV_RUN1
    { SPR_SSWV,    0,                     3, A_Chase,                S_SSWV_RUN3         }, // S_SSWV_RUN2
    { SPR_SSWV,    1,                     3, A_Chase,                S_SSWV_RUN4         }, // S_SSWV_RUN3
    { SPR_SSWV,    1,                     3, A_Chase,                S_SSWV_RUN5         }, // S_SSWV_RUN4
    { SPR_SSWV,    2,                     3, A_Chase,                S_SSWV_RUN6         }, // S_SSWV_RUN5
    { SPR_SSWV,    2,                     3, A_Chase,                S_SSWV_RUN7         }, // S_SSWV_RUN6
    { SPR_SSWV,    3,                     3, A_Chase,                S_SSWV_RUN8         }, // S_SSWV_RUN7
    { SPR_SSWV,    3,                     3, A_Chase,                S_SSWV_RUN1         }, // S_SSWV_RUN8
    { SPR_SSWV,    4,                    10, A_FaceTarget,           S_SSWV_ATK2         }, // S_SSWV_ATK1
    { SPR_SSWV,    5,                    10, A_FaceTarget,           S_SSWV_ATK3         }, // S_SSWV_ATK2
    { SPR_SSWV,   (6 | FF_FULLBRIGHT),    4, A_CPosAttack,           S_SSWV_ATK4         }, // S_SSWV_ATK3
    { SPR_SSWV,    5,                     6, A_FaceTarget,           S_SSWV_ATK5         }, // S_SSWV_ATK4
    { SPR_SSWV,   (6 | FF_FULLBRIGHT),    4, A_CPosAttack,           S_SSWV_ATK6         }, // S_SSWV_ATK5
    { SPR_SSWV,    5,                     1, A_CPosRefire,           S_SSWV_ATK2         }, // S_SSWV_ATK6
    { SPR_SSWV,    7,                     3, NULL,                   S_SSWV_PAIN2        }, // S_SSWV_PAIN
    { SPR_SSWV,    7,                     3, A_Pain,                 S_SSWV_RUN1         }, // S_SSWV_PAIN2
    { SPR_SSWV,    8,                     5, NULL,                   S_SSWV_DIE2         }, // S_SSWV_DIE1
    { SPR_SSWV,    9,                     5, A_Scream,               S_SSWV_DIE3         }, // S_SSWV_DIE2
    { SPR_SSWV,   10,                     5, A_Fall,                 S_SSWV_DIE4         }, // S_SSWV_DIE3
    { SPR_SSWV,   11,                     5, NULL,                   S_SSWV_DIE5         }, // S_SSWV_DIE4
    { SPR_SSWV,   12,                    -1, NULL,                   S_NULL              }, // S_SSWV_DIE5
    { SPR_SSWV,   13,                     5, NULL,                   S_SSWV_XDIE2        }, // S_SSWV_XDIE1
    { SPR_SSWV,   14,                     5, A_XScream,              S_SSWV_XDIE3        }, // S_SSWV_XDIE2
    { SPR_SSWV,   15,                     5, A_Fall,                 S_SSWV_XDIE4        }, // S_SSWV_XDIE3
    { SPR_SSWV,   16,                     5, NULL,                   S_SSWV_XDIE5        }, // S_SSWV_XDIE4
    { SPR_SSWV,   17,                     5, NULL,                   S_SSWV_XDIE6        }, // S_SSWV_XDIE5
    { SPR_SSWV,   18,                     5, NULL,                   S_SSWV_XDIE7        }, // S_SSWV_XDIE6
    { SPR_SSWV,   19,                     5, NULL,                   S_SSWV_XDIE8        }, // S_SSWV_XDIE7
    { SPR_SSWV,   20,                     5, NULL,                   S_SSWV_XDIE9        }, // S_SSWV_XDIE8
    { SPR_SSWV,   21,                    -1, NULL,                   S_NULL              }, // S_SSWV_XDIE9
    { SPR_SSWV,   12,                     5, NULL,                   S_SSWV_RAISE2       }, // S_SSWV_RAISE1
    { SPR_SSWV,   11,                     5, NULL,                   S_SSWV_RAISE3       }, // S_SSWV_RAISE2
    { SPR_SSWV,   10,                     5, NULL,                   S_SSWV_RAISE4       }, // S_SSWV_RAISE3
    { SPR_SSWV,    9,                     5, NULL,                   S_SSWV_RAISE5       }, // S_SSWV_RAISE4
    { SPR_SSWV,    8,                     5, NULL,                   S_SSWV_RUN1         }, // S_SSWV_RAISE5

    // Commander Keen (MT_KEEN)
    { SPR_KEEN,    0,                    -1, NULL,                   S_KEENSTND          }, // S_KEENSTND
    { SPR_KEEN,    0,                     6, NULL,                   S_COMMKEEN2         }, // S_COMMKEEN
    { SPR_KEEN,    1,                     6, NULL,                   S_COMMKEEN3         }, // S_COMMKEEN2
    { SPR_KEEN,    2,                     6, A_Scream,               S_COMMKEEN4         }, // S_COMMKEEN3
    { SPR_KEEN,    3,                     6, NULL,                   S_COMMKEEN5         }, // S_COMMKEEN4
    { SPR_KEEN,    4,                     6, NULL,                   S_COMMKEEN6         }, // S_COMMKEEN5
    { SPR_KEEN,    5,                     6, NULL,                   S_COMMKEEN7         }, // S_COMMKEEN6
    { SPR_KEEN,    6,                     6, NULL,                   S_COMMKEEN8         }, // S_COMMKEEN7
    { SPR_KEEN,    7,                     6, NULL,                   S_COMMKEEN9         }, // S_COMMKEEN8
    { SPR_KEEN,    8,                     6, NULL,                   S_COMMKEEN10        }, // S_COMMKEEN9
    { SPR_KEEN,    9,                     6, NULL,                   S_COMMKEEN11        }, // S_COMMKEEN10
    { SPR_KEEN,   10,                     6, A_KeenDie,              S_COMMKEEN12        }, // S_COMMKEEN11
    { SPR_KEEN,   11,                    -1, NULL,                   S_NULL              }, // S_COMMKEEN12
    { SPR_KEEN,   12,                     4, NULL,                   S_KEENPAIN2         }, // S_KEENPAIN
    { SPR_KEEN,   12,                     8, A_Pain,                 S_KEENSTND          }, // S_KEENPAIN2

    // Boss Brain (MT_BOSSBRAIN)
    { SPR_BBRN,    0,                    -1, NULL,                   S_NULL              }, // S_BRAIN
    { SPR_BBRN,    1,                    36, A_BrainPain,            S_BRAIN             }, // S_BRAIN_PAIN
    { SPR_BBRN,    0,                   100, A_BrainScream,          S_BRAIN_DIE2        }, // S_BRAIN_DIE1
    { SPR_BBRN,    0,                    10, NULL,                   S_BRAIN_DIE3        }, // S_BRAIN_DIE2
    { SPR_BBRN,    0,                    10, NULL,                   S_BRAIN_DIE4        }, // S_BRAIN_DIE3
    { SPR_BBRN,    0,                    -1, A_BrainDie,             S_NULL              }, // S_BRAIN_DIE4

    // MT_BOSSSPIT
    { SPR_SSWV,   0,                    10, A_Look,                 S_BRAINEYE          }, // S_BRAINEYE
    { SPR_SSWV,   0,                   181, A_BrainAwake,           S_BRAINEYE1         }, // S_BRAINEYESEE
    { SPR_SSWV,   0,                   150, A_BrainSpit,            S_BRAINEYE1         }, // S_BRAINEYE1

    // Boss Brain Projectile (MT_SPAWNSHOT)
    { SPR_BOSF,   (0 | FF_FULLBRIGHT),    3, A_SpawnSound,           S_SPAWN2            }, // S_SPAWN1
    { SPR_BOSF,   (1 | FF_FULLBRIGHT),    3, A_SpawnFly,             S_SPAWN3            }, // S_SPAWN2
    { SPR_BOSF,   (2 | FF_FULLBRIGHT),    3, A_SpawnFly,             S_SPAWN4            }, // S_SPAWN3
    { SPR_BOSF,   (3 | FF_FULLBRIGHT),    3, A_SpawnFly,             S_SPAWN1            }, // S_SPAWN4

    // Boss Brain Fire (MT_SPAWNFIRE)
    { SPR_FIRE,   (0 | FF_FULLBRIGHT),    4, A_Fire,                 S_SPAWNFIRE2        }, // S_SPAWNFIRE1
    { SPR_FIRE,   (1 | FF_FULLBRIGHT),    4, A_Fire,                 S_SPAWNFIRE3        }, // S_SPAWNFIRE2
    { SPR_FIRE,   (2 | FF_FULLBRIGHT),    4, A_Fire,                 S_SPAWNFIRE4        }, // S_SPAWNFIRE3
    { SPR_FIRE,   (3 | FF_FULLBRIGHT),    4, A_Fire,                 S_SPAWNFIRE5        }, // S_SPAWNFIRE4
    { SPR_FIRE,   (4 | FF_FULLBRIGHT),    4, A_Fire,                 S_SPAWNFIRE6        }, // S_SPAWNFIRE5
    { SPR_FIRE,   (5 | FF_FULLBRIGHT),    4, A_Fire,                 S_SPAWNFIRE7        }, // S_SPAWNFIRE6
    { SPR_FIRE,   (6 | FF_FULLBRIGHT),    4, A_Fire,                 S_SPAWNFIRE8        }, // S_SPAWNFIRE7
    { SPR_FIRE,   (7 | FF_FULLBRIGHT),    4, A_Fire,                 S_NULL              }, // S_SPAWNFIRE8
    { SPR_MISL,   (1 | FF_FULLBRIGHT),   10, NULL,                   S_BRAINEXPLODE2     }, // S_BRAINEXPLODE1
    { SPR_MISL,   (2 | FF_FULLBRIGHT),   10, NULL,                   S_BRAINEXPLODE3     }, // S_BRAINEXPLODE2
    { SPR_MISL,   (3 | FF_FULLBRIGHT),   10, A_BrainExplode,         S_NULL              }, // S_BRAINEXPLODE3

    // Green Armor (MT_MISC0)
    { SPR_ARM1,    0,                     6, NULL,                   S_ARM1A             }, // S_ARM1
    { SPR_ARM1,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_ARM1              }, // S_ARM1A

    // Blue Armor (MT_MISC1)
    { SPR_ARM2,    0,                     6, NULL,                   S_ARM2A             }, // S_ARM2
    { SPR_ARM2,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_ARM2              }, // S_ARM2A

    // Barrel (MT_BARREL)
    { SPR_BAR1,    0,                     6, NULL,                   S_BAR3              }, // S_BAR2
    { SPR_BAR1,    1,                     6, NULL,                   S_BAR1              }, // S_BAR3
    { SPR_BEXP,    0,                     5, NULL,                   S_BEXP2             }, // S_BEXP
    { SPR_BEXP,    1,                     5, A_Scream,               S_BEXP3             }, // S_BEXP2
    { SPR_BEXP,   (2 | FF_FULLBRIGHT),    5, NULL,                   S_BEXP4             }, // S_BEXP3
    { SPR_BEXP,   (3 | FF_FULLBRIGHT),   10, A_Explode,              S_BEXP5             }, // S_BEXP4
    { SPR_BEXP,   (4 | FF_FULLBRIGHT),   10, NULL,                   S_NULL              }, // S_BEXP5

    // Burning Barrel (MT_MISC77)
    { SPR_FCAN,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_BBAR2             }, // S_BBAR1
    { SPR_FCAN,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_BBAR3             }, // S_BBAR2
    { SPR_FCAN,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_BBAR1             }, // S_BBAR3

    // Health Bonus (MT_MISC2)
    { SPR_BON1,    0,                     6, NULL,                   S_BON1A             }, // S_BON1
    { SPR_BON1,    1,                     6, NULL,                   S_BON1B             }, // S_BON1A
    { SPR_BON1,    2,                     6, NULL,                   S_BON1C             }, // S_BON1B
    { SPR_BON1,    3,                     6, NULL,                   S_BON1D             }, // S_BON1C
    { SPR_BON1,    2,                     6, NULL,                   S_BON1E             }, // S_BON1D
    { SPR_BON1,    1,                     6, NULL,                   S_BON1              }, // S_BON1E

    // Armor Bonus (MT_MISC3)
    { SPR_BON2,    0,                     6, NULL,                   S_BON2A             }, // S_BON2
    { SPR_BON2,    1,                     6, NULL,                   S_BON2B             }, // S_BON2A
    { SPR_BON2,    2,                     6, NULL,                   S_BON2C             }, // S_BON2B
    { SPR_BON2,    3,                     6, NULL,                   S_BON2D             }, // S_BON2C
    { SPR_BON2,    2,                     6, NULL,                   S_BON2E             }, // S_BON2D
    { SPR_BON2,    1,                     6, NULL,                   S_BON2              }, // S_BON2E

    // Blue Keycard (MT_MISC4)
    { SPR_BKEY,    0,                    10, NULL,                   S_BKEY2             }, // S_BKEY
    { SPR_BKEY,   (1 | FF_FULLBRIGHT),   10, NULL,                   S_BKEY              }, // S_BKEY2

    // Red Keycard (MT_MISC5)
    { SPR_RKEY,    0,                    10, NULL,                   S_RKEY2             }, // S_RKEY
    { SPR_RKEY,   (1 | FF_FULLBRIGHT),   10, NULL,                   S_RKEY              }, // S_RKEY2

    // Yellow Keycard (MT_MISC6)
    { SPR_YKEY,    0,                    10, NULL,                   S_YKEY2             }, // S_YKEY
    { SPR_YKEY,   (1 | FF_FULLBRIGHT),   10, NULL,                   S_YKEY              }, // S_YKEY2

    // Blue Skull Key (MT_MISC9)
    { SPR_BSKU,    0,                    10, NULL,                   S_BSKULL2           }, // S_BSKULL
    { SPR_BSKU,   (1 | FF_FULLBRIGHT),   10, NULL,                   S_BSKULL            }, // S_BSKULL2

    // Red Skull Key (MT_MISC8)
    { SPR_RSKU,    0,                    10, NULL,                   S_RSKULL2           }, // S_RSKULL
    { SPR_RSKU,   (1 | FF_FULLBRIGHT),   10, NULL,                   S_RSKULL            }, // S_RSKULL2

    // Yellow Skull Key (MT_MISC7)
    { SPR_YSKU,    0,                    10, NULL,                   S_YSKULL2           }, // S_YSKULL
    { SPR_YSKU,   (1 | FF_FULLBRIGHT),   10, NULL,                   S_YSKULL            }, // S_YSKULL2

    // Stimpack (MT_MISC10)
    { SPR_STIM,    0,                    -1, NULL,                   S_NULL              }, // S_STIM

    // Medikit (MT_MISC11)
    { SPR_MEDI,    0,                    -1, NULL,                   S_NULL              }, // S_MEDI

    // SoulSphere (MT_MISC12)
    { SPR_SOUL,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_SOUL2             }, // S_SOUL
    { SPR_SOUL,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_SOUL3             }, // S_SOUL2
    { SPR_SOUL,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_SOUL4             }, // S_SOUL3
    { SPR_SOUL,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_SOUL5             }, // S_SOUL4
    { SPR_SOUL,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_SOUL6             }, // S_SOUL5
    { SPR_SOUL,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_SOUL              }, // S_SOUL6

    // Invulnerability (MT_INV)
    { SPR_PINV,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_PINV2             }, // S_PINV
    { SPR_PINV,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_PINV3             }, // S_PINV2
    { SPR_PINV,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_PINV4             }, // S_PINV3
    { SPR_PINV,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_PINV              }, // S_PINV4

    // Berserk (MT_MISC13)
    { SPR_PSTR,   (0 | FF_FULLBRIGHT),   -1, NULL,                   S_NULL              }, // S_PSTR

    // Partial Invisibility (MT_INS)
    { SPR_PINS,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_PINS2             }, // S_PINS
    { SPR_PINS,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_PINS3             }, // S_PINS2
    { SPR_PINS,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_PINS4             }, // S_PINS3
    { SPR_PINS,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_PINS              }, // S_PINS4

    // MegaSphere (MT_MEGA)
    { SPR_MEGA,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_MEGA2             }, // S_MEGA
    { SPR_MEGA,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_MEGA3             }, // S_MEGA2
    { SPR_MEGA,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_MEGA4             }, // S_MEGA3
    { SPR_MEGA,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_MEGA              }, // S_MEGA4

    // Radiation Shielding Suit (MT_MISC14)
    { SPR_SUIT,   (0 | FF_FULLBRIGHT),   -1, NULL,                   S_NULL              }, // S_SUIT

    // Computer Area Map (MT_MISC15)
    { SPR_PMAP,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_PMAP2             }, // S_PMAP
    { SPR_PMAP,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_PMAP3             }, // S_PMAP2
    { SPR_PMAP,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_PMAP4             }, // S_PMAP3
    { SPR_PMAP,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_PMAP5             }, // S_PMAP4
    { SPR_PMAP,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_PMAP6             }, // S_PMAP5
    { SPR_PMAP,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_PMAP              }, // S_PMAP6

    // Light Amplification Visor (MT_MISC16)
    { SPR_PVIS,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_PVIS2             }, // S_PVIS
    { SPR_PVIS,    1,                     6, NULL,                   S_PVIS              }, // S_PVIS2

    // Clip (MT_CLIP)
    { SPR_CLIP,    0,                    -1, NULL,                   S_NULL              }, // S_CLIP

    // Box of Bullets (MT_MISC17)
    { SPR_AMMO,    0,                    -1, NULL,                   S_NULL              }, // S_AMMO

    // Rocket (MT_MISC18)
    { SPR_ROCK,    0,                    -1, NULL,                   S_NULL              }, // S_ROCK

    // Box of Rockets (MT_MISC19)
    { SPR_BROK,    0,                    -1, NULL,                   S_NULL              }, // S_BROK

    // Cell (MT_MISC20)
    { SPR_CELL,    0,                    -1, NULL,                   S_NULL              }, // S_CELL

    // Cell Pack (MT_MISC21)
    { SPR_CELP,    0,                    -1, NULL,                   S_NULL              }, // S_CELP

    // Shotgun Shells (MT_MISC22)
    { SPR_SHEL,    0,                    -1, NULL,                   S_NULL              }, // S_SHEL

    // Box of Shotgun Shells (MT_MISC23)
    { SPR_SBOX,    0,                    -1, NULL,                   S_NULL              }, // S_SBOX

    // Backpack (MT_MISC24)
    { SPR_BPAK,    0,                    -1, NULL,                   S_NULL              }, // S_BPAK

    // BFG-9000 (MT_MISC25)
    { SPR_BFUG,    0,                    -1, NULL,                   S_NULL              }, // S_BFUG

    // Chaingun (MT_CHAINGUN)
    { SPR_MGUN,    0,                    -1, NULL,                   S_NULL              }, // S_MGUN

    // Chainsaw (MT_MISC26)
    { SPR_CSAW,    0,                    -1, NULL,                   S_NULL              }, // S_CSAW

    // Rocket Launcher (MT_MISC27)
    { SPR_LAUN,    0,                    -1, NULL,                   S_NULL              }, // S_LAUN

    // Plasma Rifle (MT_MISC28)
    { SPR_PLAS,    0,                    -1, NULL,                   S_NULL              }, // S_PLAS

    // Shotgun (MT_SHOTGUN)
    { SPR_SHOT,    0,                    -1, NULL,                   S_NULL              }, // S_SHOT

    // Super Shotgun (MT_SUPERSHOTGUN)
    { SPR_SGN2,    0,                    -1, NULL,                   S_NULL              }, // S_SHOT2

    // Floor Lamp (MT_MISC31)
    { SPR_COLU,   (0 | FF_FULLBRIGHT),   -1, NULL,                   S_NULL              }, // S_COLU

    // Gray Stalagmite (unused)
    { SPR_SMT2,    0,                    -1, NULL,                   S_NULL              }, // S_STALAG

    // Hanging victim, twitching (MT_MISC51 and MT_MISC60)
    { SPR_GOR1,    0,                    10, NULL,                   S_BLOODYTWITCH2     }, // S_BLOODYTWITCH
    { SPR_GOR1,    1,                    15, NULL,                   S_BLOODYTWITCH3     }, // S_BLOODYTWITCH2
    { SPR_GOR1,    2,                     8, NULL,                   S_BLOODYTWITCH4     }, // S_BLOODYTWITCH3
    { SPR_GOR1,    1,                     6, NULL,                   S_BLOODYTWITCH      }, // S_BLOODYTWITCH4

    { SPR_PLAY,   13,                    -1, NULL,                   S_NULL              }, // S_DEADTORSO
    { SPR_PLAY,   18,                    -1, NULL,                   S_NULL              }, // S_DEADBOTTOM

    // Five skulls shishkebab (MT_MISC70)
    { SPR_POL2,    0,                    -1, NULL,                   S_NULL              }, // S_HEADSONSTICK

    // Pool of blood and flesh (MT_MISC71)
    { SPR_POL5,    0,                    -1, NULL,                   S_NULL              }, // S_GIBS

    // Skull on a pole (MT_MISC72)
    { SPR_POL4,    0,                    -1, NULL,                   S_NULL              }, // S_HEADONASTICK

    // Pile of skulls and candles (MT_MISC73)
    { SPR_POL3,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_HEADCANDLES2      }, // S_HEADCANDLES
    { SPR_POL3,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_HEADCANDLES       }, // S_HEADCANDLES2

    // Impaled human (MT_MISC74)
    { SPR_POL1,    0,                    -1, NULL,                   S_NULL              }, // S_DEADSTICK

    // Twitching impaled human (MT_MISC75)
    { SPR_POL6,    0,                     6, NULL,                   S_LIVESTICK2        }, // S_LIVESTICK
    { SPR_POL6,    1,                     8, NULL,                   S_LIVESTICK         }, // S_LIVESTICK2

    // Hanging victim, arms out (MT_MISC52 and MT_MISC56)
    { SPR_GOR2,    0,                    -1, NULL,                   S_NULL              }, // S_MEAT2

    // Hanging victim, one-legged (MT_MISC53 and MT_MISC58)
    { SPR_GOR3,    0,                    -1, NULL,                   S_NULL              }, // S_MEAT3

    // Hanging pair of legs (MT_MISC54 and MT_MISC57)
    { SPR_GOR4,    0,                    -1, NULL,                   S_NULL              }, // S_MEAT4

    // Hanging leg (MT_MISC55 and MT_MISC59)
    { SPR_GOR5,    0,                    -1, NULL,                   S_NULL              }, // S_MEAT5

    // Stalagmite (MT_MISC47)
    { SPR_SMIT,    0,                    -1, NULL,                   S_NULL              }, // S_STALAGTITE

    // Tall green pillar (MT_MISC32)
    { SPR_COL1,    0,                    -1, NULL,                   S_NULL              }, // S_TALLGRNCOL

    // Short green pillar (MT_MISC33)
    { SPR_COL2,    0,                    -1, NULL,                   S_NULL              }, // S_SHRTGRNCOL

    // Tall red pillar (MT_MISC34)
    { SPR_COL3,    0,                    -1, NULL,                   S_NULL              }, // S_TALLREDCOL

    // Short red pillar (MT_MISC35)
    { SPR_COL4,    0,                    -1, NULL,                   S_NULL              }, // S_SHRTREDCOL

    // Candle (MT_MISC49)
    { SPR_CAND,   (0 | FF_FULLBRIGHT),   -1, NULL,                   S_NULL              }, // S_CANDLESTIK

    // Candelabra (MT_MISC50)
    { SPR_CBRA,   (0 | FF_FULLBRIGHT),   -1, NULL,                   S_NULL              }, // S_CANDELABRA

    // Short red pillar with skull (MT_MISC36)
    { SPR_COL6,    0,                    -1, NULL,                   S_NULL              }, // S_SKULLCOL

    // Burnt tree (MT_MISC40)
    { SPR_TRE1,    0,                    -1, NULL,                   S_NULL              }, // S_TORCHTREE

    // Large brown tree (MT_MISC76)
    { SPR_TRE2,    0,                    -1, NULL,                   S_NULL              }, // S_BIGTREE

    // Tall techno pillar (MT_MISC48)
    { SPR_ELEC,    0,                    -1, NULL,                   S_NULL              }, // S_TECHPILLAR

    // Evil eye (MT_MISC38)
    { SPR_CEYE,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_EVILEYE2          }, // S_EVILEYE
    { SPR_CEYE,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_EVILEYE3          }, // S_EVILEYE2
    { SPR_CEYE,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_EVILEYE4          }, // S_EVILEYE3
    { SPR_CEYE,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_EVILEYE           }, // S_EVILEYE4

    // Floating skull (MT_MISC39)
    { SPR_FSKU,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_FLOATSKULL2       }, // S_FLOATSKULL
    { SPR_FSKU,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_FLOATSKULL3       }, // S_FLOATSKULL2
    { SPR_FSKU,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_FLOATSKULL        }, // S_FLOATSKULL3

    // Short green pillar with beating heart (MT_MISC37)
    { SPR_COL5,    0,                    14, NULL,                   S_HEARTCOL2         }, // S_HEARTCOL
    { SPR_COL5,    1,                    14, NULL,                   S_HEARTCOL          }, // S_HEARTCOL2

    // Tall blue firestick (MT_MISC41)
    { SPR_TBLU,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_BLUETORCH2        }, // S_BLUETORCH
    { SPR_TBLU,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_BLUETORCH3        }, // S_BLUETORCH2
    { SPR_TBLU,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_BLUETORCH4        }, // S_BLUETORCH3
    { SPR_TBLU,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_BLUETORCH         }, // S_BLUETORCH4

    // Tall green firestick (MT_MISC42)
    { SPR_TGRN,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_GREENTORCH2       }, // S_GREENTORCH
    { SPR_TGRN,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_GREENTORCH3       }, // S_GREENTORCH2
    { SPR_TGRN,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_GREENTORCH4       }, // S_GREENTORCH3
    { SPR_TGRN,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_GREENTORCH        }, // S_GREENTORCH4

    // Tall red firestick (MT_MISC43)
    { SPR_TRED,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_REDTORCH2         }, // S_REDTORCH
    { SPR_TRED,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_REDTORCH3         }, // S_REDTORCH2
    { SPR_TRED,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_REDTORCH4         }, // S_REDTORCH3
    { SPR_TRED,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_REDTORCH          }, // S_REDTORCH4

    // Short blue firestick (MT_MISC44)
    { SPR_SMBT,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_BTORCHSHRT2       }, // S_BTORCHSHRT
    { SPR_SMBT,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_BTORCHSHRT3       }, // S_BTORCHSHRT2
    { SPR_SMBT,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_BTORCHSHRT4       }, // S_BTORCHSHRT3
    { SPR_SMBT,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_BTORCHSHRT        }, // S_BTORCHSHRT4

    // Short green firestick (MT_MISC45)
    { SPR_SMGT,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_GTORCHSHRT2       }, // S_GTORCHSHRT
    { SPR_SMGT,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_GTORCHSHRT3       }, // S_GTORCHSHRT2
    { SPR_SMGT,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_GTORCHSHRT4       }, // S_GTORCHSHRT3
    { SPR_SMGT,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_GTORCHSHRT        }, // S_GTORCHSHRT4

    // Short red firestick (MT_MISC46)
    { SPR_SMRT,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_RTORCHSHRT2       }, // S_RTORCHSHRT
    { SPR_SMRT,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_RTORCHSHRT3       }, // S_RTORCHSHRT2
    { SPR_SMRT,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_RTORCHSHRT4       }, // S_RTORCHSHRT3
    { SPR_SMRT,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_RTORCHSHRT        }, // S_RTORCHSHRT4

    // Hanging victim, guts removed (MT_MISC78)
    { SPR_HDB1,    0,                    -1, NULL,                   S_NULL              }, // S_HANGNOGUTS

    // Hanging victim, guts and brain removed (MT_MISC79)
    { SPR_HDB2,    0,                    -1, NULL,                   S_NULL              }, // S_HANGBNOBRAIN

    // Hanging torso, looking down (MT_MISC80)
    { SPR_HDB3,    0,                    -1, NULL,                   S_NULL              }, // S_HANGTLOOKDN

    // Hanging torso, open skull (MT_MISC81)
    { SPR_HDB4,    0,                    -1, NULL,                   S_NULL              }, // S_HANGTSKULL

    // Hanging torso, looking up (MT_MISC82)
    { SPR_HDB5,    0,                    -1, NULL,                   S_NULL              }, // S_HANGTLOOKUP

    // Hanging torso, brain removed (MT_MISC83)
    { SPR_HDB6,    0,                    -1, NULL,                   S_NULL              }, // S_HANGTNOBRAIN

    // Pool of blood and guts (MT_MISC84)
    { SPR_POB1,    0,                    -1, NULL,                   S_NULL              }, // S_COLONGIBS

    // Pool of blood (MT_MISC85)
    { SPR_POB2,    0,                    -1, NULL,                   S_NULL              }, // S_SMALLPOOL

    // Pool of brains (MT_MISC86)
    { SPR_BRS1,    0,                    -1, NULL,                   S_NULL              }, // S_BRAINSTEM

    // Tall techno floor lamp (MT_MISC29)
    { SPR_TLMP,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_TECHLAMP2         }, // S_TECHLAMP
    { SPR_TLMP,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_TECHLAMP3         }, // S_TECHLAMP2
    { SPR_TLMP,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_TECHLAMP4         }, // S_TECHLAMP3
    { SPR_TLMP,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_TECHLAMP          }, // S_TECHLAMP4

    // Short techno floor lamp (MT_MISC30)
    { SPR_TLP2,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_TECH2LAMP2        }, // S_TECH2LAMP
    { SPR_TLP2,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_TECH2LAMP3        }, // S_TECH2LAMP2
    { SPR_TLP2,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_TECH2LAMP4        }, // S_TECH2LAMP3
    { SPR_TLP2,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_TECH2LAMP         }, // S_TECH2LAMP4

    { SPR_TNT1,    0,                    -1, NULL,                   S_TNT1              }, // S_TNT1

    // killough 8/9/98: grenade
    { SPR_MISL,   (0 | FF_FULLBRIGHT),  1000, A_Die,                  S_GRENADE           }, // S_GRENADE

    { SPR_MISL,   (1 | FF_FULLBRIGHT),    4, A_Scream,               S_DETONATE2         }, // S_DETONATE
    { SPR_MISL,   (2 | FF_FULLBRIGHT),    6, A_Detonate,             S_DETONATE3         }, // S_DETONATE2
    { SPR_MISL,   (3 | FF_FULLBRIGHT),   10, NULL,                   S_NULL              }, // S_DETONATE3

    // killough 7/19/98: Marine's best friend :)
    { SPR_DOGS,    0,                    10, A_Look,                 S_DOGS_STND2        }, // S_DOGS_STND
    { SPR_DOGS,    1,                    10, A_Look,                 S_DOGS_STND         }, // S_DOGS_STND2
    { SPR_DOGS,    0,                     2, A_Chase,                S_DOGS_RUN2         }, // S_DOGS_RUN1
    { SPR_DOGS,    0,                     2, A_Chase,                S_DOGS_RUN3         }, // S_DOGS_RUN2
    { SPR_DOGS,    1,                     2, A_Chase,                S_DOGS_RUN4         }, // S_DOGS_RUN3
    { SPR_DOGS,    1,                     2, A_Chase,                S_DOGS_RUN5         }, // S_DOGS_RUN4
    { SPR_DOGS,    2,                     2, A_Chase,                S_DOGS_RUN6         }, // S_DOGS_RUN5
    { SPR_DOGS,    2,                     2, A_Chase,                S_DOGS_RUN7         }, // S_DOGS_RUN6
    { SPR_DOGS,    3,                     2, A_Chase,                S_DOGS_RUN8         }, // S_DOGS_RUN7
    { SPR_DOGS,    3,                     2, A_Chase,                S_DOGS_RUN1         }, // S_DOGS_RUN8
    { SPR_DOGS,    4,                     8, A_FaceTarget,           S_DOGS_ATK2         }, // S_DOGS_ATK1
    { SPR_DOGS,    5,                     8, A_FaceTarget,           S_DOGS_ATK3         }, // S_DOGS_ATK2
    { SPR_DOGS,    6,                     8, A_SargAttack,           S_DOGS_RUN1         }, // S_DOGS_ATK3
    { SPR_DOGS,    7,                     2, NULL,                   S_DOGS_PAIN2        }, // S_DOGS_PAIN
    { SPR_DOGS,    7,                     2, A_Pain,                 S_DOGS_RUN1         }, // S_DOGS_PAIN2
    { SPR_DOGS,    8,                     8, NULL,                   S_DOGS_DIE2         }, // S_DOGS_DIE1
    { SPR_DOGS,    9,                     8, A_Scream,               S_DOGS_DIE3         }, // S_DOGS_DIE2
    { SPR_DOGS,   10,                     4, NULL,                   S_DOGS_DIE4         }, // S_DOGS_DIE3
    { SPR_DOGS,   11,                     4, A_Fall,                 S_DOGS_DIE5         }, // S_DOGS_DIE4
    { SPR_DOGS,   12,                     4, NULL,                   S_DOGS_DIE6         }, // S_DOGS_DIE5
    { SPR_DOGS,   13,                    -1, NULL,                   S_NULL              }, // S_DOGS_DIE6
    { SPR_DOGS,   13,                     5, NULL,                   S_DOGS_RAISE2       }, // S_DOGS_RAISE1
    { SPR_DOGS,   12,                     5, NULL,                   S_DOGS_RAISE3       }, // S_DOGS_RAISE2
    { SPR_DOGS,   11,                     5, NULL,                   S_DOGS_RAISE4       }, // S_DOGS_RAISE3
    { SPR_DOGS,   10,                     5, NULL,                   S_DOGS_RAISE5       }, // S_DOGS_RAISE4
    { SPR_DOGS,    9,                     5, NULL,                   S_DOGS_RAISE6       }, // S_DOGS_RAISE5
    { SPR_DOGS,    8,                     5, NULL,                   S_DOGS_RUN1         }, // S_DOGS_RAISE6

    // add dummy beta bfg/lost soul frames for dehacked compatibility
    { SPR_BFGG,    0,                    10, A_BFGSound,             S_OLDBFG2           }, // S_OLDBFG1
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG3           }, // S_OLDBFG2
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG4           }, // S_OLDBFG3
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG5           }, // S_OLDBFG4
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG6           }, // S_OLDBFG5
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG7           }, // S_OLDBFG6
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG8           }, // S_OLDBFG7
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG9           }, // S_OLDBFG8
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG10          }, // S_OLDBFG9
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG11          }, // S_OLDBFG10
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG12          }, // S_OLDBFG11
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG13          }, // S_OLDBFG12
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG14          }, // S_OLDBFG13
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG15          }, // S_OLDBFG14
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG16          }, // S_OLDBFG15
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG17          }, // S_OLDBFG16
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG18          }, // S_OLDBFG17
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG19          }, // S_OLDBFG18
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG20          }, // S_OLDBFG19
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG21          }, // S_OLDBFG20
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG22          }, // S_OLDBFG21
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG23          }, // S_OLDBFG22
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG24          }, // S_OLDBFG23
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG25          }, // S_OLDBFG24
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG26          }, // S_OLDBFG25
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG27          }, // S_OLDBFG26
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG28          }, // S_OLDBFG27
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG29          }, // S_OLDBFG28
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG30          }, // S_OLDBFG29
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG31          }, // S_OLDBFG30
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG32          }, // S_OLDBFG31
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG33          }, // S_OLDBFG32
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG34          }, // S_OLDBFG33
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG35          }, // S_OLDBFG34
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG36          }, // S_OLDBFG35
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG37          }, // S_OLDBFG36
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG38          }, // S_OLDBFG37
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG39          }, // S_OLDBFG38
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG40          }, // S_OLDBFG39
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG41          }, // S_OLDBFG40
    { SPR_BFGG,    1,                     1, A_FireOldBFG,           S_OLDBFG42          }, // S_OLDBFG41
    { SPR_BFGG,    1,                     0, A_Light0,               S_OLDBFG43          }, // S_OLDBFG42
    { SPR_BFGG,    1,                    20, A_ReFire,               S_BFG               }, // S_OLDBFG43

    // killough 7/19/98: First plasma fireball in the beta:
    { SPR_PLS1,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_PLS1BALL2         }, // S_PLS1BALL
    { SPR_PLS1,   (1 | FF_FULLBRIGHT),    6, NULL,                   S_PLS1BALL          }, // S_PLS1BALL2
    { SPR_PLS1,   (2 | FF_FULLBRIGHT),    4, NULL,                   S_PLS1EXP2          }, // S_PLS1EXP
    { SPR_PLS1,   (3 | FF_FULLBRIGHT),    4, NULL,                   S_PLS1EXP3          }, // S_PLS1EXP2
    { SPR_PLS1,   (4 | FF_FULLBRIGHT),    4, NULL,                   S_PLS1EXP4          }, // S_PLS1EXP3
    { SPR_PLS1,   (5 | FF_FULLBRIGHT),    4, NULL,                   S_PLS1EXP5          }, // S_PLS1EXP4
    { SPR_PLS1,   (6 | FF_FULLBRIGHT),    4, NULL,                   S_NULL              }, // S_PLS1EXP5

    // killough 7/19/98: Second plasma fireball in the beta:
    { SPR_PLS2,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_PLS2BALL2         }, // S_PLS2BALL
    { SPR_PLS2,   (1 | FF_FULLBRIGHT),    4, NULL,                   S_PLS2BALL          }, // S_PLS2BALL2
    { SPR_PLS2,   (2 | FF_FULLBRIGHT),    6, NULL,                   S_PLS2BALLX2        }, // S_PLS2BALLX1
    { SPR_PLS2,   (3 | FF_FULLBRIGHT),    6, NULL,                   S_PLS2BALLX3        }, // S_PLS2BALLX2
    { SPR_PLS2,   (4 | FF_FULLBRIGHT),    6, NULL,                   S_NULL              }, // S_PLS2BALLX3

    // killough 7/11/98: beta bonus items
    { SPR_BON3,    0,                     6, NULL,                   S_BON3              }, // S_BON3
    { SPR_BON4,    0,                     6, NULL,                   S_BON4              }, // S_BON4

    // killough 10/98: beta lost souls attacked from a distance,
    // animated with colors, and stayed in the air when killed.
    { SPR_SKUL,    0,                    10, A_Look,                 S_BSKUL_STND        }, // S_BSKUL_STND
    { SPR_SKUL,    1,                     5, A_Chase,                S_BSKUL_RUN2        }, // S_BSKUL_RUN1
    { SPR_SKUL,    2,                     5, A_Chase,                S_BSKUL_RUN3        }, // S_BSKUL_RUN2
    { SPR_SKUL,    3,                     5, A_Chase,                S_BSKUL_RUN4        }, // S_BSKUL_RUN3
    { SPR_SKUL,    0,                     5, A_Chase,                S_BSKUL_RUN1        }, // S_BSKUL_RUN4
    { SPR_SKUL,    4,                     4, A_FaceTarget,           S_BSKUL_ATK2        }, // S_BSKUL_ATK1
    { SPR_SKUL,    5,                     5, A_BetaSkullAttack,      S_BSKUL_ATK3        }, // S_BSKUL_ATK2
    { SPR_SKUL,    5,                     4, NULL,                   S_BSKUL_RUN1        }, // S_BSKUL_ATK3
    { SPR_SKUL,    6,                     4, NULL,                   S_BSKUL_PAIN2       }, // S_BSKUL_PAIN1
    { SPR_SKUL,    7,                     2, A_Pain,                 S_BSKUL_RUN1        }, // S_BSKUL_PAIN2
    { SPR_SKUL,    8,                     4, NULL,                   S_BSKUL_RUN1        }, // S_BSKUL_PAIN3
    { SPR_SKUL,    9,                     5, NULL,                   S_BSKUL_DIE2        }, // S_BSKUL_DIE1
    { SPR_SKUL,   10,                     5, NULL,                   S_BSKUL_DIE3        }, // S_BSKUL_DIE2
    { SPR_SKUL,   11,                     5, NULL,                   S_BSKUL_DIE4        }, // S_BSKUL_DIE3
    { SPR_SKUL,   12,                     5, NULL,                   S_BSKUL_DIE5        }, // S_BSKUL_DIE4
    { SPR_SKUL,   13,                     5, A_Scream,               S_BSKUL_DIE6        }, // S_BSKUL_DIE5
    { SPR_SKUL,   14,                     5, NULL,                   S_BSKUL_DIE7        }, // S_BSKUL_DIE6
    { SPR_SKUL,   15,                     5, A_Fall,                 S_BSKUL_DIE8        }, // S_BSKUL_DIE7
    { SPR_SKUL,   16,                     5, A_Stop,                 S_BSKUL_DIE8        }, // S_BSKUL_DIE8

    // killough 10/98: mushroom effect
    { SPR_MISL,   (1 | FF_FULLBRIGHT),    8, A_Mushroom,             S_EXPLODE2          }, // S_MUSHROOM

    // Barrel (MT_BARREL)
    { SPR_BEXP,    0,                     6, NULL,                   S_BAR2              }, // S_BAR1

    // Smoke Trail (MT_TRAIL)
    { SPR_PUFF,   (0 | FF_FULLBRIGHT),    4, NULL,                   S_TRAIL2            }, // S_TRAIL
    { SPR_PUFF,    1,                     4, NULL,                   S_TRAIL3            }, // S_TRAIL2
    { SPR_PUFF,    2,                    10, NULL,                   S_TRAIL4            }, // S_TRAIL3
    { SPR_PUFF,    3,                    14, NULL,                   S_NULL              }  // S_TRAIL4
};

state_t hereticstates[NUMHSTATES] =
{
  //  sprite       frame               tics  action                  nextstate                 state
    { HSPR_IMPX,   0,                    -1, NULL,                   HS_NULL             }, // HS_NULL
    { HSPR_ACLO,   4,                  1050, A_FreeTargMobj,         HS_NULL             }, // HS_FREETARGMOBJ
    { HSPR_PTN1,   0,                     3, NULL,                   HS_ITEM_PTN1_2      }, // HS_ITEM_PTN1_1
    { HSPR_PTN1,   1,                     3, NULL,                   HS_ITEM_PTN1_3      }, // HS_ITEM_PTN1_2
    { HSPR_PTN1,   2,                     3, NULL,                   HS_ITEM_PTN1_1      }, // HS_ITEM_PTN1_3
    { HSPR_SHLD,   0,                    -1, NULL,                   HS_NULL             }, // HS_ITEM_SHLD1
    { HSPR_SHD2,   0,                    -1, NULL,                   HS_NULL             }, // HS_ITEM_SHD2_1
    { HSPR_BAGH,   0,                    -1, NULL,                   HS_NULL             }, // HS_ITEM_BAGH1
    { HSPR_SPMP,   0,                    -1, NULL,                   HS_NULL             }, // HS_ITEM_SPMP1
    { HSPR_ACLO,   4,                  1400, NULL,                   HS_HIDESPECIAL2     }, // HS_HIDESPECIAL1
    { HSPR_ACLO,   0,                     4, A_RestoreSpecialThing1, HS_HIDESPECIAL3     }, // HS_HIDESPECIAL2
    { HSPR_ACLO,   1,                     4, NULL,                   HS_HIDESPECIAL4     }, // HS_HIDESPECIAL3
    { HSPR_ACLO,   0,                     4, NULL,                   HS_HIDESPECIAL5     }, // HS_HIDESPECIAL4
    { HSPR_ACLO,   1,                     4, NULL,                   HS_HIDESPECIAL6     }, // HS_HIDESPECIAL5
    { HSPR_ACLO,   2,                     4, NULL,                   HS_HIDESPECIAL7     }, // HS_HIDESPECIAL6
    { HSPR_ACLO,   1,                     4, NULL,                   HS_HIDESPECIAL8     }, // HS_HIDESPECIAL7
    { HSPR_ACLO,   2,                     4, NULL,                   HS_HIDESPECIAL9     }, // HS_HIDESPECIAL8
    { HSPR_ACLO,   3,                     4, NULL,                   HS_HIDESPECIAL10    }, // HS_HIDESPECIAL9
    { HSPR_ACLO,   2,                     4, NULL,                   HS_HIDESPECIAL11    }, // HS_HIDESPECIAL10
    { HSPR_ACLO,   3,                     4, A_RestoreSpecialThing2, HS_NULL             }, // HS_HIDESPECIAL11
    { HSPR_ACLO,   3,                     3, NULL,                   HS_DORMANTARTI2     }, // HS_DORMANTARTI1
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DORMANTARTI3     }, // HS_DORMANTARTI2
    { HSPR_ACLO,   3,                     3, NULL,                   HS_DORMANTARTI4     }, // HS_DORMANTARTI3
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DORMANTARTI5     }, // HS_DORMANTARTI4
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DORMANTARTI6     }, // HS_DORMANTARTI5
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DORMANTARTI7     }, // HS_DORMANTARTI6
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DORMANTARTI8     }, // HS_DORMANTARTI7
    { HSPR_ACLO,   0,                     3, NULL,                   HS_DORMANTARTI9     }, // HS_DORMANTARTI8
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DORMANTARTI10    }, // HS_DORMANTARTI9
    { HSPR_ACLO,   0,                     3, NULL,                   HS_DORMANTARTI11    }, // HS_DORMANTARTI10
    { HSPR_ACLO,   0,                  1400, A_HideThing,            HS_DORMANTARTI12    }, // HS_DORMANTARTI11
    { HSPR_ACLO,   0,                     3, A_UnHideThing,          HS_DORMANTARTI13    }, // HS_DORMANTARTI12
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DORMANTARTI14    }, // HS_DORMANTARTI13
    { HSPR_ACLO,   0,                     3, NULL,                   HS_DORMANTARTI15    }, // HS_DORMANTARTI14
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DORMANTARTI16    }, // HS_DORMANTARTI15
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DORMANTARTI17    }, // HS_DORMANTARTI16
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DORMANTARTI18    }, // HS_DORMANTARTI17
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DORMANTARTI19    }, // HS_DORMANTARTI18
    { HSPR_ACLO,   3,                     3, NULL,                   HS_DORMANTARTI20    }, // HS_DORMANTARTI19
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DORMANTARTI21    }, // HS_DORMANTARTI20
    { HSPR_ACLO,   3,                     3, A_RestoreArtifact,      HS_NULL             }, // HS_DORMANTARTI21
    { HSPR_ACLO,   3,                     3, NULL,                   HS_DEADARTI2        }, // HS_DEADARTI1
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DEADARTI3        }, // HS_DEADARTI2
    { HSPR_ACLO,   3,                     3, NULL,                   HS_DEADARTI4        }, // HS_DEADARTI3
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DEADARTI5        }, // HS_DEADARTI4
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DEADARTI6        }, // HS_DEADARTI5
    { HSPR_ACLO,   2,                     3, NULL,                   HS_DEADARTI7        }, // HS_DEADARTI6
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DEADARTI8        }, // HS_DEADARTI7
    { HSPR_ACLO,   0,                     3, NULL,                   HS_DEADARTI9        }, // HS_DEADARTI8
    { HSPR_ACLO,   1,                     3, NULL,                   HS_DEADARTI10       }, // HS_DEADARTI9
    { HSPR_ACLO,   0,                     3, NULL,                   HS_NULL             }, // HS_DEADARTI10
    { HSPR_INVS,  (0 | FF_FULLBRIGHT),  350, NULL,                   HS_ARTI_INVS1       }, // HS_ARTI_INVS1
    { HSPR_PTN2,   0,                     4, NULL,                   HS_ARTI_PTN2_2      }, // HS_ARTI_PTN2_1
    { HSPR_PTN2,   1,                     4, NULL,                   HS_ARTI_PTN2_3      }, // HS_ARTI_PTN2_2
    { HSPR_PTN2,   2,                     4, NULL,                   HS_ARTI_PTN2_1      }, // HS_ARTI_PTN2_3
    { HSPR_SOAR,   0,                     5, NULL,                   HS_ARTI_SOAR2       }, // HS_ARTI_SOAR1
    { HSPR_SOAR,   1,                     5, NULL,                   HS_ARTI_SOAR3       }, // HS_ARTI_SOAR2
    { HSPR_SOAR,   2,                     5, NULL,                   HS_ARTI_SOAR4       }, // HS_ARTI_SOAR3
    { HSPR_SOAR,   1,                     5, NULL,                   HS_ARTI_SOAR1       }, // HS_ARTI_SOAR4
    { HSPR_INVU,   0,                     3, NULL,                   HS_ARTI_INVU2       }, // HS_ARTI_INVU1
    { HSPR_INVU,   1,                     3, NULL,                   HS_ARTI_INVU3       }, // HS_ARTI_INVU2
    { HSPR_INVU,   2,                     3, NULL,                   HS_ARTI_INVU4       }, // HS_ARTI_INVU3
    { HSPR_INVU,   3,                     3, NULL,                   HS_ARTI_INVU1       }, // HS_ARTI_INVU4
    { HSPR_PWBK,   0,                   350, NULL,                   HS_ARTI_PWBK1       }, // HS_ARTI_PWBK1
    { HSPR_EGGC,   0,                     6, NULL,                   HS_ARTI_EGGC2       }, // HS_ARTI_EGGC1
    { HSPR_EGGC,   1,                     6, NULL,                   HS_ARTI_EGGC3       }, // HS_ARTI_EGGC2
    { HSPR_EGGC,   2,                     6, NULL,                   HS_ARTI_EGGC4       }, // HS_ARTI_EGGC3
    { HSPR_EGGC,   1,                     6, NULL,                   HS_ARTI_EGGC1       }, // HS_ARTI_EGGC4
    { HSPR_EGGM,   0,                     4, NULL,                   HS_EGGFX2           }, // HS_EGGFX1
    { HSPR_EGGM,   1,                     4, NULL,                   HS_EGGFX3           }, // HS_EGGFX2
    { HSPR_EGGM,   2,                     4, NULL,                   HS_EGGFX4           }, // HS_EGGFX3
    { HSPR_EGGM,   3,                     4, NULL,                   HS_EGGFX5           }, // HS_EGGFX4
    { HSPR_EGGM,   4,                     4, NULL,                   HS_EGGFX1           }, // HS_EGGFX5
    { HSPR_FX01,  (4 | FF_FULLBRIGHT),    3, NULL,                   HS_EGGFXI1_2        }, // HS_EGGFXI1_1
    { HSPR_FX01,  (5 | FF_FULLBRIGHT),    3, NULL,                   HS_EGGFXI1_3        }, // HS_EGGFXI1_2
    { HSPR_FX01,  (6 | FF_FULLBRIGHT),    3, NULL,                   HS_EGGFXI1_4        }, // HS_EGGFXI1_3
    { HSPR_FX01,  (7 | FF_FULLBRIGHT),    3, NULL,                   HS_NULL             }, // HS_EGGFXI1_4
    { HSPR_SPHL,   0,                   350, NULL,                   HS_ARTI_SPHL1       }, // HS_ARTI_SPHL1
    { HSPR_TRCH,  (0 | FF_FULLBRIGHT),    3, NULL,                   HS_ARTI_TRCH2       }, // HS_ARTI_TRCH1
    { HSPR_TRCH,  (1 | FF_FULLBRIGHT),    3, NULL,                   HS_ARTI_TRCH3       }, // HS_ARTI_TRCH2
    { HSPR_TRCH,  (2 | FF_FULLBRIGHT),    3, NULL,                   HS_ARTI_TRCH1       }, // HS_ARTI_TRCH3
    { HSPR_FBMB,   4,                   350, NULL,                   HS_ARTI_FBMB1       }, // HS_ARTI_FBMB1
    { HSPR_FBMB,   0,                    10, NULL,                   HS_FIREBOMB2        }, // HS_FIREBOMB1
    { HSPR_FBMB,   1,                    10, NULL,                   HS_FIREBOMB3        }, // HS_FIREBOMB2
    { HSPR_FBMB,   2,                    10, NULL,                   HS_FIREBOMB4        }, // HS_FIREBOMB3
    { HSPR_FBMB,   3,                    10, NULL,                   HS_FIREBOMB5        }, // HS_FIREBOMB4
    { HSPR_FBMB,   4,                     6, A_Scream,               HS_FIREBOMB6        }, // HS_FIREBOMB5
    { HSPR_XPL1,  (0 | FF_FULLBRIGHT),    4, A_Explode,              HS_FIREBOMB7        }, // HS_FIREBOMB6
    { HSPR_XPL1,  (1 | FF_FULLBRIGHT),    4, NULL,                   HS_FIREBOMB8        }, // HS_FIREBOMB7
    { HSPR_XPL1,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_FIREBOMB9        }, // HS_FIREBOMB8
    { HSPR_XPL1,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_FIREBOMB10       }, // HS_FIREBOMB9
    { HSPR_XPL1,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_FIREBOMB11       }, // HS_FIREBOMB10
    { HSPR_XPL1,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_FIREBOMB11
    { HSPR_ATLP,   0,                     4, NULL,                   HS_ARTI_ATLP2       }, // HS_ARTI_ATLP1
    { HSPR_ATLP,   1,                     4, NULL,                   HS_ARTI_ATLP3       }, // HS_ARTI_ATLP2
    { HSPR_ATLP,   2,                     4, NULL,                   HS_ARTI_ATLP4       }, // HS_ARTI_ATLP3
    { HSPR_ATLP,   1,                     4, NULL,                   HS_ARTI_ATLP1       }, // HS_ARTI_ATLP4
    { HSPR_PPOD,   0,                    10, NULL,                   HS_POD_WAIT1        }, // HS_POD_WAIT1
    { HSPR_PPOD,   1,                    14, A_PodPain,              HS_POD_WAIT1        }, // HS_POD_PAIN1
    { HSPR_PPOD,  (2 | FF_FULLBRIGHT),    5, A_RemovePod,            HS_POD_DIE2         }, // HS_POD_DIE1
    { HSPR_PPOD,  (3 | FF_FULLBRIGHT),    5, A_Scream,               HS_POD_DIE3         }, // HS_POD_DIE2
    { HSPR_PPOD,  (4 | FF_FULLBRIGHT),    5, A_Explode,              HS_POD_DIE4         }, // HS_POD_DIE3
    { HSPR_PPOD,  (5 | FF_FULLBRIGHT),   10, NULL,                   HS_FREETARGMOBJ     }, // HS_POD_DIE4
    { HSPR_PPOD,   8,                     3, NULL,                   HS_POD_GROW2        }, // HS_POD_GROW1
    { HSPR_PPOD,   9,                     3, NULL,                   HS_POD_GROW3        }, // HS_POD_GROW2
    { HSPR_PPOD,  10,                     3, NULL,                   HS_POD_GROW4        }, // HS_POD_GROW3
    { HSPR_PPOD,  11,                     3, NULL,                   HS_POD_GROW5        }, // HS_POD_GROW4
    { HSPR_PPOD,  12,                     3, NULL,                   HS_POD_GROW6        }, // HS_POD_GROW5
    { HSPR_PPOD,  13,                     3, NULL,                   HS_POD_GROW7        }, // HS_POD_GROW6
    { HSPR_PPOD,  14,                     3, NULL,                   HS_POD_GROW8        }, // HS_POD_GROW7
    { HSPR_PPOD,  15,                     3, NULL,                   HS_POD_WAIT1        }, // HS_POD_GROW8
    { HSPR_PPOD,   6,                     8, NULL,                   HS_PODGOO2          }, // HS_PODGOO1
    { HSPR_PPOD,   7,                     8, NULL,                   HS_PODGOO1          }, // HS_PODGOO2
    { HSPR_PPOD,   6,                    10, NULL,                   HS_NULL             }, // HS_PODGOOX
    { HSPR_AMG1,   0,                    35, A_MakePod,              HS_PODGENERATOR     }, // HS_PODGENERATOR
    { HSPR_SPSH,   0,                     8, NULL,                   HS_SPLASH2          }, // HS_SPLASH1
    { HSPR_SPSH,   1,                     8, NULL,                   HS_SPLASH3          }, // HS_SPLASH2
    { HSPR_SPSH,   2,                     8, NULL,                   HS_SPLASH4          }, // HS_SPLASH3
    { HSPR_SPSH,   3,                    16, NULL,                   HS_NULL             }, // HS_SPLASH4
    { HSPR_SPSH,   3,                    10, NULL,                   HS_NULL             }, // HS_SPLASHX
    { HSPR_SPSH,   4,                     5, NULL,                   HS_SPLASHBASE2      }, // HS_SPLASHBASE1
    { HSPR_SPSH,   5,                     5, NULL,                   HS_SPLASHBASE3      }, // HS_SPLASHBASE2
    { HSPR_SPSH,   6,                     5, NULL,                   HS_SPLASHBASE4      }, // HS_SPLASHBASE3
    { HSPR_SPSH,   7,                     5, NULL,                   HS_SPLASHBASE5      }, // HS_SPLASHBASE4
    { HSPR_SPSH,   8,                     5, NULL,                   HS_SPLASHBASE6      }, // HS_SPLASHBASE5
    { HSPR_SPSH,   9,                     5, NULL,                   HS_SPLASHBASE7      }, // HS_SPLASHBASE6
    { HSPR_SPSH,  10,                     5, NULL,                   HS_NULL             }, // HS_SPLASHBASE7
    { HSPR_LVAS,  (0 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASPLASH2      }, // HS_LAVASPLASH1
    { HSPR_LVAS,  (1 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASPLASH3      }, // HS_LAVASPLASH2
    { HSPR_LVAS,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASPLASH4      }, // HS_LAVASPLASH3
    { HSPR_LVAS,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASPLASH5      }, // HS_LAVASPLASH4
    { HSPR_LVAS,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASPLASH6      }, // HS_LAVASPLASH5
    { HSPR_LVAS,  (5 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_LAVASPLASH6
    { HSPR_LVAS,  (6 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASMOKE2       }, // HS_LAVASMOKE1
    { HSPR_LVAS,  (7 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASMOKE3       }, // HS_LAVASMOKE2
    { HSPR_LVAS,  (8 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASMOKE4       }, // HS_LAVASMOKE3
    { HSPR_LVAS,  (9 | FF_FULLBRIGHT),    5, NULL,                   HS_LAVASMOKE5       }, // HS_LAVASMOKE4
    { HSPR_LVAS, (10 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_LAVASMOKE5
    { HSPR_SLDG,   0,                     8, NULL,                   HS_SLUDGECHUNK2     }, // HS_SLUDGECHUNK1
    { HSPR_SLDG,   1,                     8, NULL,                   HS_SLUDGECHUNK3     }, // HS_SLUDGECHUNK2
    { HSPR_SLDG,   2,                     8, NULL,                   HS_SLUDGECHUNK4     }, // HS_SLUDGECHUNK3
    { HSPR_SLDG,   3,                     8, NULL,                   HS_NULL             }, // HS_SLUDGECHUNK4
    { HSPR_SLDG,   3,                     6, NULL,                   HS_NULL             }, // HS_SLUDGECHUNKX
    { HSPR_SLDG,   4,                     5, NULL,                   HS_SLUDGESPLASH2    }, // HS_SLUDGESPLASH1
    { HSPR_SLDG,   5,                     5, NULL,                   HS_SLUDGESPLASH3    }, // HS_SLUDGESPLASH2
    { HSPR_SLDG,   6,                     5, NULL,                   HS_SLUDGESPLASH4    }, // HS_SLUDGESPLASH3
    { HSPR_SLDG,   7,                     5, NULL,                   HS_NULL             }, // HS_SLUDGESPLASH4
    { HSPR_SKH1,   0,                    -1, NULL,                   HS_NULL             }, // HS_SKULLHANG70_1
    { HSPR_SKH2,   0,                    -1, NULL,                   HS_NULL             }, // HS_SKULLHANG60_1
    { HSPR_SKH3,   0,                    -1, NULL,                   HS_NULL             }, // HS_SKULLHANG45_1
    { HSPR_SKH4,   0,                    -1, NULL,                   HS_NULL             }, // HS_SKULLHANG35_1
    { HSPR_CHDL,   0,                     4, NULL,                   HS_CHANDELIER2      }, // HS_CHANDELIER1
    { HSPR_CHDL,   1,                     4, NULL,                   HS_CHANDELIER3      }, // HS_CHANDELIER2
    { HSPR_CHDL,   2,                     4, NULL,                   HS_CHANDELIER1      }, // HS_CHANDELIER3
    { HSPR_SRTC,   0,                     4, NULL,                   HS_SERPTORCH2       }, // HS_SERPTORCH1
    { HSPR_SRTC,   1,                     4, NULL,                   HS_SERPTORCH3       }, // HS_SERPTORCH2
    { HSPR_SRTC,   2,                     4, NULL,                   HS_SERPTORCH1       }, // HS_SERPTORCH3
    { HSPR_SMPL,   0,                    -1, NULL,                   HS_NULL             }, // HS_SMALLPILLAR
    { HSPR_STGS,   0,                    -1, NULL,                   HS_NULL             }, // HS_STALAGMITESMALL
    { HSPR_STGL,   0,                    -1, NULL,                   HS_NULL             }, // HS_STALAGMITELARGE
    { HSPR_STCS,   0,                    -1, NULL,                   HS_NULL             }, // HS_STALACTITESMALL
    { HSPR_STCL,   0,                    -1, NULL,                   HS_NULL             }, // HS_STALACTITELARGE
    { HSPR_KFR1,  (0 | FF_FULLBRIGHT),    3, NULL,                   HS_FIREBRAZIER2     }, // HS_FIREBRAZIER1
    { HSPR_KFR1,  (1 | FF_FULLBRIGHT),    3, NULL,                   HS_FIREBRAZIER3     }, // HS_FIREBRAZIER2
    { HSPR_KFR1,  (2 | FF_FULLBRIGHT),    3, NULL,                   HS_FIREBRAZIER4     }, // HS_FIREBRAZIER3
    { HSPR_KFR1,  (3 | FF_FULLBRIGHT),    3, NULL,                   HS_FIREBRAZIER5     }, // HS_FIREBRAZIER4
    { HSPR_KFR1,  (4 | FF_FULLBRIGHT),    3, NULL,                   HS_FIREBRAZIER6     }, // HS_FIREBRAZIER5
    { HSPR_KFR1,  (5 | FF_FULLBRIGHT),    3, NULL,                   HS_FIREBRAZIER7     }, // HS_FIREBRAZIER6
    { HSPR_KFR1,  (6 | FF_FULLBRIGHT),    3, NULL,                   HS_FIREBRAZIER8     }, // HS_FIREBRAZIER7
    { HSPR_KFR1,  (7 | FF_FULLBRIGHT),    3, NULL,                   HS_FIREBRAZIER1     }, // HS_FIREBRAZIER8
    { HSPR_BARL,   0,                    -1, NULL,                   HS_NULL             }, // HS_BARREL
    { HSPR_BRPL,   0,                    -1, NULL,                   HS_NULL             }, // HS_BRPILLAR
    { HSPR_MOS1,   0,                    -1, NULL,                   HS_NULL             }, // HS_MOSS1
    { HSPR_MOS2,   0,                    -1, NULL,                   HS_NULL             }, // HS_MOSS2
    { HSPR_WTRH,  (0 | FF_FULLBRIGHT),    6, NULL,                   HS_WALLTORCH2       }, // HS_WALLTORCH1
    { HSPR_WTRH,  (1 | FF_FULLBRIGHT),    6, NULL,                   HS_WALLTORCH3       }, // HS_WALLTORCH2
    { HSPR_WTRH,  (2 | FF_FULLBRIGHT),    6, NULL,                   HS_WALLTORCH1       }, // HS_WALLTORCH3
    { HSPR_HCOR,   0,                    -1, NULL,                   HS_NULL             }, // HS_HANGINGCORPSE
    { HSPR_KGZ1,   0,                     1, NULL,                   HS_KEYGIZMO2        }, // HS_KEYGIZMO1
    { HSPR_KGZ1,   0,                     1, A_InitKeyGizmo,         HS_KEYGIZMO3        }, // HS_KEYGIZMO2
    { HSPR_KGZ1,   0,                    -1, NULL,                   HS_NULL             }, // HS_KEYGIZMO3
    { HSPR_KGZB,   0,                     1, NULL,                   HS_KGZ_START        }, // HS_KGZ_START
    { HSPR_KGZB,  (0 | FF_FULLBRIGHT),   -1, NULL,                   HS_NULL             }, // HS_KGZ_BLUEFLOAT1
    { HSPR_KGZG,  (0 | FF_FULLBRIGHT),   -1, NULL,                   HS_NULL             }, // HS_KGZ_GREENFLOAT1
    { HSPR_KGZY,  (0 | FF_FULLBRIGHT),   -1, NULL,                   HS_NULL             }, // HS_KGZ_YELLOWFLOAT1
    { HSPR_VLCO,   0,                   350, NULL,                   HS_VOLCANO2         }, // HS_VOLCANO1
    { HSPR_VLCO,   0,                    35, A_VolcanoSet,           HS_VOLCANO3         }, // HS_VOLCANO2
    { HSPR_VLCO,   1,                     3, NULL,                   HS_VOLCANO4         }, // HS_VOLCANO3
    { HSPR_VLCO,   2,                     3, NULL,                   HS_VOLCANO5         }, // HS_VOLCANO4
    { HSPR_VLCO,   3,                     3, NULL,                   HS_VOLCANO6         }, // HS_VOLCANO5
    { HSPR_VLCO,   1,                     3, NULL,                   HS_VOLCANO7         }, // HS_VOLCANO6
    { HSPR_VLCO,   2,                     3, NULL,                   HS_VOLCANO8         }, // HS_VOLCANO7
    { HSPR_VLCO,   3,                     3, NULL,                   HS_VOLCANO9         }, // HS_VOLCANO8
    { HSPR_VLCO,   4,                    10, A_VolcanoBlast,         HS_VOLCANO2         }, // HS_VOLCANO9
    { HSPR_VFBL,   0,                     4, A_BeastPuff,            HS_VOLCANOBALL2     }, // HS_VOLCANOBALL1
    { HSPR_VFBL,   1,                     4, A_BeastPuff,            HS_VOLCANOBALL1     }, // HS_VOLCANOBALL2
    { HSPR_XPL1,   0,                     4, A_VolcBallImpact,       HS_VOLCANOBALLX2    }, // HS_VOLCANOBALLX1
    { HSPR_XPL1,   1,                     4, NULL,                   HS_VOLCANOBALLX3    }, // HS_VOLCANOBALLX2
    { HSPR_XPL1,   2,                     4, NULL,                   HS_VOLCANOBALLX4    }, // HS_VOLCANOBALLX3
    { HSPR_XPL1,   3,                     4, NULL,                   HS_VOLCANOBALLX5    }, // HS_VOLCANOBALLX4
    { HSPR_XPL1,   4,                     4, NULL,                   HS_VOLCANOBALLX6    }, // HS_VOLCANOBALLX5
    { HSPR_XPL1,   5,                     4, NULL,                   HS_NULL             }, // HS_VOLCANOBALLX6
    { HSPR_VTFB,   0,                     4, NULL,                   HS_VOLCANOTBALL2    }, // HS_VOLCANOTBALL1
    { HSPR_VTFB,   1,                     4, NULL,                   HS_VOLCANOTBALL1    }, // HS_VOLCANOTBALL2
    { HSPR_SFFI,   2,                     4, NULL,                   HS_VOLCANOTBALLX2   }, // HS_VOLCANOTBALLX1
    { HSPR_SFFI,   1,                     4, NULL,                   HS_VOLCANOTBALLX3   }, // HS_VOLCANOTBALLX2
    { HSPR_SFFI,   0,                     4, NULL,                   HS_VOLCANOTBALLX4   }, // HS_VOLCANOTBALLX3
    { HSPR_SFFI,   1,                     4, NULL,                   HS_VOLCANOTBALLX5   }, // HS_VOLCANOTBALLX4
    { HSPR_SFFI,   2,                     4, NULL,                   HS_VOLCANOTBALLX6   }, // HS_VOLCANOTBALLX5
    { HSPR_SFFI,   3,                     4, NULL,                   HS_VOLCANOTBALLX7   }, // HS_VOLCANOTBALLX6
    { HSPR_SFFI,   4,                     4, NULL,                   HS_NULL             }, // HS_VOLCANOTBALLX7
    { HSPR_TGLT,   0,                     8, A_SpawnTeleGlitter,     HS_TELEGLITGEN1     }, // HS_TELEGLITGEN1
    { HSPR_TGLT,   5,                     8, A_SpawnTeleGlitter2,    HS_TELEGLITGEN2     }, // HS_TELEGLITGEN2
    { HSPR_TGLT,  (0 | FF_FULLBRIGHT),    2, NULL,                   HS_TELEGLITTER1_2   }, // HS_TELEGLITTER1_1
    { HSPR_TGLT,  (1 | FF_FULLBRIGHT),    2, A_AccTeleGlitter,       HS_TELEGLITTER1_3   }, // HS_TELEGLITTER1_2
    { HSPR_TGLT,  (2 | FF_FULLBRIGHT),    2, NULL,                   HS_TELEGLITTER1_4   }, // HS_TELEGLITTER1_3
    { HSPR_TGLT,  (3 | FF_FULLBRIGHT),    2, A_AccTeleGlitter,       HS_TELEGLITTER1_5   }, // HS_TELEGLITTER1_4
    { HSPR_TGLT,  (4 | FF_FULLBRIGHT),    2, NULL,                   HS_TELEGLITTER1_1   }, // HS_TELEGLITTER1_5
    { HSPR_TGLT,  (5 | FF_FULLBRIGHT),    2, NULL,                   HS_TELEGLITTER2_2   }, // HS_TELEGLITTER2_1
    { HSPR_TGLT,  (6 | FF_FULLBRIGHT),    2, A_AccTeleGlitter,       HS_TELEGLITTER2_3   }, // HS_TELEGLITTER2_2
    { HSPR_TGLT,  (7 | FF_FULLBRIGHT),    2, NULL,                   HS_TELEGLITTER2_4   }, // HS_TELEGLITTER2_3
    { HSPR_TGLT,  (8 | FF_FULLBRIGHT),    2, A_AccTeleGlitter,       HS_TELEGLITTER2_5   }, // HS_TELEGLITTER2_4
    { HSPR_TGLT,  (9 | FF_FULLBRIGHT),    2, NULL,                   HS_TELEGLITTER2_1   }, // HS_TELEGLITTER2_5
    { HSPR_TELE,  (0 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG2            }, // HS_TFOG1
    { HSPR_TELE,  (1 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG3            }, // HS_TFOG2
    { HSPR_TELE,  (2 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG4            }, // HS_TFOG3
    { HSPR_TELE,  (3 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG5            }, // HS_TFOG4
    { HSPR_TELE,  (4 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG6            }, // HS_TFOG5
    { HSPR_TELE,  (5 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG7            }, // HS_TFOG6
    { HSPR_TELE,  (6 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG8            }, // HS_TFOG7
    { HSPR_TELE,  (7 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG9            }, // HS_TFOG8
    { HSPR_TELE,  (6 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG10           }, // HS_TFOG9
    { HSPR_TELE,  (5 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG11           }, // HS_TFOG10
    { HSPR_TELE,  (4 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG12           }, // HS_TFOG11
    { HSPR_TELE,  (3 | FF_FULLBRIGHT),    6, NULL,                   HS_TFOG13           }, // HS_TFOG12
    { HSPR_TELE,  (2 | FF_FULLBRIGHT),    6, NULL,                   HS_NULL             }, // HS_TFOG13
    { HSPR_STFF,   0,                     0, A_Light0,               HS_NULL             }, // HS_LIGHTDONE
    { HSPR_STFF,   0,                     1, A_WeaponReady,          HS_STAFFREADY       }, // HS_STAFFREADY
    { HSPR_STFF,   0,                     1, A_Lower,                HS_STAFFDOWN        }, // HS_STAFFDOWN
    { HSPR_STFF,   0,                     1, A_Raise,                HS_STAFFUP          }, // HS_STAFFUP
    { HSPR_STFF,   3,                     4, A_WeaponReady,          HS_STAFFREADY2_2    }, // HS_STAFFREADY2_1
    { HSPR_STFF,   4,                     4, A_WeaponReady,          HS_STAFFREADY2_3    }, // HS_STAFFREADY2_2
    { HSPR_STFF,   5,                     4, A_WeaponReady,          HS_STAFFREADY2_1    }, // HS_STAFFREADY2_3
    { HSPR_STFF,   3,                     1, A_Lower,                HS_STAFFDOWN2       }, // HS_STAFFDOWN2
    { HSPR_STFF,   3,                     1, A_Raise,                HS_STAFFUP2         }, // HS_STAFFUP2
    { HSPR_STFF,   1,                     6, NULL,                   HS_STAFFATK1_2      }, // HS_STAFFATK1_1
    { HSPR_STFF,   2,                     8, A_StaffAttackPL1,       HS_STAFFATK1_3      }, // HS_STAFFATK1_2
    { HSPR_STFF,   1,                     8, A_ReFire,               HS_STAFFREADY       }, // HS_STAFFATK1_3
    { HSPR_STFF,   6,                     6, NULL,                   HS_STAFFATK2_2      }, // HS_STAFFATK2_1
    { HSPR_STFF,   7,                     8, A_StaffAttackPL2,       HS_STAFFATK2_3      }, // HS_STAFFATK2_2
    { HSPR_STFF,   6,                     8, A_ReFire,               HS_STAFFREADY2_1    }, // HS_STAFFATK2_3
    { HSPR_PUF3,  (0 | FF_FULLBRIGHT),    4, NULL,                   HS_STAFFPUFF2       }, // HS_STAFFPUFF1
    { HSPR_PUF3,   1,                     4, NULL,                   HS_STAFFPUFF3       }, // HS_STAFFPUFF2
    { HSPR_PUF3,   2,                     4, NULL,                   HS_STAFFPUFF4       }, // HS_STAFFPUFF3
    { HSPR_PUF3,   3,                     4, NULL,                   HS_NULL             }, // HS_STAFFPUFF4
    { HSPR_PUF4,  (0 | FF_FULLBRIGHT),    4, NULL,                   HS_STAFFPUFF2_2     }, // HS_STAFFPUFF2_1
    { HSPR_PUF4,  (1 | FF_FULLBRIGHT),    4, NULL,                   HS_STAFFPUFF2_3     }, // HS_STAFFPUFF2_2
    { HSPR_PUF4,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_STAFFPUFF2_4     }, // HS_STAFFPUFF2_3
    { HSPR_PUF4,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_STAFFPUFF2_5     }, // HS_STAFFPUFF2_4
    { HSPR_PUF4,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_STAFFPUFF2_6     }, // HS_STAFFPUFF2_5
    { HSPR_PUF4,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_STAFFPUFF2_6
    { HSPR_BEAK,   0,                     1, A_BeakReady,            HS_BEAKREADY        }, // HS_BEAKREADY
    { HSPR_BEAK,   0,                     1, A_Lower,                HS_BEAKDOWN         }, // HS_BEAKDOWN
    { HSPR_BEAK,   0,                     1, A_BeakRaise,            HS_BEAKUP           }, // HS_BEAKUP
    { HSPR_BEAK,   0,                    18, A_BeakAttackPL1,        HS_BEAKREADY        }, // HS_BEAKATK1_1
    { HSPR_BEAK,   0,                    12, A_BeakAttackPL2,        HS_BEAKREADY        }, // HS_BEAKATK2_1
    { HSPR_WGNT,   0,                    -1, NULL,                   HS_NULL             }, // HS_WGNT
    { HSPR_GAUN,   0,                     1, A_WeaponReady,          HS_GAUNTLETREADY    }, // HS_GAUNTLETREADY
    { HSPR_GAUN,   0,                     1, A_Lower,                HS_GAUNTLETDOWN     }, // HS_GAUNTLETDOWN
    { HSPR_GAUN,   0,                     1, A_Raise,                HS_GAUNTLETUP       }, // HS_GAUNTLETUP
    { HSPR_GAUN,   6,                     4, A_WeaponReady,          HS_GAUNTLETREADY2_2 }, // HS_GAUNTLETREADY2_1
    { HSPR_GAUN,   7,                     4, A_WeaponReady,          HS_GAUNTLETREADY2_3 }, // HS_GAUNTLETREADY2_2
    { HSPR_GAUN,   8,                     4, A_WeaponReady,          HS_GAUNTLETREADY2_1 }, // HS_GAUNTLETREADY2_3
    { HSPR_GAUN,   6,                     1, A_Lower,                HS_GAUNTLETDOWN2    }, // HS_GAUNTLETDOWN2
    { HSPR_GAUN,   6,                     1, A_Raise,                HS_GAUNTLETUP2      }, // HS_GAUNTLETUP2
    { HSPR_GAUN,   1,                     4, NULL,                   HS_GAUNTLETATK1_2   }, // HS_GAUNTLETATK1_1
    { HSPR_GAUN,   2,                     4, NULL,                   HS_GAUNTLETATK1_3   }, // HS_GAUNTLETATK1_2
    { HSPR_GAUN,  (3 | FF_FULLBRIGHT),    4, A_GauntletAttack,       HS_GAUNTLETATK1_4   }, // HS_GAUNTLETATK1_3
    { HSPR_GAUN,  (4 | FF_FULLBRIGHT),    4, A_GauntletAttack,       HS_GAUNTLETATK1_5   }, // HS_GAUNTLETATK1_4
    { HSPR_GAUN,  (5 | FF_FULLBRIGHT),    4, A_GauntletAttack,       HS_GAUNTLETATK1_6   }, // HS_GAUNTLETATK1_5
    { HSPR_GAUN,   2,                     4, A_ReFire,               HS_GAUNTLETATK1_7   }, // HS_GAUNTLETATK1_6
    { HSPR_GAUN,   1,                     4, A_Light0,               HS_GAUNTLETREADY    }, // HS_GAUNTLETATK1_7
    { HSPR_GAUN,   9,                     4, NULL,                   HS_GAUNTLETATK2_2   }, // HS_GAUNTLETATK2_1
    { HSPR_GAUN,  10,                     4, NULL,                   HS_GAUNTLETATK2_3   }, // HS_GAUNTLETATK2_2
    { HSPR_GAUN, (11 | FF_FULLBRIGHT),    4, A_GauntletAttack,       HS_GAUNTLETATK2_4   }, // HS_GAUNTLETATK2_3
    { HSPR_GAUN, (12 | FF_FULLBRIGHT),    4, A_GauntletAttack,       HS_GAUNTLETATK2_5   }, // HS_GAUNTLETATK2_4
    { HSPR_GAUN, (13 | FF_FULLBRIGHT),    4, A_GauntletAttack,       HS_GAUNTLETATK2_6   }, // HS_GAUNTLETATK2_5
    { HSPR_GAUN,  10,                     4, A_ReFire,               HS_GAUNTLETATK2_7   }, // HS_GAUNTLETATK2_6
    { HSPR_GAUN,   9,                     4, A_Light0,               HS_GAUNTLETREADY2_1 }, // HS_GAUNTLETATK2_7
    { HSPR_PUF1,  (0 | FF_FULLBRIGHT),    4, NULL,                   HS_GAUNTLETPUFF1_2  }, // HS_GAUNTLETPUFF1_1
    { HSPR_PUF1,  (1 | FF_FULLBRIGHT),    4, NULL,                   HS_GAUNTLETPUFF1_3  }, // HS_GAUNTLETPUFF1_2
    { HSPR_PUF1,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_GAUNTLETPUFF1_4  }, // HS_GAUNTLETPUFF1_3
    { HSPR_PUF1,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_GAUNTLETPUFF1_4
    { HSPR_PUF1,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_GAUNTLETPUFF2_2  }, // HS_GAUNTLETPUFF2_1
    { HSPR_PUF1,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_GAUNTLETPUFF2_3  }, // HS_GAUNTLETPUFF2_2
    { HSPR_PUF1,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_GAUNTLETPUFF2_4  }, // HS_GAUNTLETPUFF2_3
    { HSPR_PUF1,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_GAUNTLETPUFF2_4
    { HSPR_WBLS,   0,                    -1, NULL,                   HS_NULL             }, // HS_BLSR
    { HSPR_BLSR,   0,                     1, A_WeaponReady,          HS_BLASTERREADY     }, // HS_BLASTERREADY
    { HSPR_BLSR,   0,                     1, A_Lower,                HS_BLASTERDOWN      }, // HS_BLASTERDOWN
    { HSPR_BLSR,   0,                     1, A_Raise,                HS_BLASTERUP        }, // HS_BLASTERUP
    { HSPR_BLSR,   1,                     3, NULL,                   HS_BLASTERATK1_2    }, // HS_BLASTERATK1_1
    { HSPR_BLSR,   2,                     3, NULL,                   HS_BLASTERATK1_3    }, // HS_BLASTERATK1_2
    { HSPR_BLSR,   3,                     2, A_FireBlasterPL1,       HS_BLASTERATK1_4    }, // HS_BLASTERATK1_3
    { HSPR_BLSR,   2,                     2, NULL,                   HS_BLASTERATK1_5    }, // HS_BLASTERATK1_4
    { HSPR_BLSR,   1,                     2, NULL,                   HS_BLASTERATK1_6    }, // HS_BLASTERATK1_5
    { HSPR_BLSR,   0,                     0, A_ReFire,               HS_BLASTERREADY     }, // HS_BLASTERATK1_6
    { HSPR_BLSR,   1,                     0, NULL,                   HS_BLASTERATK2_2    }, // HS_BLASTERATK2_1
    { HSPR_BLSR,   2,                     0, NULL,                   HS_BLASTERATK2_3    }, // HS_BLASTERATK2_2
    { HSPR_BLSR,   3,                     3, A_FireBlasterPL2,       HS_BLASTERATK2_4    }, // HS_BLASTERATK2_3
    { HSPR_BLSR,   2,                     4, NULL,                   HS_BLASTERATK2_5    }, // HS_BLASTERATK2_4
    { HSPR_BLSR,   1,                     4, NULL,                   HS_BLASTERATK2_6    }, // HS_BLASTERATK2_5
    { HSPR_BLSR,   0,                     0, A_ReFire,               HS_BLASTERREADY     }, // HS_BLASTERATK2_6
    { HSPR_ACLO,   4,                   200, NULL,                   HS_BLASTERFX1_1     }, // HS_BLASTERFX1_1
    { HSPR_FX18,  (0 | FF_FULLBRIGHT),    3, A_SpawnRippers,         HS_BLASTERFXI1_2    }, // HS_BLASTERFXI1_1
    { HSPR_FX18,  (1 | FF_FULLBRIGHT),    3, NULL,                   HS_BLASTERFXI1_3    }, // HS_BLASTERFXI1_2
    { HSPR_FX18,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERFXI1_4    }, // HS_BLASTERFXI1_3
    { HSPR_FX18,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERFXI1_5    }, // HS_BLASTERFXI1_4
    { HSPR_FX18,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERFXI1_6    }, // HS_BLASTERFXI1_5
    { HSPR_FX18,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERFXI1_7    }, // HS_BLASTERFXI1_6
    { HSPR_FX18,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_BLASTERFXI1_7
    { HSPR_FX18,   7,                     4, NULL,                   HS_BLASTERSMOKE2    }, // HS_BLASTERSMOKE1
    { HSPR_FX18,   8,                     4, NULL,                   HS_BLASTERSMOKE3    }, // HS_BLASTERSMOKE2
    { HSPR_FX18,   9,                     4, NULL,                   HS_BLASTERSMOKE4    }, // HS_BLASTERSMOKE3
    { HSPR_FX18,  10,                     4, NULL,                   HS_BLASTERSMOKE5    }, // HS_BLASTERSMOKE4
    { HSPR_FX18,  11,                     4, NULL,                   HS_NULL             }, // HS_BLASTERSMOKE5
    { HSPR_FX18,  12,                     4, NULL,                   HS_RIPPER2          }, // HS_RIPPER1
    { HSPR_FX18,  13,                     5, NULL,                   HS_RIPPER1          }, // HS_RIPPER2
    { HSPR_FX18, (14 | FF_FULLBRIGHT),    4, NULL,                   HS_RIPPERX2         }, // HS_RIPPERX1
    { HSPR_FX18, (15 | FF_FULLBRIGHT),    4, NULL,                   HS_RIPPERX3         }, // HS_RIPPERX2
    { HSPR_FX18, (16 | FF_FULLBRIGHT),    4, NULL,                   HS_RIPPERX4         }, // HS_RIPPERX3
    { HSPR_FX18, (17 | FF_FULLBRIGHT),    4, NULL,                   HS_RIPPERX5         }, // HS_RIPPERX4
    { HSPR_FX18, (18 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RIPPERX5
    { HSPR_FX17,  (0 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERPUFF1_2   }, // HS_BLASTERPUFF1_1
    { HSPR_FX17,  (1 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERPUFF1_3   }, // HS_BLASTERPUFF1_2
    { HSPR_FX17,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERPUFF1_4   }, // HS_BLASTERPUFF1_3
    { HSPR_FX17,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERPUFF1_5   }, // HS_BLASTERPUFF1_4
    { HSPR_FX17,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_BLASTERPUFF1_5
    { HSPR_FX17,  (5 | FF_FULLBRIGHT),    3, NULL,                   HS_BLASTERPUFF2_2   }, // HS_BLASTERPUFF2_1
    { HSPR_FX17,  (6 | FF_FULLBRIGHT),    3, NULL,                   HS_BLASTERPUFF2_3   }, // HS_BLASTERPUFF2_2
    { HSPR_FX17,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERPUFF2_4   }, // HS_BLASTERPUFF2_3
    { HSPR_FX17,  (8 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERPUFF2_5   }, // HS_BLASTERPUFF2_4
    { HSPR_FX17,  (9 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERPUFF2_6   }, // HS_BLASTERPUFF2_5
    { HSPR_FX17, (10 | FF_FULLBRIGHT),    4, NULL,                   HS_BLASTERPUFF2_7   }, // HS_BLASTERPUFF2_6
    { HSPR_FX17, (11 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_BLASTERPUFF2_7
    { HSPR_WMCE,   0,                    -1, NULL,                   HS_NULL             }, // HS_WMCE
    { HSPR_MACE,   0,                     1, A_WeaponReady,          HS_MACEREADY        }, // HS_MACEREADY
    { HSPR_MACE,   0,                     1, A_Lower,                HS_MACEDOWN         }, // HS_MACEDOWN
    { HSPR_MACE,   0,                     1, A_Raise,                HS_MACEUP           }, // HS_MACEUP
    { HSPR_MACE,   1,                     4, NULL,                   HS_MACEATK1_2       }, // HS_MACEATK1_1
    { HSPR_MACE,   2,                     3, A_FireMacePL1,          HS_MACEATK1_3       }, // HS_MACEATK1_2
    { HSPR_MACE,   3,                     3, A_FireMacePL1,          HS_MACEATK1_4       }, // HS_MACEATK1_3
    { HSPR_MACE,   4,                     3, A_FireMacePL1,          HS_MACEATK1_5       }, // HS_MACEATK1_4
    { HSPR_MACE,   5,                     3, A_FireMacePL1,          HS_MACEATK1_6       }, // HS_MACEATK1_5
    { HSPR_MACE,   2,                     4, A_ReFire,               HS_MACEATK1_7       }, // HS_MACEATK1_6
    { HSPR_MACE,   3,                     4, NULL,                   HS_MACEATK1_8       }, // HS_MACEATK1_7
    { HSPR_MACE,   4,                     4, NULL,                   HS_MACEATK1_9       }, // HS_MACEATK1_8
    { HSPR_MACE,   5,                     4, NULL,                   HS_MACEATK1_10      }, // HS_MACEATK1_9
    { HSPR_MACE,   1,                     4, NULL,                   HS_MACEREADY        }, // HS_MACEATK1_10
    { HSPR_MACE,   1,                     4, NULL,                   HS_MACEATK2_2       }, // HS_MACEATK2_1
    { HSPR_MACE,   3,                     4, A_FireMacePL2,          HS_MACEATK2_3       }, // HS_MACEATK2_2
    { HSPR_MACE,   1,                     4, NULL,                   HS_MACEATK2_4       }, // HS_MACEATK2_3
    { HSPR_MACE,   0,                     8, A_ReFire,               HS_MACEREADY        }, // HS_MACEATK2_4
    { HSPR_FX02,   0,                     4, A_MacePL1Check,         HS_MACEFX1_2        }, // HS_MACEFX1_1
    { HSPR_FX02,   1,                     4, A_MacePL1Check,         HS_MACEFX1_1        }, // HS_MACEFX1_2
    { HSPR_FX02,  (5 | FF_FULLBRIGHT),    4, A_MaceBallImpact,       HS_MACEFXI1_2       }, // HS_MACEFXI1_1
    { HSPR_FX02,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_MACEFXI1_3       }, // HS_MACEFXI1_2
    { HSPR_FX02,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_MACEFXI1_4       }, // HS_MACEFXI1_3
    { HSPR_FX02,  (8 | FF_FULLBRIGHT),    4, NULL,                   HS_MACEFXI1_5       }, // HS_MACEFXI1_4
    { HSPR_FX02,  (9 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_MACEFXI1_5
    { HSPR_FX02,   2,                     4, NULL,                   HS_MACEFX2_2        }, // HS_MACEFX2_1
    { HSPR_FX02,   3,                     4, NULL,                   HS_MACEFX2_1        }, // HS_MACEFX2_2
    { HSPR_FX02,  (5 | FF_FULLBRIGHT),    4, A_MaceBallImpact2,      HS_MACEFXI1_2       }, // HS_MACEFXI2_1
    { HSPR_FX02,   0,                     4, NULL,                   HS_MACEFX3_2        }, // HS_MACEFX3_1
    { HSPR_FX02,   1,                     4, NULL,                   HS_MACEFX3_1        }, // HS_MACEFX3_2
    { HSPR_FX02,   4,                    99, NULL,                   HS_MACEFX4_1        }, // HS_MACEFX4_1
    { HSPR_FX02,  (2 | FF_FULLBRIGHT),    4, A_DeathBallImpact,      HS_MACEFXI1_2       }, // HS_MACEFXI4_1
    { HSPR_WSKL,   0,                    -1, NULL,                   HS_NULL             }, // HS_WSKL
    { HSPR_HROD,   0,                     1, A_WeaponReady,          HS_HORNRODREADY     }, // HS_HORNRODREADY
    { HSPR_HROD,   0,                     1, A_Lower,                HS_HORNRODDOWN      }, // HS_HORNRODDOWN
    { HSPR_HROD,   0,                     1, A_Raise,                HS_HORNRODUP        }, // HS_HORNRODUP
    { HSPR_HROD,   0,                     4, A_FireSkullRodPL1,      HS_HORNRODATK1_2    }, // HS_HORNRODATK1_1
    { HSPR_HROD,   1,                     4, A_FireSkullRodPL1,      HS_HORNRODATK1_3    }, // HS_HORNRODATK1_2
    { HSPR_HROD,   1,                     0, A_ReFire,               HS_HORNRODREADY     }, // HS_HORNRODATK1_3
    { HSPR_HROD,   2,                     2, NULL,                   HS_HORNRODATK2_2    }, // HS_HORNRODATK2_1
    { HSPR_HROD,   3,                     3, NULL,                   HS_HORNRODATK2_3    }, // HS_HORNRODATK2_2
    { HSPR_HROD,   4,                     2, NULL,                   HS_HORNRODATK2_4    }, // HS_HORNRODATK2_3
    { HSPR_HROD,   5,                     3, NULL,                   HS_HORNRODATK2_5    }, // HS_HORNRODATK2_4
    { HSPR_HROD,   6,                     4, A_FireSkullRodPL2,      HS_HORNRODATK2_6    }, // HS_HORNRODATK2_5
    { HSPR_HROD,   5,                     2, NULL,                   HS_HORNRODATK2_7    }, // HS_HORNRODATK2_6
    { HSPR_HROD,   4,                     3, NULL,                   HS_HORNRODATK2_8    }, // HS_HORNRODATK2_7
    { HSPR_HROD,   3,                     2, NULL,                   HS_HORNRODATK2_9    }, // HS_HORNRODATK2_8
    { HSPR_HROD,   2,                     2, A_ReFire,               HS_HORNRODREADY     }, // HS_HORNRODATK2_9
    { HSPR_FX00,  (0 | FF_FULLBRIGHT),    6, NULL,                   HS_HRODFX1_2        }, // HS_HRODFX1_1
    { HSPR_FX00,  (1 | FF_FULLBRIGHT),    6, NULL,                   HS_HRODFX1_1        }, // HS_HRODFX1_2
    { HSPR_FX00,  (7 | FF_FULLBRIGHT),    5, NULL,                   HS_HRODFXI1_2       }, // HS_HRODFXI1_1
    { HSPR_FX00,  (8 | FF_FULLBRIGHT),    5, NULL,                   HS_HRODFXI1_3       }, // HS_HRODFXI1_2
    { HSPR_FX00,  (9 | FF_FULLBRIGHT),    4, NULL,                   HS_HRODFXI1_4       }, // HS_HRODFXI1_3
    { HSPR_FX00, (10 | FF_FULLBRIGHT),    4, NULL,                   HS_HRODFXI1_5       }, // HS_HRODFXI1_4
    { HSPR_FX00, (11 | FF_FULLBRIGHT),    3, NULL,                   HS_HRODFXI1_6       }, // HS_HRODFXI1_5
    { HSPR_FX00, (12 | FF_FULLBRIGHT),    3, NULL,                   HS_NULL             }, // HS_HRODFXI1_6
    { HSPR_FX00,  (2 | FF_FULLBRIGHT),    3, NULL,                   HS_HRODFX2_2        }, // HS_HRODFX2_1
    { HSPR_FX00,  (3 | FF_FULLBRIGHT),    3, A_SkullRodPL2Seek,      HS_HRODFX2_3        }, // HS_HRODFX2_2
    { HSPR_FX00,  (4 | FF_FULLBRIGHT),    3, NULL,                   HS_HRODFX2_4        }, // HS_HRODFX2_3
    { HSPR_FX00,  (5 | FF_FULLBRIGHT),    3, A_SkullRodPL2Seek,      HS_HRODFX2_1        }, // HS_HRODFX2_4
    { HSPR_FX00,  (7 | FF_FULLBRIGHT),    5, A_AddPlayerRain,        HS_HRODFXI2_2       }, // HS_HRODFXI2_1
    { HSPR_FX00,  (8 | FF_FULLBRIGHT),    5, NULL,                   HS_HRODFXI2_3       }, // HS_HRODFXI2_2
    { HSPR_FX00,  (9 | FF_FULLBRIGHT),    4, NULL,                   HS_HRODFXI2_4       }, // HS_HRODFXI2_3
    { HSPR_FX00, (10 | FF_FULLBRIGHT),    3, NULL,                   HS_HRODFXI2_5       }, // HS_HRODFXI2_4
    { HSPR_FX00, (11 | FF_FULLBRIGHT),    3, NULL,                   HS_HRODFXI2_6       }, // HS_HRODFXI2_5
    { HSPR_FX00, (12 | FF_FULLBRIGHT),    3, NULL,                   HS_HRODFXI2_7       }, // HS_HRODFXI2_6
    { HSPR_FX00,   6,                     1, A_HideInCeiling,        HS_HRODFXI2_8       }, // HS_HRODFXI2_7
    { HSPR_FX00,   6,                     1, A_SkullRodStorm,        HS_HRODFXI2_8       }, // HS_HRODFXI2_8
    { HSPR_FX20,  (0 | FF_FULLBRIGHT),   -1, NULL,                   HS_NULL             }, // HS_RAINPLR1_1
    { HSPR_FX21,  (0 | FF_FULLBRIGHT),   -1, NULL,                   HS_NULL             }, // HS_RAINPLR2_1
    { HSPR_FX22,  (0 | FF_FULLBRIGHT),   -1, NULL,                   HS_NULL             }, // HS_RAINPLR3_1
    { HSPR_FX23,  (0 | FF_FULLBRIGHT),   -1, NULL,                   HS_NULL             }, // HS_RAINPLR4_1
    { HSPR_FX20,  (1 | FF_FULLBRIGHT),    4, A_RainImpact,           HS_RAINPLR1X_2      }, // HS_RAINPLR1X_1
    { HSPR_FX20,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR1X_3      }, // HS_RAINPLR1X_2
    { HSPR_FX20,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR1X_4      }, // HS_RAINPLR1X_3
    { HSPR_FX20,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR1X_5      }, // HS_RAINPLR1X_4
    { HSPR_FX20,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RAINPLR1X_5
    { HSPR_FX21,  (1 | FF_FULLBRIGHT),    4, A_RainImpact,           HS_RAINPLR2X_2      }, // HS_RAINPLR2X_1
    { HSPR_FX21,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR2X_3      }, // HS_RAINPLR2X_2
    { HSPR_FX21,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR2X_4      }, // HS_RAINPLR2X_3
    { HSPR_FX21,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR2X_5      }, // HS_RAINPLR2X_4
    { HSPR_FX21,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RAINPLR2X_5
    { HSPR_FX22,  (1 | FF_FULLBRIGHT),    4, A_RainImpact,           HS_RAINPLR3X_2      }, // HS_RAINPLR3X_1
    { HSPR_FX22,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR3X_3      }, // HS_RAINPLR3X_2
    { HSPR_FX22,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR3X_4      }, // HS_RAINPLR3X_3
    { HSPR_FX22,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR3X_5      }, // HS_RAINPLR3X_4
    { HSPR_FX22,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RAINPLR3X_5
    { HSPR_FX23,  (1 | FF_FULLBRIGHT),    4, A_RainImpact,           HS_RAINPLR4X_2      }, // HS_RAINPLR4X_1
    { HSPR_FX23,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR4X_3      }, // HS_RAINPLR4X_2
    { HSPR_FX23,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR4X_4      }, // HS_RAINPLR4X_3
    { HSPR_FX23,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINPLR4X_5      }, // HS_RAINPLR4X_4
    { HSPR_FX23,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RAINPLR4X_5
    { HSPR_FX20,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINAIRXPLR1_2   }, // HS_RAINAIRXPLR1_1
    { HSPR_FX21,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINAIRXPLR2_2   }, // HS_RAINAIRXPLR2_1
    { HSPR_FX22,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINAIRXPLR3_2   }, // HS_RAINAIRXPLR3_1
    { HSPR_FX23,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINAIRXPLR4_2   }, // HS_RAINAIRXPLR4_1
    { HSPR_FX20,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINAIRXPLR1_3   }, // HS_RAINAIRXPLR1_2
    { HSPR_FX21,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINAIRXPLR2_3   }, // HS_RAINAIRXPLR2_2
    { HSPR_FX22,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINAIRXPLR3_3   }, // HS_RAINAIRXPLR3_2
    { HSPR_FX23,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_RAINAIRXPLR4_3   }, // HS_RAINAIRXPLR4_2
    { HSPR_FX20,  (8 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RAINAIRXPLR1_3
    { HSPR_FX21,  (8 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RAINAIRXPLR2_3
    { HSPR_FX22,  (8 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RAINAIRXPLR3_3
    { HSPR_FX23,  (8 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_RAINAIRXPLR4_3
    { HSPR_GWND,   0,                     1, A_WeaponReady,          HS_GOLDWANDREADY    }, // HS_GOLDWANDREADY
    { HSPR_GWND,   0,                     1, A_Lower,                HS_GOLDWANDDOWN     }, // HS_GOLDWANDDOWN
    { HSPR_GWND,   0,                     1, A_Raise,                HS_GOLDWANDUP       }, // HS_GOLDWANDUP
    { HSPR_GWND,   1,                     3, NULL,                   HS_GOLDWANDATK1_2   }, // HS_GOLDWANDATK1_1
    { HSPR_GWND,   2,                     5, A_FireGoldWandPL1,      HS_GOLDWANDATK1_3   }, // HS_GOLDWANDATK1_2
    { HSPR_GWND,   3,                     3, NULL,                   HS_GOLDWANDATK1_4   }, // HS_GOLDWANDATK1_3
    { HSPR_GWND,   3,                     0, A_ReFire,               HS_GOLDWANDREADY    }, // HS_GOLDWANDATK1_4
    { HSPR_GWND,   1,                     3, NULL,                   HS_GOLDWANDATK2_2   }, // HS_GOLDWANDATK2_1
    { HSPR_GWND,   2,                     4, A_FireGoldWandPL2,      HS_GOLDWANDATK2_3   }, // HS_GOLDWANDATK2_2
    { HSPR_GWND,   3,                     3, NULL,                   HS_GOLDWANDATK2_4   }, // HS_GOLDWANDATK2_3
    { HSPR_GWND,   3,                     0, A_ReFire,               HS_GOLDWANDREADY    }, // HS_GOLDWANDATK2_4
    { HSPR_FX01,  (0 | FF_FULLBRIGHT),    6, NULL,                   HS_GWANDFX1_2       }, // HS_GWANDFX1_1
    { HSPR_FX01,  (1 | FF_FULLBRIGHT),    6, NULL,                   HS_GWANDFX1_1       }, // HS_GWANDFX1_2
    { HSPR_FX01,  (4 | FF_FULLBRIGHT),    3, NULL,                   HS_GWANDFXI1_2      }, // HS_GWANDFXI1_1
    { HSPR_FX01,  (5 | FF_FULLBRIGHT),    3, NULL,                   HS_GWANDFXI1_3      }, // HS_GWANDFXI1_2
    { HSPR_FX01,  (6 | FF_FULLBRIGHT),    3, NULL,                   HS_GWANDFXI1_4      }, // HS_GWANDFXI1_3
    { HSPR_FX01,  (7 | FF_FULLBRIGHT),    3, NULL,                   HS_NULL             }, // HS_GWANDFXI1_4
    { HSPR_FX01,  (2 | FF_FULLBRIGHT),    6, NULL,                   HS_GWANDFX2_2       }, // HS_GWANDFX2_1
    { HSPR_FX01,  (3 | FF_FULLBRIGHT),    6, NULL,                   HS_GWANDFX2_1       }, // HS_GWANDFX2_2
    { HSPR_PUF2,  (0 | FF_FULLBRIGHT),    3, NULL,                   HS_GWANDPUFF1_2     }, // HS_GWANDPUFF1_1
    { HSPR_PUF2,  (1 | FF_FULLBRIGHT),    3, NULL,                   HS_GWANDPUFF1_3     }, // HS_GWANDPUFF1_2
    { HSPR_PUF2,  (2 | FF_FULLBRIGHT),    3, NULL,                   HS_GWANDPUFF1_4     }, // HS_GWANDPUFF1_3
    { HSPR_PUF2,  (3 | FF_FULLBRIGHT),    3, NULL,                   HS_GWANDPUFF1_5     }, // HS_GWANDPUFF1_4
    { HSPR_PUF2,  (4 | FF_FULLBRIGHT),    3, NULL,                   HS_NULL             }, // HS_GWANDPUFF1_5
    { HSPR_WPHX,   0,                    -1, NULL,                   HS_NULL             }, // HS_WPHX
    { HSPR_PHNX,   0,                     1, A_WeaponReady,          HS_PHOENIXREADY     }, // HS_PHOENIXREADY
    { HSPR_PHNX,   0,                     1, A_Lower,                HS_PHOENIXDOWN      }, // HS_PHOENIXDOWN
    { HSPR_PHNX,   0,                     1, A_Raise,                HS_PHOENIXUP        }, // HS_PHOENIXUP
    { HSPR_PHNX,   1,                     5, NULL,                   HS_PHOENIXATK1_2    }, // HS_PHOENIXATK1_1
    { HSPR_PHNX,   2,                     7, A_FirePhoenixPL1,       HS_PHOENIXATK1_3    }, // HS_PHOENIXATK1_2
    { HSPR_PHNX,   3,                     4, NULL,                   HS_PHOENIXATK1_4    }, // HS_PHOENIXATK1_3
    { HSPR_PHNX,   1,                     4, NULL,                   HS_PHOENIXATK1_5    }, // HS_PHOENIXATK1_4
    { HSPR_PHNX,   1,                     0, A_ReFire,               HS_PHOENIXREADY     }, // HS_PHOENIXATK1_5
    { HSPR_PHNX,   1,                     3, A_InitPhoenixPL2,       HS_PHOENIXATK2_2    }, // HS_PHOENIXATK2_1
    { HSPR_PHNX,  (2 | FF_FULLBRIGHT),    1, A_FirePhoenixPL2,       HS_PHOENIXATK2_3    }, // HS_PHOENIXATK2_2
    { HSPR_PHNX,   1,                     4, A_ReFire,               HS_PHOENIXATK2_4    }, // HS_PHOENIXATK2_3
    { HSPR_PHNX,   1,                     4, A_ShutdownPhoenixPL2,   HS_PHOENIXREADY     }, // HS_PHOENIXATK2_4
    { HSPR_FX04,  (0 | FF_FULLBRIGHT),    4, A_PhoenixPuff,          HS_PHOENIXFX1_1     }, // HS_PHOENIXFX1_1
    { HSPR_FX08,  (0 | FF_FULLBRIGHT),    6, A_Explode,              HS_PHOENIXFXI1_2    }, // HS_PHOENIXFXI1_1
    { HSPR_FX08,  (1 | FF_FULLBRIGHT),    5, NULL,                   HS_PHOENIXFXI1_3    }, // HS_PHOENIXFXI1_2
    { HSPR_FX08,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_PHOENIXFXI1_4    }, // HS_PHOENIXFXI1_3
    { HSPR_FX08,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_PHOENIXFXI1_5    }, // HS_PHOENIXFXI1_4
    { HSPR_FX08,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_PHOENIXFXI1_6    }, // HS_PHOENIXFXI1_5
    { HSPR_FX08,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_PHOENIXFXI1_7    }, // HS_PHOENIXFXI1_6
    { HSPR_FX08,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_PHOENIXFXI1_8    }, // HS_PHOENIXFXI1_7
    { HSPR_FX08,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_PHOENIXFXI1_8
    { HSPR_FX08,  (8 | FF_FULLBRIGHT),    8, NULL,                   HS_PHOENIXFXIX_1    }, // HS_PHOENIXFXIX_1
    { HSPR_FX08,  (9 | FF_FULLBRIGHT),    8, NULL,                   HS_PHOENIXFXIX_2    }, // HS_PHOENIXFXIX_2
    { HSPR_FX08, (10 | FF_FULLBRIGHT),    8, NULL,                   HS_NULL             }, // HS_PHOENIXFXIX_3
    { HSPR_FX04,   1,                     4, NULL,                   HS_PHOENIXPUFF2     }, // HS_PHOENIXPUFF1
    { HSPR_FX04,   2,                     4, NULL,                   HS_PHOENIXPUFF3     }, // HS_PHOENIXPUFF2
    { HSPR_FX04,   3,                     4, NULL,                   HS_PHOENIXPUFF4     }, // HS_PHOENIXPUFF3
    { HSPR_FX04,   4,                     4, NULL,                   HS_PHOENIXPUFF5     }, // HS_PHOENIXPUFF4
    { HSPR_FX04,   5,                     4, NULL,                   HS_NULL             }, // HS_PHOENIXPUFF5
    { HSPR_FX09,  (0 | FF_FULLBRIGHT),    2, NULL,                   HS_PHOENIXFX2_2     }, // HS_PHOENIXFX2_1
    { HSPR_FX09,  (1 | FF_FULLBRIGHT),    2, NULL,                   HS_PHOENIXFX2_3     }, // HS_PHOENIXFX2_2
    { HSPR_FX09,  (0 | FF_FULLBRIGHT),    2, NULL,                   HS_PHOENIXFX2_4     }, // HS_PHOENIXFX2_3
    { HSPR_FX09,  (1 | FF_FULLBRIGHT),    2, NULL,                   HS_PHOENIXFX2_5     }, // HS_PHOENIXFX2_4
    { HSPR_FX09,  (0 | FF_FULLBRIGHT),    2, NULL,                   HS_PHOENIXFX2_6     }, // HS_PHOENIXFX2_5
    { HSPR_FX09,  (1 | FF_FULLBRIGHT),    2, A_FlameEnd,             HS_PHOENIXFX2_7     }, // HS_PHOENIXFX2_6
    { HSPR_FX09,  (2 | FF_FULLBRIGHT),    2, NULL,                   HS_PHOENIXFX2_8     }, // HS_PHOENIXFX2_7
    { HSPR_FX09,  (3 | FF_FULLBRIGHT),    2, NULL,                   HS_PHOENIXFX2_9     }, // HS_PHOENIXFX2_8
    { HSPR_FX09,  (4 | FF_FULLBRIGHT),    2, NULL,                   HS_PHOENIXFX2_10    }, // HS_PHOENIXFX2_9
    { HSPR_FX09,  (5 | FF_FULLBRIGHT),    2, NULL,                   HS_NULL             }, // HS_PHOENIXFX2_10
    { HSPR_FX09,  (6 | FF_FULLBRIGHT),    3, NULL,                   HS_PHOENIXFXI2_2    }, // HS_PHOENIXFXI2_1
    { HSPR_FX09,  (7 | FF_FULLBRIGHT),    3, A_FloatPuff,            HS_PHOENIXFXI2_3    }, // HS_PHOENIXFXI2_2
    { HSPR_FX09,  (8 | FF_FULLBRIGHT),    4, NULL,                   HS_PHOENIXFXI2_4    }, // HS_PHOENIXFXI2_3
    { HSPR_FX09,  (9 | FF_FULLBRIGHT),    5, NULL,                   HS_PHOENIXFXI2_5    }, // HS_PHOENIXFXI2_4
    { HSPR_FX09, (10 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_PHOENIXFXI2_5
    { HSPR_WBOW,   0,                    -1, NULL,                   HS_NULL             }, // HS_WBOW
    { HSPR_CRBW,   0,                     1, A_WeaponReady,          HS_CRBOW2           }, // HS_CRBOW1
    { HSPR_CRBW,   0,                     1, A_WeaponReady,          HS_CRBOW3           }, // HS_CRBOW2
    { HSPR_CRBW,   0,                     1, A_WeaponReady,          HS_CRBOW4           }, // HS_CRBOW3
    { HSPR_CRBW,   0,                     1, A_WeaponReady,          HS_CRBOW5           }, // HS_CRBOW4
    { HSPR_CRBW,   0,                     1, A_WeaponReady,          HS_CRBOW6           }, // HS_CRBOW5
    { HSPR_CRBW,   0,                     1, A_WeaponReady,          HS_CRBOW7           }, // HS_CRBOW6
    { HSPR_CRBW,   1,                     1, A_WeaponReady,          HS_CRBOW8           }, // HS_CRBOW7
    { HSPR_CRBW,   1,                     1, A_WeaponReady,          HS_CRBOW9           }, // HS_CRBOW8
    { HSPR_CRBW,   1,                     1, A_WeaponReady,          HS_CRBOW10          }, // HS_CRBOW9
    { HSPR_CRBW,   1,                     1, A_WeaponReady,          HS_CRBOW11          }, // HS_CRBOW10
    { HSPR_CRBW,   1,                     1, A_WeaponReady,          HS_CRBOW12          }, // HS_CRBOW11
    { HSPR_CRBW,   1,                     1, A_WeaponReady,          HS_CRBOW13          }, // HS_CRBOW12
    { HSPR_CRBW,   2,                     1, A_WeaponReady,          HS_CRBOW14          }, // HS_CRBOW13
    { HSPR_CRBW,   2,                     1, A_WeaponReady,          HS_CRBOW15          }, // HS_CRBOW14
    { HSPR_CRBW,   2,                     1, A_WeaponReady,          HS_CRBOW16          }, // HS_CRBOW15
    { HSPR_CRBW,   2,                     1, A_WeaponReady,          HS_CRBOW17          }, // HS_CRBOW16
    { HSPR_CRBW,   2,                     1, A_WeaponReady,          HS_CRBOW18          }, // HS_CRBOW17
    { HSPR_CRBW,   2,                     1, A_WeaponReady,          HS_CRBOW1           }, // HS_CRBOW18
    { HSPR_CRBW,   0,                     1, A_Lower,                HS_CRBOWDOWN        }, // HS_CRBOWDOWN
    { HSPR_CRBW,   0,                     1, A_Raise,                HS_CRBOWUP          }, // HS_CRBOWUP
    { HSPR_CRBW,   3,                     6, A_FireCrossbowPL1,      HS_CRBOWATK1_2      }, // HS_CRBOWATK1_1
    { HSPR_CRBW,   4,                     3, NULL,                   HS_CRBOWATK1_3      }, // HS_CRBOWATK1_2
    { HSPR_CRBW,   5,                     3, NULL,                   HS_CRBOWATK1_4      }, // HS_CRBOWATK1_3
    { HSPR_CRBW,   6,                     3, NULL,                   HS_CRBOWATK1_5      }, // HS_CRBOWATK1_4
    { HSPR_CRBW,   7,                     3, NULL,                   HS_CRBOWATK1_6      }, // HS_CRBOWATK1_5
    { HSPR_CRBW,   0,                     4, NULL,                   HS_CRBOWATK1_7      }, // HS_CRBOWATK1_6
    { HSPR_CRBW,   1,                     4, NULL,                   HS_CRBOWATK1_8      }, // HS_CRBOWATK1_7
    { HSPR_CRBW,   2,                     5, A_ReFire,               HS_CRBOW1           }, // HS_CRBOWATK1_8
    { HSPR_CRBW,   3,                     5, A_FireCrossbowPL2,      HS_CRBOWATK2_2      }, // HS_CRBOWATK2_1
    { HSPR_CRBW,   4,                     3, NULL,                   HS_CRBOWATK2_3      }, // HS_CRBOWATK2_2
    { HSPR_CRBW,   5,                     2, NULL,                   HS_CRBOWATK2_4      }, // HS_CRBOWATK2_3
    { HSPR_CRBW,   6,                     3, NULL,                   HS_CRBOWATK2_5      }, // HS_CRBOWATK2_4
    { HSPR_CRBW,   7,                     2, NULL,                   HS_CRBOWATK2_6      }, // HS_CRBOWATK2_5
    { HSPR_CRBW,   0,                     3, NULL,                   HS_CRBOWATK2_7      }, // HS_CRBOWATK2_6
    { HSPR_CRBW,   1,                     3, NULL,                   HS_CRBOWATK2_8      }, // HS_CRBOWATK2_7
    { HSPR_CRBW,   2,                     4, A_ReFire,               HS_CRBOW1           }, // HS_CRBOWATK2_8
    { HSPR_FX03,  (1 | FF_FULLBRIGHT),    1, NULL,                   HS_CRBOWFX1         }, // HS_CRBOWFX1
    { HSPR_FX03,  (7 | FF_FULLBRIGHT),    8, NULL,                   HS_CRBOWFXI1_2      }, // HS_CRBOWFXI1_1
    { HSPR_FX03,  (8 | FF_FULLBRIGHT),    8, NULL,                   HS_CRBOWFXI1_3      }, // HS_CRBOWFXI1_2
    { HSPR_FX03,  (9 | FF_FULLBRIGHT),    8, NULL,                   HS_NULL             }, // HS_CRBOWFXI1_3
    { HSPR_FX03,  (1 | FF_FULLBRIGHT),    1, A_BoltSpark,            HS_CRBOWFX2         }, // HS_CRBOWFX2
    { HSPR_FX03,  (0 | FF_FULLBRIGHT),    1, NULL,                   HS_CRBOWFX3         }, // HS_CRBOWFX3
    { HSPR_FX03,  (2 | FF_FULLBRIGHT),    8, NULL,                   HS_CRBOWFXI3_2      }, // HS_CRBOWFXI3_1
    { HSPR_FX03,  (3 | FF_FULLBRIGHT),    8, NULL,                   HS_CRBOWFXI3_3      }, // HS_CRBOWFXI3_2
    { HSPR_FX03,  (4 | FF_FULLBRIGHT),    8, NULL,                   HS_NULL             }, // HS_CRBOWFXI3_3
    { HSPR_FX03,  (5 | FF_FULLBRIGHT),    8, NULL,                   HS_CRBOWFX4_2       }, // HS_CRBOWFX4_1
    { HSPR_FX03,  (6 | FF_FULLBRIGHT),    8, NULL,                   HS_NULL             }, // HS_CRBOWFX4_2
    { HSPR_BLOD,   2,                     8, NULL,                   HS_BLOOD2           }, // HS_BLOOD1
    { HSPR_BLOD,   1,                     8, NULL,                   HS_BLOOD3           }, // HS_BLOOD2
    { HSPR_BLOD,   0,                     8, NULL,                   HS_NULL             }, // HS_BLOOD3
    { HSPR_BLOD,   2,                     8, NULL,                   HS_BLOODSPLATTER2   }, // HS_BLOODSPLATTER1
    { HSPR_BLOD,   1,                     8, NULL,                   HS_BLOODSPLATTER3   }, // HS_BLOODSPLATTER2
    { HSPR_BLOD,   0,                     8, NULL,                   HS_NULL             }, // HS_BLOODSPLATTER3
    { HSPR_BLOD,   0,                     6, NULL,                   HS_NULL             }, // HS_BLOODSPLATTERX
    { HSPR_PLAY,   0,                    -1, NULL,                   HS_NULL             }, // HS_PLAY
    { HSPR_PLAY,   0,                     4, NULL,                   HS_PLAY_RUN2        }, // HS_PLAY_RUN1
    { HSPR_PLAY,   1,                     4, NULL,                   HS_PLAY_RUN3        }, // HS_PLAY_RUN2
    { HSPR_PLAY,   2,                     4, NULL,                   HS_PLAY_RUN4        }, // HS_PLAY_RUN3
    { HSPR_PLAY,   3,                     4, NULL,                   HS_PLAY_RUN1        }, // HS_PLAY_RUN4
    { HSPR_PLAY,   4,                    12, NULL,                   HS_PLAY             }, // HS_PLAY_ATK1
    { HSPR_PLAY,  (5 | FF_FULLBRIGHT),    6, NULL,                   HS_PLAY_ATK1        }, // HS_PLAY_ATK2
    { HSPR_PLAY,   6,                     4, NULL,                   HS_PLAY_PAIN2       }, // HS_PLAY_PAIN
    { HSPR_PLAY,   6,                     4, A_Pain,                 HS_PLAY             }, // HS_PLAY_PAIN2
    { HSPR_PLAY,   7,                     6, NULL,                   HS_PLAY_DIE2        }, // HS_PLAY_DIE1
    { HSPR_PLAY,   8,                     6, A_Scream,               HS_PLAY_DIE3        }, // HS_PLAY_DIE2
    { HSPR_PLAY,   9,                     6, NULL,                   HS_PLAY_DIE4        }, // HS_PLAY_DIE3
    { HSPR_PLAY,  10,                     6, NULL,                   HS_PLAY_DIE5        }, // HS_PLAY_DIE4
    { HSPR_PLAY,  11,                     6, A_NoBlocking,           HS_PLAY_DIE6        }, // HS_PLAY_DIE5
    { HSPR_PLAY,  12,                     6, NULL,                   HS_PLAY_DIE7        }, // HS_PLAY_DIE6
    { HSPR_PLAY,  13,                     6, NULL,                   HS_PLAY_DIE8        }, // HS_PLAY_DIE7
    { HSPR_PLAY,  14,                     6, NULL,                   HS_PLAY_DIE9        }, // HS_PLAY_DIE8
    { HSPR_PLAY,  15,                    -1, NULL,                   HS_NULL             }, // HS_PLAY_DIE9
    { HSPR_PLAY,  16,                     5, A_Scream,               HS_PLAY_XDIE2       }, // HS_PLAY_XDIE1
    { HSPR_PLAY,  17,                     5, A_SkullPop,             HS_PLAY_XDIE3       }, // HS_PLAY_XDIE2
    { HSPR_PLAY,  18,                     5, A_NoBlocking,           HS_PLAY_XDIE4       }, // HS_PLAY_XDIE3
    { HSPR_PLAY,  19,                     5, NULL,                   HS_PLAY_XDIE5       }, // HS_PLAY_XDIE4
    { HSPR_PLAY,  20,                     5, NULL,                   HS_PLAY_XDIE6       }, // HS_PLAY_XDIE5
    { HSPR_PLAY,  21,                     5, NULL,                   HS_PLAY_XDIE7       }, // HS_PLAY_XDIE6
    { HSPR_PLAY,  22,                     5, NULL,                   HS_PLAY_XDIE8       }, // HS_PLAY_XDIE7
    { HSPR_PLAY,  23,                     5, NULL,                   HS_PLAY_XDIE9       }, // HS_PLAY_XDIE8
    { HSPR_PLAY,  24,                    -1, NULL,                   HS_NULL             }, // HS_PLAY_XDIE9
    { HSPR_FDTH,  (0 | FF_FULLBRIGHT),    5, A_FlameSnd,             HS_PLAY_FDTH2       }, // HS_PLAY_FDTH1
    { HSPR_FDTH,  (1 | FF_FULLBRIGHT),    4, NULL,                   HS_PLAY_FDTH3       }, // HS_PLAY_FDTH2
    { HSPR_FDTH,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_PLAY_FDTH4       }, // HS_PLAY_FDTH3
    { HSPR_FDTH,  (3 | FF_FULLBRIGHT),    4, A_Scream,               HS_PLAY_FDTH5       }, // HS_PLAY_FDTH4
    { HSPR_FDTH,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_PLAY_FDTH6       }, // HS_PLAY_FDTH5
    { HSPR_FDTH,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_PLAY_FDTH7       }, // HS_PLAY_FDTH6
    { HSPR_FDTH,  (6 | FF_FULLBRIGHT),    5, A_FlameSnd,             HS_PLAY_FDTH8       }, // HS_PLAY_FDTH7
    { HSPR_FDTH,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_PLAY_FDTH9       }, // HS_PLAY_FDTH8
    { HSPR_FDTH,  (8 | FF_FULLBRIGHT),    5, NULL,                   HS_PLAY_FDTH10      }, // HS_PLAY_FDTH9
    { HSPR_FDTH,  (9 | FF_FULLBRIGHT),    4, NULL,                   HS_PLAY_FDTH11      }, // HS_PLAY_FDTH10
    { HSPR_FDTH, (10 | FF_FULLBRIGHT),    5, NULL,                   HS_PLAY_FDTH12      }, // HS_PLAY_FDTH11
    { HSPR_FDTH, (11 | FF_FULLBRIGHT),    4, NULL,                   HS_PLAY_FDTH13      }, // HS_PLAY_FDTH12
    { HSPR_FDTH, (12 | FF_FULLBRIGHT),    5, NULL,                   HS_PLAY_FDTH14      }, // HS_PLAY_FDTH13
    { HSPR_FDTH, (13 | FF_FULLBRIGHT),    4, NULL,                   HS_PLAY_FDTH15      }, // HS_PLAY_FDTH14
    { HSPR_FDTH, (14 | FF_FULLBRIGHT),    5, A_NoBlocking,           HS_PLAY_FDTH16      }, // HS_PLAY_FDTH15
    { HSPR_FDTH, (15 | FF_FULLBRIGHT),    4, NULL,                   HS_PLAY_FDTH17      }, // HS_PLAY_FDTH16
    { HSPR_FDTH, (16 | FF_FULLBRIGHT),    5, NULL,                   HS_PLAY_FDTH18      }, // HS_PLAY_FDTH17
    { HSPR_FDTH, (17 | FF_FULLBRIGHT),    4, NULL,                   HS_PLAY_FDTH19      }, // HS_PLAY_FDTH18
    { HSPR_ACLO,   4,                    35, A_CheckBurnGone,        HS_PLAY_FDTH19      }, // HS_PLAY_FDTH19
    { HSPR_ACLO,   4,                     8, NULL,                   HS_NULL             }, // HS_PLAY_FDTH20
    { HSPR_BSKL,   0,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL2     }, // HS_BLOODYSKULL1
    { HSPR_BSKL,   1,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL3     }, // HS_BLOODYSKULL2
    { HSPR_BSKL,   2,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL4     }, // HS_BLOODYSKULL3
    { HSPR_BSKL,   3,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL5     }, // HS_BLOODYSKULL4
    { HSPR_BSKL,   4,                     5, A_CheckSkullFloor,      HS_BLOODYSKULL1     }, // HS_BLOODYSKULL5
    { HSPR_BSKL,   5,                    16, A_CheckSkullDone,       HS_BLOODYSKULLX1    }, // HS_BLOODYSKULLX1
    { HSPR_BSKL,   5,                  1050, NULL,                   HS_NULL             }, // HS_BLOODYSKULLX2
    { HSPR_CHKN,   0,                    -1, NULL,                   HS_NULL             }, // HS_CHICPLAY
    { HSPR_CHKN,   0,                     3, NULL,                   HS_CHICPLAY_RUN2    }, // HS_CHICPLAY_RUN1
    { HSPR_CHKN,   1,                     3, NULL,                   HS_CHICPLAY_RUN3    }, // HS_CHICPLAY_RUN2
    { HSPR_CHKN,   0,                     3, NULL,                   HS_CHICPLAY_RUN4    }, // HS_CHICPLAY_RUN3
    { HSPR_CHKN,   1,                     3, NULL,                   HS_CHICPLAY_RUN1    }, // HS_CHICPLAY_RUN4
    { HSPR_CHKN,   2,                    12, NULL,                   HS_CHICPLAY         }, // HS_CHICPLAY_ATK1
    { HSPR_CHKN,   3,                     4, A_Feathers,             HS_CHICPLAY_PAIN2   }, // HS_CHICPLAY_PAIN
    { HSPR_CHKN,   2,                     4, A_Pain,                 HS_CHICPLAY         }, // HS_CHICPLAY_PAIN2
    { HSPR_CHKN,   0,                    10, A_ChicLook,             HS_CHICKEN_LOOK2    }, // HS_CHICKEN_LOOK1
    { HSPR_CHKN,   1,                    10, A_ChicLook,             HS_CHICKEN_LOOK1    }, // HS_CHICKEN_LOOK2
    { HSPR_CHKN,   0,                     3, A_ChicChase,            HS_CHICKEN_WALK2    }, // HS_CHICKEN_WALK1
    { HSPR_CHKN,   1,                     3, A_ChicChase,            HS_CHICKEN_WALK1    }, // HS_CHICKEN_WALK2
    { HSPR_CHKN,   3,                     5, A_Feathers,             HS_CHICKEN_PAIN2    }, // HS_CHICKEN_PAIN1
    { HSPR_CHKN,   2,                     5, A_ChicPain,             HS_CHICKEN_WALK1    }, // HS_CHICKEN_PAIN2
    { HSPR_CHKN,   0,                     8, A_FaceTarget,           HS_CHICKEN_ATK2     }, // HS_CHICKEN_ATK1
    { HSPR_CHKN,   2,                    10, A_ChicAttack,           HS_CHICKEN_WALK1    }, // HS_CHICKEN_ATK2
    { HSPR_CHKN,   4,                     6, A_Scream,               HS_CHICKEN_DIE2     }, // HS_CHICKEN_DIE1
    { HSPR_CHKN,   5,                     6, A_Feathers,             HS_CHICKEN_DIE3     }, // HS_CHICKEN_DIE2
    { HSPR_CHKN,   6,                     6, NULL,                   HS_CHICKEN_DIE4     }, // HS_CHICKEN_DIE3
    { HSPR_CHKN,   7,                     6, A_NoBlocking,           HS_CHICKEN_DIE5     }, // HS_CHICKEN_DIE4
    { HSPR_CHKN,   8,                     6, NULL,                   HS_CHICKEN_DIE6     }, // HS_CHICKEN_DIE5
    { HSPR_CHKN,   9,                     6, NULL,                   HS_CHICKEN_DIE7     }, // HS_CHICKEN_DIE6
    { HSPR_CHKN,  10,                     6, NULL,                   HS_CHICKEN_DIE8     }, // HS_CHICKEN_DIE7
    { HSPR_CHKN,  11,                    -1, NULL,                   HS_NULL             }, // HS_CHICKEN_DIE8
    { HSPR_CHKN,  12,                     3, NULL,                   HS_FEATHER2         }, // HS_FEATHER1
    { HSPR_CHKN,  13,                     3, NULL,                   HS_FEATHER3         }, // HS_FEATHER2
    { HSPR_CHKN,  14,                     3, NULL,                   HS_FEATHER4         }, // HS_FEATHER3
    { HSPR_CHKN,  15,                     3, NULL,                   HS_FEATHER5         }, // HS_FEATHER4
    { HSPR_CHKN,  16,                     3, NULL,                   HS_FEATHER6         }, // HS_FEATHER5
    { HSPR_CHKN,  15,                     3, NULL,                   HS_FEATHER7         }, // HS_FEATHER6
    { HSPR_CHKN,  14,                     3, NULL,                   HS_FEATHER8         }, // HS_FEATHER7
    { HSPR_CHKN,  13,                     3, NULL,                   HS_FEATHER1         }, // HS_FEATHER8
    { HSPR_CHKN,  13,                     6, NULL,                   HS_NULL             }, // HS_FEATHERX
    { HSPR_MUMM,   0,                    10, A_Look,                 HS_MUMMY_LOOK2      }, // HS_MUMMY_LOOK1
    { HSPR_MUMM,   1,                    10, A_Look,                 HS_MUMMY_LOOK1      }, // HS_MUMMY_LOOK2
    { HSPR_MUMM,   0,                     4, A_Chase,                HS_MUMMY_WALK2      }, // HS_MUMMY_WALK1
    { HSPR_MUMM,   1,                     4, A_Chase,                HS_MUMMY_WALK3      }, // HS_MUMMY_WALK2
    { HSPR_MUMM,   2,                     4, A_Chase,                HS_MUMMY_WALK4      }, // HS_MUMMY_WALK3
    { HSPR_MUMM,   3,                     4, A_Chase,                HS_MUMMY_WALK1      }, // HS_MUMMY_WALK4
    { HSPR_MUMM,   4,                     6, A_FaceTarget,           HS_MUMMY_ATK2       }, // HS_MUMMY_ATK1
    { HSPR_MUMM,   5,                     6, A_MummyAttack,          HS_MUMMY_ATK3       }, // HS_MUMMY_ATK2
    { HSPR_MUMM,   6,                     6, A_FaceTarget,           HS_MUMMY_WALK1      }, // HS_MUMMY_ATK3
    { HSPR_MUMM,  23,                     5, A_FaceTarget,           HS_MUMMYL_ATK2      }, // HS_MUMMYL_ATK1
    { HSPR_MUMM, (24 | FF_FULLBRIGHT),    5, A_FaceTarget,           HS_MUMMYL_ATK3      }, // HS_MUMMYL_ATK2
    { HSPR_MUMM,  23,                     5, A_FaceTarget,           HS_MUMMYL_ATK4      }, // HS_MUMMYL_ATK3
    { HSPR_MUMM, (24 | FF_FULLBRIGHT),    5, A_FaceTarget,           HS_MUMMYL_ATK5      }, // HS_MUMMYL_ATK4
    { HSPR_MUMM,  23,                     5, A_FaceTarget,           HS_MUMMYL_ATK6      }, // HS_MUMMYL_ATK5
    { HSPR_MUMM, (24 | FF_FULLBRIGHT),   15, A_MummyAttack2,         HS_MUMMY_WALK1      }, // HS_MUMMYL_ATK6
    { HSPR_MUMM,   7,                     4, NULL,                   HS_MUMMY_PAIN2      }, // HS_MUMMY_PAIN1
    { HSPR_MUMM,   7,                     4, A_Pain,                 HS_MUMMY_WALK1      }, // HS_MUMMY_PAIN2
    { HSPR_MUMM,   8,                     5, NULL,                   HS_MUMMY_DIE2       }, // HS_MUMMY_DIE1
    { HSPR_MUMM,   9,                     5, A_Scream,               HS_MUMMY_DIE3       }, // HS_MUMMY_DIE2
    { HSPR_MUMM,  10,                     5, A_MummySoul,            HS_MUMMY_DIE4       }, // HS_MUMMY_DIE3
    { HSPR_MUMM,  11,                     5, NULL,                   HS_MUMMY_DIE5       }, // HS_MUMMY_DIE4
    { HSPR_MUMM,  12,                     5, A_NoBlocking,           HS_MUMMY_DIE6       }, // HS_MUMMY_DIE5
    { HSPR_MUMM,  13,                     5, NULL,                   HS_MUMMY_DIE7       }, // HS_MUMMY_DIE6
    { HSPR_MUMM,  14,                     5, NULL,                   HS_MUMMY_DIE8       }, // HS_MUMMY_DIE7
    { HSPR_MUMM,  15,                    -1, NULL,                   HS_NULL             }, // HS_MUMMY_DIE8
    { HSPR_MUMM,  16,                     5, NULL,                   HS_MUMMY_SOUL2      }, // HS_MUMMY_SOUL1
    { HSPR_MUMM,  17,                     5, NULL,                   HS_MUMMY_SOUL3      }, // HS_MUMMY_SOUL2
    { HSPR_MUMM,  18,                     5, NULL,                   HS_MUMMY_SOUL4      }, // HS_MUMMY_SOUL3
    { HSPR_MUMM,  19,                     9, NULL,                   HS_MUMMY_SOUL5      }, // HS_MUMMY_SOUL4
    { HSPR_MUMM,  20,                     5, NULL,                   HS_MUMMY_SOUL6      }, // HS_MUMMY_SOUL5
    { HSPR_MUMM,  21,                     5, NULL,                   HS_MUMMY_SOUL7      }, // HS_MUMMY_SOUL6
    { HSPR_MUMM,  22,                     5, NULL,                   HS_NULL             }, // HS_MUMMY_SOUL7
    { HSPR_FX15,  (0 | FF_FULLBRIGHT),    5, A_ContMobjSound,        HS_MUMMYFX1_2       }, // HS_MUMMYFX1_1
    { HSPR_FX15,  (1 | FF_FULLBRIGHT),    5, A_MummyFX1Seek,         HS_MUMMYFX1_3       }, // HS_MUMMYFX1_2
    { HSPR_FX15,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_MUMMYFX1_4       }, // HS_MUMMYFX1_3
    { HSPR_FX15,  (1 | FF_FULLBRIGHT),    5, A_MummyFX1Seek,         HS_MUMMYFX1_1       }, // HS_MUMMYFX1_4
    { HSPR_FX15,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_MUMMYFXI1_2      }, // HS_MUMMYFXI1_1
    { HSPR_FX15,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_MUMMYFXI1_3      }, // HS_MUMMYFXI1_2
    { HSPR_FX15,  (5 | FF_FULLBRIGHT),    5, NULL,                   HS_MUMMYFXI1_4      }, // HS_MUMMYFXI1_3
    { HSPR_FX15,  (6 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_MUMMYFXI1_4
    { HSPR_BEAS,   0,                    10, A_Look,                 HS_BEAST_LOOK2      }, // HS_BEAST_LOOK1
    { HSPR_BEAS,   1,                    10, A_Look,                 HS_BEAST_LOOK1      }, // HS_BEAST_LOOK2
    { HSPR_BEAS,   0,                     3, A_Chase,                HS_BEAST_WALK2      }, // HS_BEAST_WALK1
    { HSPR_BEAS,   1,                     3, A_Chase,                HS_BEAST_WALK3      }, // HS_BEAST_WALK2
    { HSPR_BEAS,   2,                     3, A_Chase,                HS_BEAST_WALK4      }, // HS_BEAST_WALK3
    { HSPR_BEAS,   3,                     3, A_Chase,                HS_BEAST_WALK5      }, // HS_BEAST_WALK4
    { HSPR_BEAS,   4,                     3, A_Chase,                HS_BEAST_WALK6      }, // HS_BEAST_WALK5
    { HSPR_BEAS,   5,                     3, A_Chase,                HS_BEAST_WALK1      }, // HS_BEAST_WALK6
    { HSPR_BEAS,   7,                    10, A_FaceTarget,           HS_BEAST_ATK2       }, // HS_BEAST_ATK1
    { HSPR_BEAS,   8,                    10, A_BeastAttack,          HS_BEAST_WALK1      }, // HS_BEAST_ATK2
    { HSPR_BEAS,   6,                     3, NULL,                   HS_BEAST_PAIN2      }, // HS_BEAST_PAIN1
    { HSPR_BEAS,   6,                     3, A_Pain,                 HS_BEAST_WALK1      }, // HS_BEAST_PAIN2
    { HSPR_BEAS,  17,                     6, NULL,                   HS_BEAST_DIE2       }, // HS_BEAST_DIE1
    { HSPR_BEAS,  18,                     6, A_Scream,               HS_BEAST_DIE3       }, // HS_BEAST_DIE2
    { HSPR_BEAS,  19,                     6, NULL,                   HS_BEAST_DIE4       }, // HS_BEAST_DIE3
    { HSPR_BEAS,  20,                     6, NULL,                   HS_BEAST_DIE5       }, // HS_BEAST_DIE4
    { HSPR_BEAS,  21,                     6, NULL,                   HS_BEAST_DIE6       }, // HS_BEAST_DIE5
    { HSPR_BEAS,  22,                     6, A_NoBlocking,           HS_BEAST_DIE7       }, // HS_BEAST_DIE6
    { HSPR_BEAS,  23,                     6, NULL,                   HS_BEAST_DIE8       }, // HS_BEAST_DIE7
    { HSPR_BEAS,  24,                     6, NULL,                   HS_BEAST_DIE9       }, // HS_BEAST_DIE8
    { HSPR_BEAS,  25,                    -1, NULL,                   HS_NULL             }, // HS_BEAST_DIE9
    { HSPR_BEAS,   9,                     5, NULL,                   HS_BEAST_XDIE2      }, // HS_BEAST_XDIE1
    { HSPR_BEAS,  10,                     6, A_Scream,               HS_BEAST_XDIE3      }, // HS_BEAST_XDIE2
    { HSPR_BEAS,  11,                     5, NULL,                   HS_BEAST_XDIE4      }, // HS_BEAST_XDIE3
    { HSPR_BEAS,  12,                     6, NULL,                   HS_BEAST_XDIE5      }, // HS_BEAST_XDIE4
    { HSPR_BEAS,  13,                     5, NULL,                   HS_BEAST_XDIE6      }, // HS_BEAST_XDIE5
    { HSPR_BEAS,  14,                     6, A_NoBlocking,           HS_BEAST_XDIE7      }, // HS_BEAST_XDIE6
    { HSPR_BEAS,  15,                     5, NULL,                   HS_BEAST_XDIE8      }, // HS_BEAST_XDIE7
    { HSPR_BEAS,  16,                    -1, NULL,                   HS_NULL             }, // HS_BEAST_XDIE8
    { HSPR_FRB1,   0,                     2, A_BeastPuff,            HS_BEASTBALL2       }, // HS_BEASTBALL1
    { HSPR_FRB1,   0,                     2, A_BeastPuff,            HS_BEASTBALL3       }, // HS_BEASTBALL2
    { HSPR_FRB1,   1,                     2, A_BeastPuff,            HS_BEASTBALL4       }, // HS_BEASTBALL3
    { HSPR_FRB1,   1,                     2, A_BeastPuff,            HS_BEASTBALL5       }, // HS_BEASTBALL4
    { HSPR_FRB1,   2,                     2, A_BeastPuff,            HS_BEASTBALL6       }, // HS_BEASTBALL5
    { HSPR_FRB1,   2,                     2, A_BeastPuff,            HS_BEASTBALL1       }, // HS_BEASTBALL6
    { HSPR_FRB1,   3,                     4, NULL,                   HS_BEASTBALLX2      }, // HS_BEASTBALLX1
    { HSPR_FRB1,   4,                     4, NULL,                   HS_BEASTBALLX3      }, // HS_BEASTBALLX2
    { HSPR_FRB1,   5,                     4, NULL,                   HS_BEASTBALLX4      }, // HS_BEASTBALLX3
    { HSPR_FRB1,   6,                     4, NULL,                   HS_BEASTBALLX5      }, // HS_BEASTBALLX4
    { HSPR_FRB1,   7,                     4, NULL,                   HS_NULL             }, // HS_BEASTBALLX5
    { HSPR_FRB1,   0,                     4, NULL,                   HS_BURNBALL2        }, // HS_BURNBALL1
    { HSPR_FRB1,   1,                     4, NULL,                   HS_BURNBALL3        }, // HS_BURNBALL2
    { HSPR_FRB1,   2,                     4, NULL,                   HS_BURNBALL4        }, // HS_BURNBALL3
    { HSPR_FRB1,   3,                     4, NULL,                   HS_BURNBALL5        }, // HS_BURNBALL4
    { HSPR_FRB1,   4,                     4, NULL,                   HS_BURNBALL6        }, // HS_BURNBALL5
    { HSPR_FRB1,   5,                     4, NULL,                   HS_BURNBALL7        }, // HS_BURNBALL6
    { HSPR_FRB1,   6,                     4, NULL,                   HS_BURNBALL8        }, // HS_BURNBALL7
    { HSPR_FRB1,   7,                     4, NULL,                   HS_NULL             }, // HS_BURNBALL8
    { HSPR_FRB1,  (0 | FF_FULLBRIGHT),    4, NULL,                   HS_BURNBALLFB2      }, // HS_BURNBALLFB1
    { HSPR_FRB1,  (1 | FF_FULLBRIGHT),    4, NULL,                   HS_BURNBALLFB3      }, // HS_BURNBALLFB2
    { HSPR_FRB1,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_BURNBALLFB4      }, // HS_BURNBALLFB3
    { HSPR_FRB1,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_BURNBALLFB5      }, // HS_BURNBALLFB4
    { HSPR_FRB1,  (4 | FF_FULLBRIGHT),    4, NULL,                   HS_BURNBALLFB6      }, // HS_BURNBALLFB5
    { HSPR_FRB1,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_BURNBALLFB7      }, // HS_BURNBALLFB6
    { HSPR_FRB1,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_BURNBALLFB8      }, // HS_BURNBALLFB7
    { HSPR_FRB1,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_BURNBALLFB8
    { HSPR_FRB1,   3,                     4, NULL,                   HS_PUFFY2           }, // HS_PUFFY1
    { HSPR_FRB1,   4,                     4, NULL,                   HS_PUFFY3           }, // HS_PUFFY2
    { HSPR_FRB1,   5,                     4, NULL,                   HS_PUFFY4           }, // HS_PUFFY3
    { HSPR_FRB1,   6,                     4, NULL,                   HS_PUFFY5           }, // HS_PUFFY4
    { HSPR_FRB1,   7,                     4, NULL,                   HS_NULL             }, // HS_PUFFY5
    { HSPR_SNKE,   0,                    10, A_Look,                 HS_SNAKE_LOOK2      }, // HS_SNAKE_LOOK1
    { HSPR_SNKE,   1,                    10, A_Look,                 HS_SNAKE_LOOK1      }, // HS_SNAKE_LOOK2
    { HSPR_SNKE,   0,                     4, A_Chase,                HS_SNAKE_WALK2      }, // HS_SNAKE_WALK1
    { HSPR_SNKE,   1,                     4, A_Chase,                HS_SNAKE_WALK3      }, // HS_SNAKE_WALK2
    { HSPR_SNKE,   2,                     4, A_Chase,                HS_SNAKE_WALK4      }, // HS_SNAKE_WALK3
    { HSPR_SNKE,   3,                     4, A_Chase,                HS_SNAKE_WALK1      }, // HS_SNAKE_WALK4
    { HSPR_SNKE,   5,                     5, A_FaceTarget,           HS_SNAKE_ATK2       }, // HS_SNAKE_ATK1
    { HSPR_SNKE,   5,                     5, A_FaceTarget,           HS_SNAKE_ATK3       }, // HS_SNAKE_ATK2
    { HSPR_SNKE,   5,                     4, A_SnakeAttack,          HS_SNAKE_ATK4       }, // HS_SNAKE_ATK3
    { HSPR_SNKE,   5,                     4, A_SnakeAttack,          HS_SNAKE_ATK5       }, // HS_SNAKE_ATK4
    { HSPR_SNKE,   5,                     4, A_SnakeAttack,          HS_SNAKE_ATK6       }, // HS_SNAKE_ATK5
    { HSPR_SNKE,   5,                     5, A_FaceTarget,           HS_SNAKE_ATK7       }, // HS_SNAKE_ATK6
    { HSPR_SNKE,   5,                     5, A_FaceTarget,           HS_SNAKE_ATK8       }, // HS_SNAKE_ATK7
    { HSPR_SNKE,   5,                     5, A_FaceTarget,           HS_SNAKE_ATK9       }, // HS_SNAKE_ATK8
    { HSPR_SNKE,   5,                     4, A_SnakeAttack2,         HS_SNAKE_WALK1      }, // HS_SNAKE_ATK9
    { HSPR_SNKE,   4,                     3, NULL,                   HS_SNAKE_PAIN2      }, // HS_SNAKE_PAIN1
    { HSPR_SNKE,   4,                     3, A_Pain,                 HS_SNAKE_WALK1      }, // HS_SNAKE_PAIN2
    { HSPR_SNKE,   6,                     5, NULL,                   HS_SNAKE_DIE2       }, // HS_SNAKE_DIE1
    { HSPR_SNKE,   7,                     5, A_Scream,               HS_SNAKE_DIE3       }, // HS_SNAKE_DIE2
    { HSPR_SNKE,   8,                     5, NULL,                   HS_SNAKE_DIE4       }, // HS_SNAKE_DIE3
    { HSPR_SNKE,   9,                     5, NULL,                   HS_SNAKE_DIE5       }, // HS_SNAKE_DIE4
    { HSPR_SNKE,  10,                     5, NULL,                   HS_SNAKE_DIE6       }, // HS_SNAKE_DIE5
    { HSPR_SNKE,  11,                     5, NULL,                   HS_SNAKE_DIE7       }, // HS_SNAKE_DIE6
    { HSPR_SNKE,  12,                     5, A_NoBlocking,           HS_SNAKE_DIE8       }, // HS_SNAKE_DIE7
    { HSPR_SNKE,  13,                     5, NULL,                   HS_SNAKE_DIE9       }, // HS_SNAKE_DIE8
    { HSPR_SNKE,  14,                     5, NULL,                   HS_SNAKE_DIE10      }, // HS_SNAKE_DIE9
    { HSPR_SNKE,  15,                    -1, NULL,                   HS_NULL             }, // HS_SNAKE_DIE10
    { HSPR_SNFX,  (0 | FF_FULLBRIGHT),    5, NULL,                   HS_SNAKEPRO_A2      }, // HS_SNAKEPRO_A1
    { HSPR_SNFX,  (1 | FF_FULLBRIGHT),    5, NULL,                   HS_SNAKEPRO_A3      }, // HS_SNAKEPRO_A2
    { HSPR_SNFX,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_SNAKEPRO_A4      }, // HS_SNAKEPRO_A3
    { HSPR_SNFX,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_SNAKEPRO_A1      }, // HS_SNAKEPRO_A4
    { HSPR_SNFX,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_SNAKEPRO_AX2     }, // HS_SNAKEPRO_AX1
    { HSPR_SNFX,  (5 | FF_FULLBRIGHT),    5, NULL,                   HS_SNAKEPRO_AX3     }, // HS_SNAKEPRO_AX2
    { HSPR_SNFX,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_SNAKEPRO_AX4     }, // HS_SNAKEPRO_AX3
    { HSPR_SNFX,  (7 | FF_FULLBRIGHT),    3, NULL,                   HS_SNAKEPRO_AX5     }, // HS_SNAKEPRO_AX4
    { HSPR_SNFX,  (8 | FF_FULLBRIGHT),    3, NULL,                   HS_NULL             }, // HS_SNAKEPRO_AX5
    { HSPR_SNFX,  (9 | FF_FULLBRIGHT),    6, NULL,                   HS_SNAKEPRO_B2      }, // HS_SNAKEPRO_B1
    { HSPR_SNFX, (10 | FF_FULLBRIGHT),    6, NULL,                   HS_SNAKEPRO_B1      }, // HS_SNAKEPRO_B2
    { HSPR_SNFX, (11 | FF_FULLBRIGHT),    5, NULL,                   HS_SNAKEPRO_BX2     }, // HS_SNAKEPRO_BX1
    { HSPR_SNFX, (12 | FF_FULLBRIGHT),    5, NULL,                   HS_SNAKEPRO_BX3     }, // HS_SNAKEPRO_BX2
    { HSPR_SNFX, (13 | FF_FULLBRIGHT),    4, NULL,                   HS_SNAKEPRO_BX4     }, // HS_SNAKEPRO_BX3
    { HSPR_SNFX, (14 | FF_FULLBRIGHT),    3, NULL,                   HS_NULL             }, // HS_SNAKEPRO_BX4
    { HSPR_HEAD,   0,                    10, A_Look,                 HS_HEAD_LOOK        }, // HS_HEAD_LOOK
    { HSPR_HEAD,   0,                     4, A_Chase,                HS_HEAD_FLOAT       }, // HS_HEAD_FLOAT
    { HSPR_HEAD,   0,                     5, A_FaceTarget,           HS_HEAD_ATK2        }, // HS_HEAD_ATK1
    { HSPR_HEAD,   1,                    20, A_HeadAttack2,          HS_HEAD_FLOAT       }, // HS_HEAD_ATK2
    { HSPR_HEAD,   0,                     4, NULL,                   HS_HEAD_PAIN2       }, // HS_HEAD_PAIN1
    { HSPR_HEAD,   0,                     4, A_Pain,                 HS_HEAD_FLOAT       }, // HS_HEAD_PAIN2
    { HSPR_HEAD,   2,                     7, NULL,                   HS_HEAD_DIE2        }, // HS_HEAD_DIE1
    { HSPR_HEAD,   3,                     7, A_Scream,               HS_HEAD_DIE3        }, // HS_HEAD_DIE2
    { HSPR_HEAD,   4,                     7, NULL,                   HS_HEAD_DIE4        }, // HS_HEAD_DIE3
    { HSPR_HEAD,   5,                     7, NULL,                   HS_HEAD_DIE5        }, // HS_HEAD_DIE4
    { HSPR_HEAD,   6,                     7, A_NoBlocking,           HS_HEAD_DIE6        }, // HS_HEAD_DIE5
    { HSPR_HEAD,   7,                     7, NULL,                   HS_HEAD_DIE7        }, // HS_HEAD_DIE6
    { HSPR_HEAD,   8,                    -1, A_BossDeath2,           HS_NULL             }, // HS_HEAD_DIE7
    { HSPR_FX05,   0,                     6, NULL,                   HS_HEADFX1_2        }, // HS_HEADFX1_1
    { HSPR_FX05,   1,                     6, NULL,                   HS_HEADFX1_3        }, // HS_HEADFX1_2
    { HSPR_FX05,   2,                     6, NULL,                   HS_HEADFX1_1        }, // HS_HEADFX1_3
    { HSPR_FX05,   3,                     5, A_HeadIceImpact,        HS_HEADFXI1_2       }, // HS_HEADFXI1_1
    { HSPR_FX05,   4,                     5, NULL,                   HS_HEADFXI1_3       }, // HS_HEADFXI1_2
    { HSPR_FX05,   5,                     5, NULL,                   HS_HEADFXI1_4       }, // HS_HEADFXI1_3
    { HSPR_FX05,   6,                     5, NULL,                   HS_NULL             }, // HS_HEADFXI1_4
    { HSPR_FX05,   7,                     6, NULL,                   HS_HEADFX2_2        }, // HS_HEADFX2_1
    { HSPR_FX05,   8,                     6, NULL,                   HS_HEADFX2_3        }, // HS_HEADFX2_2
    { HSPR_FX05,   9,                     6, NULL,                   HS_HEADFX2_1        }, // HS_HEADFX2_3
    { HSPR_FX05,   3,                     5, NULL,                   HS_HEADFXI2_2       }, // HS_HEADFXI2_1
    { HSPR_FX05,   4,                     5, NULL,                   HS_HEADFXI2_3       }, // HS_HEADFXI2_2
    { HSPR_FX05,   5,                     5, NULL,                   HS_HEADFXI2_4       }, // HS_HEADFXI2_3
    { HSPR_FX05,   6,                     5, NULL,                   HS_NULL             }, // HS_HEADFXI2_4
    { HSPR_FX06,   0,                     4, A_HeadFireGrow,         HS_HEADFX3_2        }, // HS_HEADFX3_1
    { HSPR_FX06,   1,                     4, A_HeadFireGrow,         HS_HEADFX3_3        }, // HS_HEADFX3_2
    { HSPR_FX06,   2,                     4, A_HeadFireGrow,         HS_HEADFX3_1        }, // HS_HEADFX3_3
    { HSPR_FX06,   0,                     5, NULL,                   HS_HEADFX3_5        }, // HS_HEADFX3_4
    { HSPR_FX06,   1,                     5, NULL,                   HS_HEADFX3_6        }, // HS_HEADFX3_5
    { HSPR_FX06,   2,                     5, NULL,                   HS_HEADFX3_4        }, // HS_HEADFX3_6
    { HSPR_FX06,   3,                     5, NULL,                   HS_HEADFXI3_2       }, // HS_HEADFXI3_1
    { HSPR_FX06,   4,                     5, NULL,                   HS_HEADFXI3_3       }, // HS_HEADFXI3_2
    { HSPR_FX06,   5,                     5, NULL,                   HS_HEADFXI3_4       }, // HS_HEADFXI3_3
    { HSPR_FX06,   6,                     5, NULL,                   HS_NULL             }, // HS_HEADFXI3_4
    { HSPR_FX07,   3,                     3, NULL,                   HS_HEADFX4_2        }, // HS_HEADFX4_1
    { HSPR_FX07,   4,                     3, NULL,                   HS_HEADFX4_3        }, // HS_HEADFX4_2
    { HSPR_FX07,   5,                     3, NULL,                   HS_HEADFX4_4        }, // HS_HEADFX4_3
    { HSPR_FX07,   6,                     3, NULL,                   HS_HEADFX4_5        }, // HS_HEADFX4_4
    { HSPR_FX07,   0,                     3, A_WhirlwindSeek,        HS_HEADFX4_6        }, // HS_HEADFX4_5
    { HSPR_FX07,   1,                     3, A_WhirlwindSeek,        HS_HEADFX4_7        }, // HS_HEADFX4_6
    { HSPR_FX07,   2,                     3, A_WhirlwindSeek,        HS_HEADFX4_5        }, // HS_HEADFX4_7
    { HSPR_FX07,   6,                     4, NULL,                   HS_HEADFXI4_2       }, // HS_HEADFXI4_1
    { HSPR_FX07,   5,                     4, NULL,                   HS_HEADFXI4_3       }, // HS_HEADFXI4_2
    { HSPR_FX07,   4,                     4, NULL,                   HS_HEADFXI4_4       }, // HS_HEADFXI4_3
    { HSPR_FX07,   3,                     4, NULL,                   HS_NULL             }, // HS_HEADFXI4_4
    { HSPR_CLNK,   0,                    10, A_Look,                 HS_CLINK_LOOK2      }, // HS_CLINK_LOOK1
    { HSPR_CLNK,   1,                    10, A_Look,                 HS_CLINK_LOOK1      }, // HS_CLINK_LOOK2
    { HSPR_CLNK,   0,                     3, A_Chase,                HS_CLINK_WALK2      }, // HS_CLINK_WALK1
    { HSPR_CLNK,   1,                     3, A_Chase,                HS_CLINK_WALK3      }, // HS_CLINK_WALK2
    { HSPR_CLNK,   2,                     3, A_Chase,                HS_CLINK_WALK4      }, // HS_CLINK_WALK3
    { HSPR_CLNK,   3,                     3, A_Chase,                HS_CLINK_WALK1      }, // HS_CLINK_WALK4
    { HSPR_CLNK,   4,                     5, A_FaceTarget,           HS_CLINK_ATK2       }, // HS_CLINK_ATK1
    { HSPR_CLNK,   5,                     4, A_FaceTarget,           HS_CLINK_ATK3       }, // HS_CLINK_ATK2
    { HSPR_CLNK,   6,                     7, A_ClinkAttack,          HS_CLINK_WALK1      }, // HS_CLINK_ATK3
    { HSPR_CLNK,   7,                     3, NULL,                   HS_CLINK_PAIN2      }, // HS_CLINK_PAIN1
    { HSPR_CLNK,   7,                     3, A_Pain,                 HS_CLINK_WALK1      }, // HS_CLINK_PAIN2
    { HSPR_CLNK,   8,                     6, NULL,                   HS_CLINK_DIE2       }, // HS_CLINK_DIE1
    { HSPR_CLNK,   9,                     6, NULL,                   HS_CLINK_DIE3       }, // HS_CLINK_DIE2
    { HSPR_CLNK,  10,                     5, A_Scream,               HS_CLINK_DIE4       }, // HS_CLINK_DIE3
    { HSPR_CLNK,  11,                     5, A_NoBlocking,           HS_CLINK_DIE5       }, // HS_CLINK_DIE4
    { HSPR_CLNK,  12,                     5, NULL,                   HS_CLINK_DIE6       }, // HS_CLINK_DIE5
    { HSPR_CLNK,  13,                     5, NULL,                   HS_CLINK_DIE7       }, // HS_CLINK_DIE6
    { HSPR_CLNK,  14,                    -1, NULL,                   HS_NULL             }, // HS_CLINK_DIE7
    { HSPR_WZRD,   0,                    10, A_Look,                 HS_WIZARD_LOOK2     }, // HS_WIZARD_LOOK1
    { HSPR_WZRD,   1,                    10, A_Look,                 HS_WIZARD_LOOK1     }, // HS_WIZARD_LOOK2
    { HSPR_WZRD,   0,                     3, A_Chase,                HS_WIZARD_WALK2     }, // HS_WIZARD_WALK1
    { HSPR_WZRD,   0,                     4, A_Chase,                HS_WIZARD_WALK3     }, // HS_WIZARD_WALK2
    { HSPR_WZRD,   0,                     3, A_Chase,                HS_WIZARD_WALK4     }, // HS_WIZARD_WALK3
    { HSPR_WZRD,   0,                     4, A_Chase,                HS_WIZARD_WALK5     }, // HS_WIZARD_WALK4
    { HSPR_WZRD,   1,                     3, A_Chase,                HS_WIZARD_WALK6     }, // HS_WIZARD_WALK5
    { HSPR_WZRD,   1,                     4, A_Chase,                HS_WIZARD_WALK7     }, // HS_WIZARD_WALK6
    { HSPR_WZRD,   1,                     3, A_Chase,                HS_WIZARD_WALK8     }, // HS_WIZARD_WALK7
    { HSPR_WZRD,   1,                     4, A_Chase,                HS_WIZARD_WALK1     }, // HS_WIZARD_WALK8
    { HSPR_WZRD,   2,                     4, A_WizAtk1,              HS_WIZARD_ATK2      }, // HS_WIZARD_ATK1
    { HSPR_WZRD,   2,                     4, A_WizAtk2,              HS_WIZARD_ATK3      }, // HS_WIZARD_ATK2
    { HSPR_WZRD,   2,                     4, A_WizAtk1,              HS_WIZARD_ATK4      }, // HS_WIZARD_ATK3
    { HSPR_WZRD,   2,                     4, A_WizAtk2,              HS_WIZARD_ATK5      }, // HS_WIZARD_ATK4
    { HSPR_WZRD,   2,                     4, A_WizAtk1,              HS_WIZARD_ATK6      }, // HS_WIZARD_ATK5
    { HSPR_WZRD,   2,                     4, A_WizAtk2,              HS_WIZARD_ATK7      }, // HS_WIZARD_ATK6
    { HSPR_WZRD,   2,                     4, A_WizAtk1,              HS_WIZARD_ATK8      }, // HS_WIZARD_ATK7
    { HSPR_WZRD,   2,                     4, A_WizAtk2,              HS_WIZARD_ATK9      }, // HS_WIZARD_ATK8
    { HSPR_WZRD,   3,                    12, A_WizAtk3,              HS_WIZARD_WALK1     }, // HS_WIZARD_ATK9
    { HSPR_WZRD,   4,                     3, A_GhostOff,             HS_WIZARD_PAIN2     }, // HS_WIZARD_PAIN1
    { HSPR_WZRD,   4,                     3, A_Pain,                 HS_WIZARD_WALK1     }, // HS_WIZARD_PAIN2
    { HSPR_WZRD,   5,                     6, A_GhostOff,             HS_WIZARD_DIE2      }, // HS_WIZARD_DIE1
    { HSPR_WZRD,   6,                     6, A_Scream,               HS_WIZARD_DIE3      }, // HS_WIZARD_DIE2
    { HSPR_WZRD,   7,                     6, NULL,                   HS_WIZARD_DIE4      }, // HS_WIZARD_DIE3
    { HSPR_WZRD,   8,                     6, NULL,                   HS_WIZARD_DIE5      }, // HS_WIZARD_DIE4
    { HSPR_WZRD,   9,                     6, A_NoBlocking,           HS_WIZARD_DIE6      }, // HS_WIZARD_DIE5
    { HSPR_WZRD,  10,                     6, NULL,                   HS_WIZARD_DIE7      }, // HS_WIZARD_DIE6
    { HSPR_WZRD,  11,                     6, NULL,                   HS_WIZARD_DIE8      }, // HS_WIZARD_DIE7
    { HSPR_WZRD,  12,                    -1, NULL,                   HS_NULL             }, // HS_WIZARD_DIE8
    { HSPR_FX11,  (0 | FF_FULLBRIGHT),    6, NULL,                   HS_WIZFX1_2         }, // HS_WIZFX1_1
    { HSPR_FX11,  (1 | FF_FULLBRIGHT),    6, NULL,                   HS_WIZFX1_1         }, // HS_WIZFX1_2
    { HSPR_FX11,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_WIZFXI1_2        }, // HS_WIZFXI1_1
    { HSPR_FX11,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_WIZFXI1_3        }, // HS_WIZFXI1_2
    { HSPR_FX11,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_WIZFXI1_4        }, // HS_WIZFXI1_3
    { HSPR_FX11,  (5 | FF_FULLBRIGHT),    5, NULL,                   HS_WIZFXI1_5        }, // HS_WIZFXI1_4
    { HSPR_FX11,  (6 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_WIZFXI1_5
    { HSPR_IMPX,   0,                    10, A_Look,                 HS_IMP_LOOK2        }, // HS_IMP_LOOK1
    { HSPR_IMPX,   1,                    10, A_Look,                 HS_IMP_LOOK3        }, // HS_IMP_LOOK2
    { HSPR_IMPX,   2,                    10, A_Look,                 HS_IMP_LOOK4        }, // HS_IMP_LOOK3
    { HSPR_IMPX,   1,                    10, A_Look,                 HS_IMP_LOOK1        }, // HS_IMP_LOOK4
    { HSPR_IMPX,   0,                     3, A_Chase,                HS_IMP_FLY2         }, // HS_IMP_FLY1
    { HSPR_IMPX,   0,                     3, A_Chase,                HS_IMP_FLY3         }, // HS_IMP_FLY2
    { HSPR_IMPX,   1,                     3, A_Chase,                HS_IMP_FLY4         }, // HS_IMP_FLY3
    { HSPR_IMPX,   1,                     3, A_Chase,                HS_IMP_FLY5         }, // HS_IMP_FLY4
    { HSPR_IMPX,   2,                     3, A_Chase,                HS_IMP_FLY6         }, // HS_IMP_FLY5
    { HSPR_IMPX,   2,                     3, A_Chase,                HS_IMP_FLY7         }, // HS_IMP_FLY6
    { HSPR_IMPX,   1,                     3, A_Chase,                HS_IMP_FLY8         }, // HS_IMP_FLY7
    { HSPR_IMPX,   1,                     3, A_Chase,                HS_IMP_FLY1         }, // HS_IMP_FLY8
    { HSPR_IMPX,   3,                     6, A_FaceTarget,           HS_IMP_MEATK2       }, // HS_IMP_MEATK1
    { HSPR_IMPX,   4,                     6, A_FaceTarget,           HS_IMP_MEATK3       }, // HS_IMP_MEATK2
    { HSPR_IMPX,   5,                     6, A_ImpMeAttack,          HS_IMP_FLY1         }, // HS_IMP_MEATK3
    { HSPR_IMPX,   0,                    10, A_FaceTarget,           HS_IMP_MSATK1_2     }, // HS_IMP_MSATK1_1
    { HSPR_IMPX,   1,                     6, A_ImpMsAttack,          HS_IMP_MSATK1_3     }, // HS_IMP_MSATK1_2
    { HSPR_IMPX,   2,                     6, NULL,                   HS_IMP_MSATK1_4     }, // HS_IMP_MSATK1_3
    { HSPR_IMPX,   1,                     6, NULL,                   HS_IMP_MSATK1_5     }, // HS_IMP_MSATK1_4
    { HSPR_IMPX,   0,                     6, NULL,                   HS_IMP_MSATK1_6     }, // HS_IMP_MSATK1_5
    { HSPR_IMPX,   1,                     6, NULL,                   HS_IMP_MSATK1_3     }, // HS_IMP_MSATK1_6
    { HSPR_IMPX,   3,                     6, A_FaceTarget,           HS_IMP_MSATK2_2     }, // HS_IMP_MSATK2_1
    { HSPR_IMPX,   4,                     6, A_FaceTarget,           HS_IMP_MSATK2_3     }, // HS_IMP_MSATK2_2
    { HSPR_IMPX,   5,                     6, A_ImpMsAttack2,         HS_IMP_FLY1         }, // HS_IMP_MSATK2_3
    { HSPR_IMPX,   6,                     3, NULL,                   HS_IMP_PAIN2        }, // HS_IMP_PAIN1
    { HSPR_IMPX,   6,                     3, A_Pain,                 HS_IMP_FLY1         }, // HS_IMP_PAIN2
    { HSPR_IMPX,   6,                     4, A_ImpDeath,             HS_IMP_DIE2         }, // HS_IMP_DIE1
    { HSPR_IMPX,   7,                     5, NULL,                   HS_IMP_DIE2         }, // HS_IMP_DIE2
    { HSPR_IMPX,  18,                     5, A_ImpXDeath1,           HS_IMP_XDIE2        }, // HS_IMP_XDIE1
    { HSPR_IMPX,  19,                     5, NULL,                   HS_IMP_XDIE3        }, // HS_IMP_XDIE2
    { HSPR_IMPX,  20,                     5, NULL,                   HS_IMP_XDIE4        }, // HS_IMP_XDIE3
    { HSPR_IMPX,  21,                     5, A_ImpXDeath2,           HS_IMP_XDIE5        }, // HS_IMP_XDIE4
    { HSPR_IMPX,  22,                     5, NULL,                   HS_IMP_XDIE5        }, // HS_IMP_XDIE5
    { HSPR_IMPX,   8,                     7, A_ImpExplode,           HS_IMP_CRASH2       }, // HS_IMP_CRASH1
    { HSPR_IMPX,   9,                     7, A_Scream,               HS_IMP_CRASH3       }, // HS_IMP_CRASH2
    { HSPR_IMPX,  10,                     7, NULL,                   HS_IMP_CRASH4       }, // HS_IMP_CRASH3
    { HSPR_IMPX,  11,                    -1, NULL,                   HS_NULL             }, // HS_IMP_CRASH4
    { HSPR_IMPX,  23,                     7, NULL,                   HS_IMP_XCRASH2      }, // HS_IMP_XCRASH1
    { HSPR_IMPX,  24,                     7, NULL,                   HS_IMP_XCRASH3      }, // HS_IMP_XCRASH2
    { HSPR_IMPX,  25,                    -1, NULL,                   HS_NULL             }, // HS_IMP_XCRASH3
    { HSPR_IMPX,  12,                     5, NULL,                   HS_IMP_CHUNKA2      }, // HS_IMP_CHUNKA1
    { HSPR_IMPX,  13,                    -1, NULL,                   HS_IMP_CHUNKA3      }, // HS_IMP_CHUNKA2
    { HSPR_IMPX,  14,                    -1, NULL,                   HS_NULL             }, // HS_IMP_CHUNKA3
    { HSPR_IMPX,  15,                     5, NULL,                   HS_IMP_CHUNKB2      }, // HS_IMP_CHUNKB1
    { HSPR_IMPX,  16,                    -1, NULL,                   HS_IMP_CHUNKB3      }, // HS_IMP_CHUNKB2
    { HSPR_IMPX,  17,                    -1, NULL,                   HS_NULL             }, // HS_IMP_CHUNKB3
    { HSPR_FX10,  (0 | FF_FULLBRIGHT),    6, NULL,                   HS_IMPFX2           }, // HS_IMPFX1
    { HSPR_FX10,  (1 | FF_FULLBRIGHT),    6, NULL,                   HS_IMPFX3           }, // HS_IMPFX2
    { HSPR_FX10,  (2 | FF_FULLBRIGHT),    6, NULL,                   HS_IMPFX1           }, // HS_IMPFX3
    { HSPR_FX10,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_IMPFXI2          }, // HS_IMPFXI1
    { HSPR_FX10,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_IMPFXI3          }, // HS_IMPFXI2
    { HSPR_FX10,  (5 | FF_FULLBRIGHT),    5, NULL,                   HS_IMPFXI4          }, // HS_IMPFXI3
    { HSPR_FX10,  (6 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_IMPFXI4
    { HSPR_KNIG,   0,                    10, A_Look,                 HS_KNIGHT_STND2     }, // HS_KNIGHT_STND1
    { HSPR_KNIG,   1,                    10, A_Look,                 HS_KNIGHT_STND1     }, // HS_KNIGHT_STND2
    { HSPR_KNIG,   0,                     4, A_Chase,                HS_KNIGHT_WALK2     }, // HS_KNIGHT_WALK1
    { HSPR_KNIG,   1,                     4, A_Chase,                HS_KNIGHT_WALK3     }, // HS_KNIGHT_WALK2
    { HSPR_KNIG,   2,                     4, A_Chase,                HS_KNIGHT_WALK4     }, // HS_KNIGHT_WALK3
    { HSPR_KNIG,   3,                     4, A_Chase,                HS_KNIGHT_WALK1     }, // HS_KNIGHT_WALK4
    { HSPR_KNIG,   4,                    10, A_FaceTarget,           HS_KNIGHT_ATK2      }, // HS_KNIGHT_ATK1
    { HSPR_KNIG,   5,                     8, A_FaceTarget,           HS_KNIGHT_ATK3      }, // HS_KNIGHT_ATK2
    { HSPR_KNIG,   6,                     8, A_KnightAttack,         HS_KNIGHT_ATK4      }, // HS_KNIGHT_ATK3
    { HSPR_KNIG,   4,                    10, A_FaceTarget,           HS_KNIGHT_ATK5      }, // HS_KNIGHT_ATK4
    { HSPR_KNIG,   5,                     8, A_FaceTarget,           HS_KNIGHT_ATK6      }, // HS_KNIGHT_ATK5
    { HSPR_KNIG,   6,                     8, A_KnightAttack,         HS_KNIGHT_WALK1     }, // HS_KNIGHT_ATK6
    { HSPR_KNIG,   7,                     3, NULL,                   HS_KNIGHT_PAIN2     }, // HS_KNIGHT_PAIN1
    { HSPR_KNIG,   7,                     3, A_Pain,                 HS_KNIGHT_WALK1     }, // HS_KNIGHT_PAIN2
    { HSPR_KNIG,   8,                     6, NULL,                   HS_KNIGHT_DIE2      }, // HS_KNIGHT_DIE1
    { HSPR_KNIG,   9,                     6, A_Scream,               HS_KNIGHT_DIE3      }, // HS_KNIGHT_DIE2
    { HSPR_KNIG,  10,                     6, NULL,                   HS_KNIGHT_DIE4      }, // HS_KNIGHT_DIE3
    { HSPR_KNIG,  11,                     6, A_NoBlocking,           HS_KNIGHT_DIE5      }, // HS_KNIGHT_DIE4
    { HSPR_KNIG,  12,                     6, NULL,                   HS_KNIGHT_DIE6      }, // HS_KNIGHT_DIE5
    { HSPR_KNIG,  13,                     6, NULL,                   HS_KNIGHT_DIE7      }, // HS_KNIGHT_DIE6
    { HSPR_KNIG,  14,                    -1, NULL,                   HS_NULL             }, // HS_KNIGHT_DIE7
    { HSPR_SPAX,  (0 | FF_FULLBRIGHT),    3, A_ContMobjSound,        HS_SPINAXE2         }, // HS_SPINAXE1
    { HSPR_SPAX,  (1 | FF_FULLBRIGHT),    3, NULL,                   HS_SPINAXE3         }, // HS_SPINAXE2
    { HSPR_SPAX,  (2 | FF_FULLBRIGHT),    3, NULL,                   HS_SPINAXE1         }, // HS_SPINAXE3
    { HSPR_SPAX,  (3 | FF_FULLBRIGHT),    6, NULL,                   HS_SPINAXEX2        }, // HS_SPINAXEX1
    { HSPR_SPAX,  (4 | FF_FULLBRIGHT),    6, NULL,                   HS_SPINAXEX3        }, // HS_SPINAXEX2
    { HSPR_SPAX,  (5 | FF_FULLBRIGHT),    6, NULL,                   HS_NULL             }, // HS_SPINAXEX3
    { HSPR_RAXE,  (0 | FF_FULLBRIGHT),    5, A_DripBlood,            HS_REDAXE2          }, // HS_REDAXE1
    { HSPR_RAXE,  (1 | FF_FULLBRIGHT),    5, A_DripBlood,            HS_REDAXE1          }, // HS_REDAXE2
    { HSPR_RAXE,  (2 | FF_FULLBRIGHT),    6, NULL,                   HS_REDAXEX2         }, // HS_REDAXEX1
    { HSPR_RAXE,  (3 | FF_FULLBRIGHT),    6, NULL,                   HS_REDAXEX3         }, // HS_REDAXEX2
    { HSPR_RAXE,  (4 | FF_FULLBRIGHT),    6, NULL,                   HS_NULL             }, // HS_REDAXEX3
    { HSPR_SRCR,   0,                    10, A_Look,                 HS_SRCR1_LOOK2      }, // HS_SRCR1_LOOK1
    { HSPR_SRCR,   1,                    10, A_Look,                 HS_SRCR1_LOOK1      }, // HS_SRCR1_LOOK2
    { HSPR_SRCR,   0,                     5, A_Sor1Chase,            HS_SRCR1_WALK2      }, // HS_SRCR1_WALK1
    { HSPR_SRCR,   1,                     5, A_Sor1Chase,            HS_SRCR1_WALK3      }, // HS_SRCR1_WALK2
    { HSPR_SRCR,   2,                     5, A_Sor1Chase,            HS_SRCR1_WALK4      }, // HS_SRCR1_WALK3
    { HSPR_SRCR,   3,                     5, A_Sor1Chase,            HS_SRCR1_WALK1      }, // HS_SRCR1_WALK4
    { HSPR_SRCR,  16,                     6, A_Sor1Pain,             HS_SRCR1_WALK1      }, // HS_SRCR1_PAIN1
    { HSPR_SRCR,  16,                     7, A_FaceTarget,           HS_SRCR1_ATK2       }, // HS_SRCR1_ATK1
    { HSPR_SRCR,  17,                     6, A_FaceTarget,           HS_SRCR1_ATK3       }, // HS_SRCR1_ATK2
    { HSPR_SRCR,  18,                    10, A_Srcr1Attack,          HS_SRCR1_WALK1      }, // HS_SRCR1_ATK3
    { HSPR_SRCR,  18,                    10, A_FaceTarget,           HS_SRCR1_ATK5       }, // HS_SRCR1_ATK4
    { HSPR_SRCR,  16,                     7, A_FaceTarget,           HS_SRCR1_ATK6       }, // HS_SRCR1_ATK5
    { HSPR_SRCR,  17,                     6, A_FaceTarget,           HS_SRCR1_ATK7       }, // HS_SRCR1_ATK6
    { HSPR_SRCR,  18,                    10, A_Srcr1Attack,          HS_SRCR1_WALK1      }, // HS_SRCR1_ATK7
    { HSPR_SRCR,   4,                     7, NULL,                   HS_SRCR1_DIE2       }, // HS_SRCR1_DIE1
    { HSPR_SRCR,   5,                     7, A_Scream,               HS_SRCR1_DIE3       }, // HS_SRCR1_DIE2
    { HSPR_SRCR,   6,                     7, NULL,                   HS_SRCR1_DIE4       }, // HS_SRCR1_DIE3
    { HSPR_SRCR,   7,                     6, NULL,                   HS_SRCR1_DIE5       }, // HS_SRCR1_DIE4
    { HSPR_SRCR,   8,                     6, NULL,                   HS_SRCR1_DIE6       }, // HS_SRCR1_DIE5
    { HSPR_SRCR,   9,                     6, NULL,                   HS_SRCR1_DIE7       }, // HS_SRCR1_DIE6
    { HSPR_SRCR,  10,                     6, NULL,                   HS_SRCR1_DIE8       }, // HS_SRCR1_DIE7
    { HSPR_SRCR,  11,                    25, A_SorZap,               HS_SRCR1_DIE9       }, // HS_SRCR1_DIE8
    { HSPR_SRCR,  12,                     5, NULL,                   HS_SRCR1_DIE10      }, // HS_SRCR1_DIE9
    { HSPR_SRCR,  13,                     5, NULL,                   HS_SRCR1_DIE11      }, // HS_SRCR1_DIE10
    { HSPR_SRCR,  14,                     4, NULL,                   HS_SRCR1_DIE12      }, // HS_SRCR1_DIE11
    { HSPR_SRCR,  11,                    20, A_SorZap,               HS_SRCR1_DIE13      }, // HS_SRCR1_DIE12
    { HSPR_SRCR,  12,                     5, NULL,                   HS_SRCR1_DIE14      }, // HS_SRCR1_DIE13
    { HSPR_SRCR,  13,                     5, NULL,                   HS_SRCR1_DIE15      }, // HS_SRCR1_DIE14
    { HSPR_SRCR,  14,                     4, NULL,                   HS_SRCR1_DIE16      }, // HS_SRCR1_DIE15
    { HSPR_SRCR,  11,                    12, NULL,                   HS_SRCR1_DIE17      }, // HS_SRCR1_DIE16
    { HSPR_SRCR,  15,                    -1, A_SorcererRise,         HS_NULL             }, // HS_SRCR1_DIE17
    { HSPR_FX14,  (0 | FF_FULLBRIGHT),    6, NULL,                   HS_SRCRFX1_2        }, // HS_SRCRFX1_1
    { HSPR_FX14,  (1 | FF_FULLBRIGHT),    6, NULL,                   HS_SRCRFX1_3        }, // HS_SRCRFX1_2
    { HSPR_FX14,  (2 | FF_FULLBRIGHT),    6, NULL,                   HS_SRCRFX1_1        }, // HS_SRCRFX1_3
    { HSPR_FX14,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_SRCRFXI1_2       }, // HS_SRCRFXI1_1
    { HSPR_FX14,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_SRCRFXI1_3       }, // HS_SRCRFXI1_2
    { HSPR_FX14,  (5 | FF_FULLBRIGHT),    5, NULL,                   HS_SRCRFXI1_4       }, // HS_SRCRFXI1_3
    { HSPR_FX14,  (6 | FF_FULLBRIGHT),    5, NULL,                   HS_SRCRFXI1_5       }, // HS_SRCRFXI1_4
    { HSPR_FX14,  (7 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_SRCRFXI1_5
    { HSPR_SOR2,   0,                     4, NULL,                   HS_SOR2_RISE2       }, // HS_SOR2_RISE1
    { HSPR_SOR2,   1,                     4, NULL,                   HS_SOR2_RISE3       }, // HS_SOR2_RISE2
    { HSPR_SOR2,   2,                     4, A_SorRise,              HS_SOR2_RISE4       }, // HS_SOR2_RISE3
    { HSPR_SOR2,   3,                     4, NULL,                   HS_SOR2_RISE5       }, // HS_SOR2_RISE4
    { HSPR_SOR2,   4,                     4, NULL,                   HS_SOR2_RISE6       }, // HS_SOR2_RISE5
    { HSPR_SOR2,   5,                     4, NULL,                   HS_SOR2_RISE7       }, // HS_SOR2_RISE6
    { HSPR_SOR2,   6,                    12, A_SorSightSnd,          HS_SOR2_WALK1       }, // HS_SOR2_RISE7
    { HSPR_SOR2,  12,                    10, A_Look,                 HS_SOR2_LOOK2       }, // HS_SOR2_LOOK1
    { HSPR_SOR2,  13,                    10, A_Look,                 HS_SOR2_LOOK1       }, // HS_SOR2_LOOK2
    { HSPR_SOR2,  12,                     4, A_Chase,                HS_SOR2_WALK2       }, // HS_SOR2_WALK1
    { HSPR_SOR2,  13,                     4, A_Chase,                HS_SOR2_WALK3       }, // HS_SOR2_WALK2
    { HSPR_SOR2,  14,                     4, A_Chase,                HS_SOR2_WALK4       }, // HS_SOR2_WALK3
    { HSPR_SOR2,  15,                     4, A_Chase,                HS_SOR2_WALK1       }, // HS_SOR2_WALK4
    { HSPR_SOR2,  16,                     3, NULL,                   HS_SOR2_PAIN2       }, // HS_SOR2_PAIN1
    { HSPR_SOR2,  16,                     6, A_Pain,                 HS_SOR2_WALK1       }, // HS_SOR2_PAIN2
    { HSPR_SOR2,  17,                     9, A_Srcr2Decide,          HS_SOR2_ATK2        }, // HS_SOR2_ATK1
    { HSPR_SOR2,  18,                     9, A_FaceTarget,           HS_SOR2_ATK3        }, // HS_SOR2_ATK2
    { HSPR_SOR2,  19,                    20, A_Srcr2Attack,          HS_SOR2_WALK1       }, // HS_SOR2_ATK3
    { HSPR_SOR2,  11,                     6, NULL,                   HS_SOR2_TELE2       }, // HS_SOR2_TELE1
    { HSPR_SOR2,  10,                     6, NULL,                   HS_SOR2_TELE3       }, // HS_SOR2_TELE2
    { HSPR_SOR2,   9,                     6, NULL,                   HS_SOR2_TELE4       }, // HS_SOR2_TELE3
    { HSPR_SOR2,   8,                     6, NULL,                   HS_SOR2_TELE5       }, // HS_SOR2_TELE4
    { HSPR_SOR2,   7,                     6, NULL,                   HS_SOR2_TELE6       }, // HS_SOR2_TELE5
    { HSPR_SOR2,   6,                     6, NULL,                   HS_SOR2_WALK1       }, // HS_SOR2_TELE6
    { HSPR_SDTH,   0,                     8, A_Sor2DthInit,          HS_SOR2_DIE2        }, // HS_SOR2_DIE1
    { HSPR_SDTH,   1,                     8, NULL,                   HS_SOR2_DIE3        }, // HS_SOR2_DIE2
    { HSPR_SDTH,   2,                     8, A_SorDSph,              HS_SOR2_DIE4        }, // HS_SOR2_DIE3
    { HSPR_SDTH,   3,                     7, NULL,                   HS_SOR2_DIE5        }, // HS_SOR2_DIE4
    { HSPR_SDTH,   4,                     7, NULL,                   HS_SOR2_DIE6        }, // HS_SOR2_DIE5
    { HSPR_SDTH,   5,                     7, A_Sor2DthLoop,          HS_SOR2_DIE7        }, // HS_SOR2_DIE6
    { HSPR_SDTH,   6,                     6, A_SorDExp,              HS_SOR2_DIE8        }, // HS_SOR2_DIE7
    { HSPR_SDTH,   7,                     6, NULL,                   HS_SOR2_DIE9        }, // HS_SOR2_DIE8
    { HSPR_SDTH,   8,                    18, NULL,                   HS_SOR2_DIE10       }, // HS_SOR2_DIE9
    { HSPR_SDTH,   9,                     6, A_NoBlocking,           HS_SOR2_DIE11       }, // HS_SOR2_DIE10
    { HSPR_SDTH,  10,                     6, A_SorDBon,              HS_SOR2_DIE12       }, // HS_SOR2_DIE11
    { HSPR_SDTH,  11,                     6, NULL,                   HS_SOR2_DIE13       }, // HS_SOR2_DIE12
    { HSPR_SDTH,  12,                     6, NULL,                   HS_SOR2_DIE14       }, // HS_SOR2_DIE13
    { HSPR_SDTH,  13,                     6, NULL,                   HS_SOR2_DIE15       }, // HS_SOR2_DIE14
    { HSPR_SDTH,  14,                    -1, A_BossDeath2,           HS_NULL             }, // HS_SOR2_DIE15
    { HSPR_FX16,  (0 | FF_FULLBRIGHT),    3, A_BlueSpark,            HS_SOR2FX1_2        }, // HS_SOR2FX1_1
    { HSPR_FX16,  (1 | FF_FULLBRIGHT),    3, A_BlueSpark,            HS_SOR2FX1_3        }, // HS_SOR2FX1_2
    { HSPR_FX16,  (2 | FF_FULLBRIGHT),    3, A_BlueSpark,            HS_SOR2FX1_1        }, // HS_SOR2FX1_3
    { HSPR_FX16,  (6 | FF_FULLBRIGHT),    5, A_Explode,              HS_SOR2FXI1_2       }, // HS_SOR2FXI1_1
    { HSPR_FX16,  (7 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FXI1_3       }, // HS_SOR2FXI1_2
    { HSPR_FX16,  (8 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FXI1_4       }, // HS_SOR2FXI1_3
    { HSPR_FX16,  (9 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FXI1_5       }, // HS_SOR2FXI1_4
    { HSPR_FX16, (10 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FXI1_6       }, // HS_SOR2FXI1_5
    { HSPR_FX16, (11 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_SOR2FXI1_6
    { HSPR_FX16,  (3 | FF_FULLBRIGHT),   12, NULL,                   HS_SOR2FXSPARK2     }, // HS_SOR2FXSPARK1
    { HSPR_FX16,  (4 | FF_FULLBRIGHT),   12, NULL,                   HS_SOR2FXSPARK3     }, // HS_SOR2FXSPARK2
    { HSPR_FX16,  (5 | FF_FULLBRIGHT),   12, NULL,                   HS_NULL             }, // HS_SOR2FXSPARK3
    { HSPR_FX11,  (0 | FF_FULLBRIGHT),   35, NULL,                   HS_SOR2FX2_2        }, // HS_SOR2FX2_1
    { HSPR_FX11,  (0 | FF_FULLBRIGHT),    5, A_GenWizard,            HS_SOR2FX2_3        }, // HS_SOR2FX2_2
    { HSPR_FX11,  (1 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FX2_2        }, // HS_SOR2FX2_3
    { HSPR_FX11,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FXI2_2       }, // HS_SOR2FXI2_1
    { HSPR_FX11,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FXI2_3       }, // HS_SOR2FXI2_2
    { HSPR_FX11,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FXI2_4       }, // HS_SOR2FXI2_3
    { HSPR_FX11,  (5 | FF_FULLBRIGHT),    5, NULL,                   HS_SOR2FXI2_5       }, // HS_SOR2FXI2_4
    { HSPR_FX11,  (6 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_SOR2FXI2_5
    { HSPR_SOR2,   6,                     8, NULL,                   HS_SOR2TELEFADE2    }, // HS_SOR2TELEFADE1
    { HSPR_SOR2,   7,                     6, NULL,                   HS_SOR2TELEFADE3    }, // HS_SOR2TELEFADE2
    { HSPR_SOR2,   8,                     6, NULL,                   HS_SOR2TELEFADE4    }, // HS_SOR2TELEFADE3
    { HSPR_SOR2,   9,                     6, NULL,                   HS_SOR2TELEFADE5    }, // HS_SOR2TELEFADE4
    { HSPR_SOR2,  10,                     6, NULL,                   HS_SOR2TELEFADE6    }, // HS_SOR2TELEFADE5
    { HSPR_SOR2,  11,                     6, NULL,                   HS_NULL             }, // HS_SOR2TELEFADE6
    { HSPR_MNTR,   0,                    10, A_Look,                 HS_MNTR_LOOK2       }, // HS_MNTR_LOOK1
    { HSPR_MNTR,   1,                    10, A_Look,                 HS_MNTR_LOOK1       }, // HS_MNTR_LOOK2
    { HSPR_MNTR,   0,                     5, A_Chase,                HS_MNTR_WALK2       }, // HS_MNTR_WALK1
    { HSPR_MNTR,   1,                     5, A_Chase,                HS_MNTR_WALK3       }, // HS_MNTR_WALK2
    { HSPR_MNTR,   2,                     5, A_Chase,                HS_MNTR_WALK4       }, // HS_MNTR_WALK3
    { HSPR_MNTR,   3,                     5, A_Chase,                HS_MNTR_WALK1       }, // HS_MNTR_WALK4
    { HSPR_MNTR,  21,                    10, A_FaceTarget,           HS_MNTR_ATK1_2      }, // HS_MNTR_ATK1_1
    { HSPR_MNTR,  22,                     7, A_FaceTarget,           HS_MNTR_ATK1_3      }, // HS_MNTR_ATK1_2
    { HSPR_MNTR,  23,                    12, A_MinotaurAtk1,         HS_MNTR_WALK1       }, // HS_MNTR_ATK1_3
    { HSPR_MNTR,  21,                    10, A_MinotaurDecide,       HS_MNTR_ATK2_2      }, // HS_MNTR_ATK2_1
    { HSPR_MNTR,  24,                     4, A_FaceTarget,           HS_MNTR_ATK2_3      }, // HS_MNTR_ATK2_2
    { HSPR_MNTR,  25,                     9, A_MinotaurAtk2,         HS_MNTR_WALK1       }, // HS_MNTR_ATK2_3
    { HSPR_MNTR,  21,                    10, A_FaceTarget,           HS_MNTR_ATK3_2      }, // HS_MNTR_ATK3_1
    { HSPR_MNTR,  22,                     7, A_FaceTarget,           HS_MNTR_ATK3_3      }, // HS_MNTR_ATK3_2
    { HSPR_MNTR,  23,                    12, A_MinotaurAtk3,         HS_MNTR_WALK1       }, // HS_MNTR_ATK3_3
    { HSPR_MNTR,  23,                    12, NULL,                   HS_MNTR_ATK3_1      }, // HS_MNTR_ATK3_4
    { HSPR_MNTR,  20,                     2, A_MinotaurCharge,       HS_MNTR_ATK4_1      }, // HS_MNTR_ATK4_1
    { HSPR_MNTR,   4,                     3, NULL,                   HS_MNTR_PAIN2       }, // HS_MNTR_PAIN1
    { HSPR_MNTR,   4,                     6, A_Pain,                 HS_MNTR_WALK1       }, // HS_MNTR_PAIN2
    { HSPR_MNTR,   5,                     6, NULL,                   HS_MNTR_DIE2        }, // HS_MNTR_DIE1
    { HSPR_MNTR,   6,                     5, NULL,                   HS_MNTR_DIE3        }, // HS_MNTR_DIE2
    { HSPR_MNTR,   7,                     6, A_Scream,               HS_MNTR_DIE4        }, // HS_MNTR_DIE3
    { HSPR_MNTR,   8,                     5, NULL,                   HS_MNTR_DIE5        }, // HS_MNTR_DIE4
    { HSPR_MNTR,   9,                     6, NULL,                   HS_MNTR_DIE6        }, // HS_MNTR_DIE5
    { HSPR_MNTR,  10,                     5, NULL,                   HS_MNTR_DIE7        }, // HS_MNTR_DIE6
    { HSPR_MNTR,  11,                     6, NULL,                   HS_MNTR_DIE8        }, // HS_MNTR_DIE7
    { HSPR_MNTR,  12,                     5, A_NoBlocking,           HS_MNTR_DIE9        }, // HS_MNTR_DIE8
    { HSPR_MNTR,  13,                     6, NULL,                   HS_MNTR_DIE10       }, // HS_MNTR_DIE9
    { HSPR_MNTR,  14,                     5, NULL,                   HS_MNTR_DIE11       }, // HS_MNTR_DIE10
    { HSPR_MNTR,  15,                     6, NULL,                   HS_MNTR_DIE12       }, // HS_MNTR_DIE11
    { HSPR_MNTR,  16,                     5, NULL,                   HS_MNTR_DIE13       }, // HS_MNTR_DIE12
    { HSPR_MNTR,  17,                     6, NULL,                   HS_MNTR_DIE14       }, // HS_MNTR_DIE13
    { HSPR_MNTR,  18,                     5, NULL,                   HS_MNTR_DIE15       }, // HS_MNTR_DIE14
    { HSPR_MNTR,  19,                    -1, A_BossDeath2,           HS_NULL             }, // HS_MNTR_DIE15
    { HSPR_FX12,  (0 | FF_FULLBRIGHT),    6, NULL,                   HS_MNTRFX1_2        }, // HS_MNTRFX1_1
    { HSPR_FX12,  (1 | FF_FULLBRIGHT),    6, NULL,                   HS_MNTRFX1_1        }, // HS_MNTRFX1_2
    { HSPR_FX12,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFXI1_2       }, // HS_MNTRFXI1_1
    { HSPR_FX12,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFXI1_3       }, // HS_MNTRFXI1_2
    { HSPR_FX12,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFXI1_4       }, // HS_MNTRFXI1_3
    { HSPR_FX12,  (5 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFXI1_5       }, // HS_MNTRFXI1_4
    { HSPR_FX12,  (6 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFXI1_6       }, // HS_MNTRFXI1_5
    { HSPR_FX12,  (7 | FF_FULLBRIGHT),    5, NULL,                   HS_NULL             }, // HS_MNTRFXI1_6
    { HSPR_FX13,   0,                     2, A_MntrFloorFire,        HS_MNTRFX2_1        }, // HS_MNTRFX2_1
    { HSPR_FX13,  (8 | FF_FULLBRIGHT),    4, A_Explode,              HS_MNTRFXI2_2       }, // HS_MNTRFXI2_1
    { HSPR_FX13,  (9 | FF_FULLBRIGHT),    4, NULL,                   HS_MNTRFXI2_3       }, // HS_MNTRFXI2_2
    { HSPR_FX13, (10 | FF_FULLBRIGHT),    4, NULL,                   HS_MNTRFXI2_4       }, // HS_MNTRFXI2_3
    { HSPR_FX13, (11 | FF_FULLBRIGHT),    4, NULL,                   HS_MNTRFXI2_5       }, // HS_MNTRFXI2_4
    { HSPR_FX13, (12 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_MNTRFXI2_5
    { HSPR_FX13,  (3 | FF_FULLBRIGHT),    4, NULL,                   HS_MNTRFX3_2        }, // HS_MNTRFX3_1
    { HSPR_FX13,  (2 | FF_FULLBRIGHT),    4, NULL,                   HS_MNTRFX3_3        }, // HS_MNTRFX3_2
    { HSPR_FX13,  (1 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFX3_4        }, // HS_MNTRFX3_3
    { HSPR_FX13,  (2 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFX3_5        }, // HS_MNTRFX3_4
    { HSPR_FX13,  (3 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFX3_6        }, // HS_MNTRFX3_5
    { HSPR_FX13,  (4 | FF_FULLBRIGHT),    5, NULL,                   HS_MNTRFX3_7        }, // HS_MNTRFX3_6
    { HSPR_FX13,  (5 | FF_FULLBRIGHT),    4, NULL,                   HS_MNTRFX3_8        }, // HS_MNTRFX3_7
    { HSPR_FX13,  (6 | FF_FULLBRIGHT),    4, NULL,                   HS_MNTRFX3_9        }, // HS_MNTRFX3_8
    { HSPR_FX13,  (7 | FF_FULLBRIGHT),    4, NULL,                   HS_NULL             }, // HS_MNTRFX3_9
    { HSPR_AKYY,  (0 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY2            }, // HS_AKYY1
    { HSPR_AKYY,  (1 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY3            }, // HS_AKYY2
    { HSPR_AKYY,  (2 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY4            }, // HS_AKYY3
    { HSPR_AKYY,  (3 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY5            }, // HS_AKYY4
    { HSPR_AKYY,  (4 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY6            }, // HS_AKYY5
    { HSPR_AKYY,  (5 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY7            }, // HS_AKYY6
    { HSPR_AKYY,  (6 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY8            }, // HS_AKYY7
    { HSPR_AKYY,  (7 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY9            }, // HS_AKYY8
    { HSPR_AKYY,  (8 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY10           }, // HS_AKYY9
    { HSPR_AKYY,  (9 | FF_FULLBRIGHT),    3, NULL,                   HS_AKYY1            }, // HS_AKYY10
    { HSPR_BKYY,  (0 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY2            }, // HS_BKYY1
    { HSPR_BKYY,  (1 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY3            }, // HS_BKYY2
    { HSPR_BKYY,  (2 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY4            }, // HS_BKYY3
    { HSPR_BKYY,  (3 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY5            }, // HS_BKYY4
    { HSPR_BKYY,  (4 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY6            }, // HS_BKYY5
    { HSPR_BKYY,  (5 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY7            }, // HS_BKYY6
    { HSPR_BKYY,  (6 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY8            }, // HS_BKYY7
    { HSPR_BKYY,  (7 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY9            }, // HS_BKYY8
    { HSPR_BKYY,  (8 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY10           }, // HS_BKYY9
    { HSPR_BKYY,  (9 | FF_FULLBRIGHT),    3, NULL,                   HS_BKYY1            }, // HS_BKYY10
    { HSPR_CKYY,  (0 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY2            }, // HS_CKYY1
    { HSPR_CKYY,  (1 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY3            }, // HS_CKYY2
    { HSPR_CKYY,  (2 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY4            }, // HS_CKYY3
    { HSPR_CKYY,  (3 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY5            }, // HS_CKYY4
    { HSPR_CKYY,  (4 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY6            }, // HS_CKYY5
    { HSPR_CKYY,  (5 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY7            }, // HS_CKYY6
    { HSPR_CKYY,  (6 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY8            }, // HS_CKYY7
    { HSPR_CKYY,  (7 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY9            }, // HS_CKYY8
    { HSPR_CKYY,  (8 | FF_FULLBRIGHT),    3, NULL,                   HS_CKYY1            }, // HS_CKYY9
    { HSPR_AMG1,   0,                    -1, NULL,                   HS_NULL             }, // HS_AMG1
    { HSPR_AMG2,   0,                     4, NULL,                   HS_AMG2_2           }, // HS_AMG2_1
    { HSPR_AMG2,   1,                     4, NULL,                   HS_AMG2_3           }, // HS_AMG2_2
    { HSPR_AMG2,   2,                     4, NULL,                   HS_AMG2_1           }, // HS_AMG2_3
    { HSPR_AMM1,   0,                    -1, NULL,                   HS_NULL             }, // HS_AMM1
    { HSPR_AMM2,   0,                    -1, NULL,                   HS_NULL             }, // HS_AMM2
    { HSPR_AMC1,   0,                    -1, NULL,                   HS_NULL             }, // HS_AMC1
    { HSPR_AMC2,   0,                     5, NULL,                   HS_AMC2_2           }, // HS_AMC2_1
    { HSPR_AMC2,   1,                     5, NULL,                   HS_AMC2_3           }, // HS_AMC2_2
    { HSPR_AMC2,   2,                     5, NULL,                   HS_AMC2_1           }, // HS_AMC2_3
    { HSPR_AMS1,   0,                     5, NULL,                   HS_AMS1_2           }, // HS_AMS1_1
    { HSPR_AMS1,   1,                     5, NULL,                   HS_AMS1_1           }, // HS_AMS1_2
    { HSPR_AMS2,   0,                     5, NULL,                   HS_AMS2_2           }, // HS_AMS2_1
    { HSPR_AMS2,   1,                     5, NULL,                   HS_AMS2_1           }, // HS_AMS2_2
    { HSPR_AMP1,   0,                     4, NULL,                   HS_AMP1_2           }, // HS_AMP1_1
    { HSPR_AMP1,   1,                     4, NULL,                   HS_AMP1_3           }, // HS_AMP1_2
    { HSPR_AMP1,   2,                     4, NULL,                   HS_AMP1_1           }, // HS_AMP1_3
    { HSPR_AMP2,   0,                     4, NULL,                   HS_AMP2_2           }, // HS_AMP2_1
    { HSPR_AMP2,   1,                     4, NULL,                   HS_AMP2_3           }, // HS_AMP2_2
    { HSPR_AMP2,   2,                     4, NULL,                   HS_AMP2_1           }, // HS_AMP2_3
    { HSPR_AMB1,   0,                     4, NULL,                   HS_AMB1_2           }, // HS_AMB1_1
    { HSPR_AMB1,   1,                     4, NULL,                   HS_AMB1_3           }, // HS_AMB1_2
    { HSPR_AMB1,   2,                     4, NULL,                   HS_AMB1_1           }, // HS_AMB1_3
    { HSPR_AMB2,   0,                     4, NULL,                   HS_AMB2_2           }, // HS_AMB2_1
    { HSPR_AMB2,   1,                     4, NULL,                   HS_AMB2_3           }, // HS_AMB2_2
    { HSPR_AMB2,   2,                     4, NULL,                   HS_AMB2_1           }, // HS_AMB2_3
    { HSPR_AMG1,   0,                   100, A_ESound,               HS_SND_WIND         }, // HS_SND_WIND
    { HSPR_AMG1,   0,                    85, A_ESound,               HS_SND_WATERFALL    }  // HS_SND_WATERFALL
};
