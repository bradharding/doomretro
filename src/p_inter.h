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

#pragma once

bool P_GiveBody(const int num, const int max, const bool stat);
bool P_GiveMegaHealth(const bool stat);
bool P_GiveArmor(const armortype_t armortype, const bool stat);
bool P_GiveAllCards(void);
bool P_GiveAllKeyCards(void);
bool P_GiveAllSkullKeys(void);
bool P_GiveAllCardsInMap(void);
bool P_GivePower(const int power);
bool P_GiveAllWeapons(void);
bool P_GiveBackpack(const bool giveammo, const bool stat);
bool P_GiveFullAmmo(void);
void P_AddBonus(void);
void P_UpdateAmmoStat(const ammotype_t ammotype, const int num);
void P_UpdateArmorStat(const int num);
void P_UpdateHealthStat(const int num);
void P_KillMobj(mobj_t *target, mobj_t *inflicter, mobj_t *source, const bool telefragged);

extern int  cardsfound;
