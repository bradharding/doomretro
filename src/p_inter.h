/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see the accompanying AUTHORS file.

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
  along with DOOM Retro. If not, see <http://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

#if !defined(__P_INTER_H__)
#define __P_INTER_H__

dboolean P_GiveBody(player_t *player, int num);
void P_GiveMegaHealth(player_t *player);
dboolean P_GiveArmor(player_t *player, armortype_t armortype);
void P_GiveCard(player_t *player, card_t card);
dboolean P_GiveAllCards(player_t *player);
dboolean P_GivePower(player_t *player, int power);
dboolean P_GiveAllWeapons(player_t *player);
dboolean P_GiveBackpack(player_t *player, dboolean giveammo);
dboolean P_GiveFullAmmo(player_t *player);
void P_AddBonus(player_t *player, int amount);
void G_RemoveChoppers(void);
void P_UpdateKillStat(mobjtype_t type, unsigned int value);
void P_KillMobj(mobj_t *source, mobj_t *target);

extern dboolean oldweaponsowned[NUMWEAPONS];

#endif
