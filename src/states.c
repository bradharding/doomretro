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

#include "d_player.h"
#include "states.h"

void A_BabyMetal(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BetaSkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BFGSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BFGSpray(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BossDeath(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainAwake(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainExplode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainPain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BrainSpit(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BruisAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_BspiAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Chase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CheckReload(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CloseShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CPosRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_CyberAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Detonate(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Die(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Explode(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FaceTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fall(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatAttack3(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FatRaise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Fire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireBFG(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCGun(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireCrackle(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireMissile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireOldBFG(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePistol(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FirePlasma(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireShotgun(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_FireShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_GunFlash(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_HeadAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Hoof(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_KeenDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light0(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light1(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Light2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_LoadShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Look(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Lower(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Metal(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Mushroom(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_OpenShotgun2(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Pain(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PainAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PainDie(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PlayerScream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_PosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Punch(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Raise(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_ReFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SargAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Saw(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Scream(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelFist(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelMissile(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkelWhoosh(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SkullPop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnFly(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpawnSound(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SpidRefire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_SPosAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_StartFire(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Stop(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_Tracer(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_TroopAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileAttack(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileChase(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileStart(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_VileTarget(mobj_t *actor, player_t *player, pspdef_t *psp);
void A_WeaponReady(mobj_t *actor, player_t *player, pspdef_t *psp);
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
    { SPR_BBRN,    1,                   100, A_BrainScream,          S_BRAIN_DIE2        }, // S_BRAIN_DIE1
    { SPR_BBRN,    1,                    10, NULL,                   S_BRAIN_DIE3        }, // S_BRAIN_DIE2
    { SPR_BBRN,    1,                    10, NULL,                   S_BRAIN_DIE4        }, // S_BRAIN_DIE3
    { SPR_BBRN,    1,                    -1, A_BrainDie,             S_NULL              }, // S_BRAIN_DIE4

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
    { SPR_ARM1,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_ARM1A             }, // S_ARM1
    { SPR_ARM1,    1,                     6, NULL,                   S_ARM1              }, // S_ARM1A

    // Blue Armor (MT_MISC1)
    { SPR_ARM2,   (0 | FF_FULLBRIGHT),    6, NULL,                   S_ARM2A             }, // S_ARM2
    { SPR_ARM2,    1,                     6, NULL,                   S_ARM2              }, // S_ARM2A

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
    { SPR_SMIT,    0,                    -1, NULL,                   S_NULL              }, // S_STALAGMITE

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
