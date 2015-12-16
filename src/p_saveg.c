/*
========================================================================

                               DOOM Retro
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright � 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright � 2013-2016 Brad Harding.

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

#include "am_map.h"
#include "c_console.h"
#include "d_deh.h"
#include "doomstat.h"
#include "dstrings.h"
#include "i_system.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_saveg.h"
#include "p_tick.h"
#include "version.h"
#include "z_zone.h"

#define SAVEGAME_EOF    0x1D

FILE    *save_stream;
int     savegamelength;
dboolean savegame_error;

void P_SpawnShadow(mobj_t *actor);

// Get the filename of a temporary file to write the savegame to. After
// the file has been successfully saved, it will be renamed to the
// real file.
char *P_TempSaveGameFile(void)
{
    static char *filename;

    if (!filename)
        filename = M_StringJoin(savegamefolder, "temp.save", NULL);

    return filename;
}

// Get the filename of the save game file to use for the specified slot.
char *P_SaveGameFile(int slot)
{
    static char         *filename;
    static size_t       filename_size;
    char                basename[32];

    if (!filename)
    {
        filename_size = strlen(savegamefolder) + 32;
        filename = malloc(filename_size);
    }

    M_snprintf(basename, 32, PACKAGE_SAVE, slot);
    M_snprintf(filename, filename_size, "%s%s", savegamefolder, basename);

    return filename;
}

// Endian-safe integer read/write functions
static byte saveg_read8(void)
{
    byte        result;

    if (fread(&result, 1, 1, save_stream) < 1)
        savegame_error = true;

    return result;
}

static void saveg_write8(byte value)
{
    if (fwrite(&value, 1, 1, save_stream) < 1)
        savegame_error = true;
}

static short saveg_read16(void)
{
    int result;

    result = saveg_read8();
    result |= saveg_read8() << 8;

    return result;
}

static void saveg_write16(short value)
{
    saveg_write8(value & 0xFF);
    saveg_write8((value >> 8) & 0xFF);
}

static int saveg_read32(void)
{
    int result;

    result = saveg_read8();
    result |= saveg_read8() << 8;
    result |= saveg_read8() << 16;
    result |= saveg_read8() << 24;

    return result;
}

static void saveg_write32(int value)
{
    saveg_write8(value & 0xFF);
    saveg_write8((value >> 8) & 0xFF);
    saveg_write8((value >> 16) & 0xFF);
    saveg_write8((value >> 24) & 0xFF);
}

// Pad to 4-byte boundaries
static void saveg_read_pad(void)
{
    unsigned long       pos = ftell(save_stream);
    int                 padding = (4 - (pos & 3)) & 3;
    int                 i;

    for (i = 0; i < padding; ++i)
        saveg_read8();
}

static void saveg_write_pad(void)
{
    unsigned long       pos = ftell(save_stream);
    int                 padding = (4 - (pos & 3)) & 3;
    int                 i;

    for (i = 0; i < padding; ++i)
        saveg_write8(0);
}

// Pointers
static void *saveg_readp(void)
{
    return (void *)(intptr_t)saveg_read32();
}

static void saveg_writep(void *p)
{
    saveg_write32((intptr_t)p);
}

// Enum values are 32-bit integers.
#define saveg_read_enum         saveg_read32
#define saveg_write_enum        saveg_write32

//
// Structure read/write functions
//

//
// mapthing_t
//
static void saveg_read_mapthing_t(mapthing_t *str)
{
    // short x
    str->x = saveg_read16();

    // short y
    str->y = saveg_read16();

    // short angle
    str->angle = saveg_read16();

    // short type
    str->type = saveg_read16();

    // short options
    str->options = saveg_read16();
}

static void saveg_write_mapthing_t(mapthing_t *str)
{
    // short x
    saveg_write16(str->x);

    // short y
    saveg_write16(str->y);

    // short angle
    saveg_write16(str->angle);

    // short type
    saveg_write16(str->type);

    // short options
    saveg_write16(str->options);
}

//
// mobj_t
//
static void saveg_read_mobj_t(mobj_t *str)
{
    int pl;

    // fixed_t x
    str->x = saveg_read32();

    // fixed_t y
    str->y = saveg_read32();

    // fixed_t z
    str->z = saveg_read32();

    // struct mobj_s *snext
    str->snext = (mobj_t *)saveg_readp();

    // struct mobj_s **sprev
    str->sprev = (mobj_t **)saveg_readp();

    // angle_t angle
    str->angle = saveg_read32();

    // spritenum_t sprite
    str->sprite = (spritenum_t)saveg_read_enum();

    // int frame
    str->frame = saveg_read32();

    // struct mobj_s *bnext
    str->bnext = (mobj_t *)saveg_readp();

    // struct mobj_s **bprev
    str->bprev = (mobj_t **)saveg_readp();

    // struct subsector_s *subsector
    str->subsector = (subsector_t *)saveg_readp();

    // fixed_t floorz
    str->floorz = saveg_read32();

    // fixed_t ceilingz
    str->ceilingz = saveg_read32();

    // fixed_t dropoffz
    str->dropoffz = saveg_read32();

    // fixed_t radius
    str->radius = saveg_read32();

    // fixed_t height
    str->height = saveg_read32();

    // fixed_t projectilepassheight
    str->projectilepassheight = saveg_read32();

    // fixed_t momx
    str->momx = saveg_read32();

    // fixed_t momy
    str->momy = saveg_read32();

    // fixed_t momz
    str->momz = saveg_read32();

    // int validcount
    str->validcount = saveg_read32();

    // mobjtype_t type
    str->type = (mobjtype_t)saveg_read_enum();

    // mobjinfo_t *info
    str->info = (mobjinfo_t *)saveg_readp();

    // int tics
    str->tics = saveg_read32();

    // state_t *state
    str->state = &states[saveg_read32()];

    // int flags
    str->flags = saveg_read32();

    // int flags2
    str->flags2 = saveg_read32();

    // int health
    str->health = saveg_read32();

    // int movedir
    str->movedir = saveg_read32();

    // int movecount
    str->movecount = saveg_read32();

    // struct mobj_s *target
    str->target = (mobj_t *)saveg_readp();

    // int reactiontime
    str->reactiontime = saveg_read32();

    // int threshold
    str->threshold = saveg_read32();

    // struct player_s *player
    if ((pl = saveg_read32()) > 0)
    {
        str->player = &players[pl - 1];
        str->player->mo = str;
    }
    else
        str->player = NULL;

    // int lastlook
    str->lastlook = saveg_read32();

    // mapthing_t spawnpoint
    saveg_read_mapthing_t(&str->spawnpoint);

    // struct mobj_s *tracer
    str->tracer = (mobj_t *)saveg_readp();

    // struct mobj_s *lastenemy
    str->lastenemy = (mobj_t *)saveg_readp();

    // int floatbob
    str->floatbob = saveg_read32();

    // struct msecnode_s *touching_sectorlist
    str->touching_sectorlist = NULL;

    // short gear
    str->gear = saveg_read16();

    // int bloodsplats
    str->bloodsplats = saveg_read32();

    // struct mobj_s *shadow
    str->shadow = NULL;

    // int blood
    str->blood = saveg_read32();

    // dboolean interp
    str->interp = saveg_read32();

    // int oldx
    str->oldx = saveg_read32();

    // int oldy
    str->oldy = saveg_read32();

    // int oldz
    str->oldz = saveg_read32();

    // angle_t oldangle
    str->oldangle = saveg_read32();

    // int pitch
    str->pitch = saveg_read32();
}

static void saveg_write_mobj_t(mobj_t *str)
{
    // fixed_t x
    saveg_write32(str->x);

    // fixed_t y
    saveg_write32(str->y);

    // fixed_t z
    saveg_write32(str->z);

    // struct mobj_s *snext
    saveg_writep(str->snext);

    // struct mobj_s *sprev
    saveg_writep(str->sprev);

    // angle_t angle
    saveg_write32(str->angle);

    // spritenum_t sprite
    saveg_write_enum(str->sprite);

    // int frame
    saveg_write32(str->frame);

    // struct mobj_s *bnext
    saveg_writep(str->bnext);

    // struct mobj_s *bprev
    saveg_writep(str->bprev);

    // struct subsector_s *subsector
    saveg_writep(str->subsector);

    // fixed_t floorz
    saveg_write32(str->floorz);

    // fixed_t ceilingz
    saveg_write32(str->ceilingz);

    // fixed_t dropoffz
    saveg_write32(str->dropoffz);

    // fixed_t radius;
    saveg_write32(str->radius);

    // fixed_t height
    saveg_write32(str->height);

    // fixed_t projectilepassheight
    saveg_write32(str->projectilepassheight);

    // fixed_t momx
    saveg_write32(str->momx);

    // fixed_t momy
    saveg_write32(str->momy);

    // fixed_t momz
    saveg_write32(str->momz);

    // int validcount
    saveg_write32(str->validcount);

    // mobjtype_t type
    saveg_write_enum(str->type);

    // mobjinfo_t *info
    saveg_writep(str->info);

    // int tics
    saveg_write32(str->tics);

    // state_t *state
    saveg_write32(str->state - states);

    // int flags
    saveg_write32(str->flags);

    // int flags2
    saveg_write32(str->flags2);

    // int health
    saveg_write32(str->health);

    // int movedir
    saveg_write32(str->movedir);

    // int movecount
    saveg_write32(str->movecount);

    // struct mobj_s *target
    saveg_writep((void *)(uintptr_t)P_ThinkerToIndex((thinker_t *)str->target));

    // int reactiontime
    saveg_write32(str->reactiontime);

    // int threshold
    saveg_write32(str->threshold);

    // struct player_s *player
    saveg_write32(str->player ? str->player - players + 1 : 0);

    // int lastlook
    saveg_write32(str->lastlook);

    // mapthing_t spawnpoint
    saveg_write_mapthing_t(&str->spawnpoint);

    // struct mobj_s *tracer
    saveg_writep((void *)(uintptr_t)P_ThinkerToIndex((thinker_t *)str->tracer));

    // struct mobj_s *lastenemy
    saveg_writep((void *)(uintptr_t)P_ThinkerToIndex((thinker_t *)str->lastenemy));

    // int floatbob
    saveg_write32(str->floatbob);

    // short gear
    saveg_write16(str->gear);

    // int bloodsplats
    saveg_write32(str->bloodsplats);

    // int blood
    saveg_write32(str->blood);

    // dboolean interp
    saveg_write32(str->interp);

    // int oldx
    saveg_write32(str->oldx);

    // int oldy
    saveg_write32(str->oldy);

    // int oldz
    saveg_write32(str->oldz);

    // angle_t oldangle
    saveg_write32(str->oldangle);

    // int pitch
    saveg_write32(str->pitch);
}

//
// ticcmd_t
//
static void saveg_read_ticcmd_t(ticcmd_t *str)
{
    // signed char forwardmove
    str->forwardmove = saveg_read8();

    // signed char sidemove
    str->sidemove = saveg_read8();

    // short angleturn
    str->angleturn = saveg_read16();

    // byte buttons
    str->buttons = saveg_read8();
}

static void saveg_write_ticcmd_t(ticcmd_t *str)
{
    // signed char forwardmove
    saveg_write8(str->forwardmove);

    // signed char sidemove
    saveg_write8(str->sidemove);

    // short angleturn
    saveg_write16(str->angleturn);

    // byte buttons
    saveg_write8(str->buttons);
}

//
// pspdef_t
//
static void saveg_read_pspdef_t(pspdef_t *str)
{
    int state;

    // state_t *state
    state = saveg_read32();
    str->state = (state > 0 ? &states[state] : NULL);

    // int tics
    str->tics = saveg_read32();

    // fixed_t sx
    str->sx = saveg_read32();

    // fixed_t sy
    str->sy = saveg_read32();
}

static void saveg_write_pspdef_t(pspdef_t *str)
{
    // state_t *state
    saveg_write32(str->state ? str->state - states : 0);

    // int tics
    saveg_write32(str->tics);

    // fixed_t sx
    saveg_write32(str->sx);

    // fixed_t sy
    saveg_write32(str->sy);
}

extern int oldhealth;
extern int cardsfound;

//
// player_t
//
static void saveg_read_player_t(player_t *str)
{
    int i;

    // mobj_t *mo
    str->mo = (mobj_t *)saveg_readp();

    // playerstate_t playerstate
    str->playerstate = (playerstate_t)saveg_read_enum();

    // ticcmd_t cmd
    saveg_read_ticcmd_t(&str->cmd);

    // fixed_t viewz
    str->viewz = saveg_read32();

    // fixed_t viewheight
    str->viewheight = saveg_read32();

    // fixed_t deltaviewheight
    str->deltaviewheight = saveg_read32();

    // fixed_t bob
    str->bob = saveg_read32();

    // fixed_t momx
    str->momx = saveg_read32();

    // fixed_t momy
    str->momy = saveg_read32();

    // int health
    str->health = saveg_read32();

    // int oldhealth
    oldhealth = saveg_read32();

    // int armorpoints
    str->armorpoints = saveg_read32();

    // int armortype
    str->armortype = saveg_read32();

    // int powers[NUMPOWERS]
    for (i = 0; i < NUMPOWERS; ++i)
        str->powers[i] = saveg_read32();

    // int cards[NUMCARDS]
    for (i = 0; i < NUMCARDS; ++i)
    {
        str->cards[i] = saveg_read32();
        cardsfound = MAX(cardsfound, str->cards[i]);
    }

    // int neededcard
    str->neededcard = saveg_read32();

    // int neededcardflash
    str->neededcardflash = saveg_read32();

    // dboolean backpack
    str->backpack = saveg_read32();

    // weapontype_t readyweapon
    str->readyweapon = (weapontype_t)saveg_read_enum();

    // weapontype_t pendingweapon
    str->pendingweapon = (weapontype_t)saveg_read_enum();

    // dboolean weaponowned[NUMWEAPONS]
    for (i = 0; i < NUMWEAPONS; ++i)
        str->weaponowned[i] = saveg_read32();

    str->shotguns = (str->weaponowned[wp_shotgun] || str->weaponowned[wp_supershotgun]);

    // int ammo[NUMAMMO]
    for (i = 0; i < NUMAMMO; ++i)
        str->ammo[i] = saveg_read32();

    // int maxammo[NUMAMMO]
    for (i = 0; i < NUMAMMO; ++i)
        str->maxammo[i] = saveg_read32();

    // int attackdown
    str->attackdown = saveg_read32();

    // int usedown
    str->usedown = saveg_read32();

    // int cheats
    str->cheats = saveg_read32();

    // int refire
    str->refire = saveg_read32();

    // int killcount
    str->killcount = saveg_read32();

    // int itemcount
    str->itemcount = saveg_read32();

    // int secretcount
    str->secretcount = saveg_read32();

    // char *message
    str->message = (char *)saveg_readp();

    // int damagecount
    str->damagecount = saveg_read32();

    // int bonuscount
    str->bonuscount = saveg_read32();

    // mobj_t *attacker
    str->attacker = (mobj_t *)saveg_readp();

    // int extralight
    str->extralight = saveg_read32();

    // int fixedcolormap
    str->fixedcolormap = saveg_read32();

    // int colormap
    str->colormap = saveg_read32();

    // pspdef_t psprites[NUMPSPRITES]
    for (i = 0; i < NUMPSPRITES; ++i)
        saveg_read_pspdef_t(&str->psprites[i]);

    // dboolean didsecret
    str->didsecret = saveg_read32();

    // weapontype_t preferredshotgun
    str->preferredshotgun = (weapontype_t)saveg_read_enum();

    // int shotguns
    str->shotguns = saveg_read32();

    // weapontype_t fistorchainsaw
    str->fistorchainsaw = (weapontype_t)saveg_read_enum();

    // dboolean invulnbeforechoppers
    str->invulnbeforechoppers = saveg_read32();

    // dboolean chainsawbeforechoppers
    str->chainsawbeforechoppers = saveg_read32();

    // weapontype_t weaponbeforechoppers
    str->weaponbeforechoppers = (weapontype_t)saveg_read_enum();

    // angle_t oldviewz
    str->oldviewz = saveg_read32();

    // int damageinflicted
    str->damageinflicted = saveg_read32();

    // int damagereceived
    str->damagereceived = saveg_read32();

    // int cheated
    str->cheated = saveg_read32();

    // int shotshit
    str->shotshit = saveg_read32();

    // int shotsfired
    str->shotsfired = saveg_read32();

    // int deaths
    str->deaths = saveg_read32();
}

static void saveg_write_player_t(player_t *str)
{
    int i;

    // mobj_t *mo
    saveg_writep(str->mo);

    // playerstate_t playerstate
    saveg_write_enum(str->playerstate);

    // ticcmd_t cmd
    saveg_write_ticcmd_t(&str->cmd);

    // fixed_t viewz
    saveg_write32(str->viewz);

    // fixed_t viewheight
    saveg_write32(str->viewheight);

    // fixed_t deltaviewheight
    saveg_write32(str->deltaviewheight);

    // fixed_t bob
    saveg_write32(str->bob);

    // fixed_t momx
    saveg_write32(str->momx);

    // fixed_t momy
    saveg_write32(str->momy);

    // int health
    saveg_write32(str->health);

    // int oldhealth
    saveg_write32(oldhealth);

    // int armorpoints
    saveg_write32(str->armorpoints);

    // int armortype
    saveg_write32(str->armortype);

    // int powers[NUMPOWERS]
    for (i = 0; i < NUMPOWERS; ++i)
        saveg_write32(str->powers[i]);

    // int cards[NUMCARDS]
    for (i = 0; i < NUMCARDS; ++i)
        saveg_write32(str->cards[i]);

    // int neededcard
    saveg_write32(str->neededcard);

    // int neededcardflash
    saveg_write32(str->neededcardflash);

    // dboolean backpack
    saveg_write32(str->backpack);

    // weapontype_t readyweapon
    saveg_write_enum(str->readyweapon);

    // weapontype_t pendingweapon
    saveg_write_enum(str->pendingweapon);

    // dboolean weaponowned[NUMWEAPONS]
    for (i = 0; i < NUMWEAPONS; ++i)
        saveg_write32(str->weaponowned[i]);

    // int ammo[NUMAMMO]
    for (i = 0; i < NUMAMMO; ++i)
        saveg_write32(str->ammo[i]);

    // int maxammo[NUMAMMO]
    for (i = 0; i < NUMAMMO; ++i)
        saveg_write32(str->maxammo[i]);

    // int attackdown
    saveg_write32(str->attackdown);

    // int usedown
    saveg_write32(str->usedown);

    // int cheats
    saveg_write32(str->cheats);

    // int refire
    saveg_write32(str->refire);

    // int killcount
    saveg_write32(str->killcount);

    // int itemcount
    saveg_write32(str->itemcount);

    // int secretcount
    saveg_write32(str->secretcount);

    // char *message
    saveg_writep(str->message);

    // int damagecount
    saveg_write32(str->damagecount);

    // int bonuscount
    saveg_write32(str->bonuscount);

    // mobj_t *attacker
    saveg_writep(str->attacker);

    // int extralight
    saveg_write32(str->extralight);

    // int fixedcolormap
    saveg_write32(str->fixedcolormap);

    // int colormap
    saveg_write32(str->colormap);

    // pspdef_t psprites[NUMPSPRITES]
    for (i = 0; i < NUMPSPRITES; ++i)
        saveg_write_pspdef_t(&str->psprites[i]);

    // dboolean didsecret
    saveg_write32(str->didsecret);

    // weapontype_t prefferedshotgun
    saveg_write_enum(str->preferredshotgun);

    // int shotguns
    saveg_write32(str->shotguns);

    // int fistorchainsaw
    saveg_write32(str->fistorchainsaw);

    // dboolean invulnbeforechoppers
    saveg_write32(str->invulnbeforechoppers);

    // dboolean chainsawbeforechoppers
    saveg_write32(str->chainsawbeforechoppers);

    // weapontype_t weaponbeforechoppers
    saveg_write_enum(str->weaponbeforechoppers);

    // angle_t oldviewz
    saveg_write32(str->oldviewz);

    // int damageinflicted
    saveg_write32(str->damageinflicted);

    // int damagereceived
    saveg_write32(str->damagereceived);

    // int cheated
    saveg_write32(str->cheated);

    // int shotshit
    saveg_write32(str->shotshit);

    // int shotsfired
    saveg_write32(str->shotsfired);

    // int deaths
    saveg_write32(str->deaths);
}

//
// ceiling_t
//
static void saveg_read_ceiling_t(ceiling_t *str)
{
    // ceiling_e type
    str->type = (ceiling_e)saveg_read_enum();

    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // fixed_t bottomheight
    str->bottomheight = saveg_read32();

    // fixed_t topheight
    str->topheight = saveg_read32();

    // fixed_t speed
    str->speed = saveg_read32();

    // fixed_t oldspeed
    str->oldspeed = saveg_read32();

    // dboolean crush
    str->crush = saveg_read32();

    // int newspecial
    str->newspecial = saveg_read32();

    // short texture
    str->texture = saveg_read16();

    // int direction
    str->direction = saveg_read32();

    // int tag
    str->tag = saveg_read32();

    // int olddirection
    str->olddirection = saveg_read32();

    // struct ceilinglist_s
}

static void saveg_write_ceiling_t(ceiling_t *str)
{
    // ceiling_e type
    saveg_write_enum(str->type);

    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // fixed_t bottomheight
    saveg_write32(str->bottomheight);

    // fixed_t topheight
    saveg_write32(str->topheight);

    // fixed_t speed
    saveg_write32(str->speed);

    // fixed_t oldspeed
    saveg_write32(str->oldspeed);

    // dboolean crush
    saveg_write32(str->crush);

    // int newspecial
    saveg_write32(str->newspecial);

    // short texture
    saveg_write16(str->texture);

    // int direction
    saveg_write32(str->direction);

    // int tag
    saveg_write32(str->tag);

    // int olddirection
    saveg_write32(str->olddirection);

    // struct ceilinglist_s
}

//
// vldoor_t
//
static void saveg_read_vldoor_t(vldoor_t *str)
{
    // vldoor_e type
    str->type = (vldoor_e)saveg_read_enum();

    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // fixed_t topheight
    str->topheight = saveg_read32();

    // fixed_t speed
    str->speed = saveg_read32();

    // int direction
    str->direction = saveg_read32();

    // int topwait
    str->topwait = saveg_read32();

    // int topcountdown
    str->topcountdown = saveg_read32();

    // line_t *line
    str->line = &lines[saveg_read32()];

    // int lighttag
    str->lighttag = saveg_read32();
}

static void saveg_write_vldoor_t(vldoor_t *str)
{
    // vldoor_e type
    saveg_write_enum(str->type);

    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // fixed_t topheight
    saveg_write32(str->topheight);

    // fixed_t speed
    saveg_write32(str->speed);

    // int direction
    saveg_write32(str->direction);

    // int topwait
    saveg_write32(str->topwait);

    // int topcountdown
    saveg_write32(str->topcountdown);

    // line_t *line
    saveg_write32(str->line - lines);

    // int lighttag
    saveg_write32(str->lighttag);
}

//
// floormove_t
//
static void saveg_read_floormove_t(floormove_t *str)
{
    // floor_e type
    str->type = (floor_e)saveg_read_enum();

    // dboolean crush
    str->crush = saveg_read32();

    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // int direction
    str->direction = saveg_read32();

    // int newspecial
    str->newspecial = saveg_read32();

    // short texture
    str->texture = saveg_read16();

    // fixed_t floordestheight
    str->floordestheight = saveg_read32();

    // fixed_t speed
    str->speed = saveg_read32();

    // dboolean stopsound
    str->stopsound = saveg_read32();
}

static void saveg_write_floormove_t(floormove_t *str)
{
    // floor_e type
    saveg_write_enum(str->type);

    // dboolean crush
    saveg_write32(str->crush);

    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // int direction
    saveg_write32(str->direction);

    // int newspecial
    saveg_write32(str->newspecial);

    // short texture
    saveg_write16(str->texture);

    // fixed_t floordestheight
    saveg_write32(str->floordestheight);

    // fixed_t speed
    saveg_write32(str->speed);

    // dboolean stopsound
    saveg_write32(str->stopsound);
}

//
// plat_t
//
static void saveg_read_plat_t(plat_t *str)
{
    // thinker_t thinker;
    str->thinker.function = (saveg_read32() ? T_PlatRaise : NULL);

    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // fixed_t speed
    str->speed = saveg_read32();

    // fixed_t low
    str->low = saveg_read32();

    // fixed_t high
    str->high = saveg_read32();

    // int wait
    str->wait = saveg_read32();

    // int count
    str->count = saveg_read32();

    // plat_e status
    str->status = (plat_e)saveg_read_enum();

    // plat_e oldstatus
    str->oldstatus = (plat_e)saveg_read_enum();

    // dboolean crush
    str->crush = saveg_read32();

    // int tag
    str->tag = saveg_read32();

    // plattype_e type
    str->type = (plattype_e)saveg_read_enum();
}

static void saveg_write_plat_t(plat_t *str)
{
    // thinker_t thinker;
    saveg_write32(!!str->thinker.function);

    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // fixed_t speed
    saveg_write32(str->speed);

    // fixed_t low
    saveg_write32(str->low);

    // fixed_t high
    saveg_write32(str->high);

    // int wait
    saveg_write32(str->wait);

    // int count
    saveg_write32(str->count);

    // plat_e status
    saveg_write_enum(str->status);

    // plat_e oldstatus
    saveg_write_enum(str->oldstatus);

    // dboolean crush
    saveg_write32(str->crush);

    // int tag
    saveg_write32(str->tag);

    // plattype_e type
    saveg_write_enum(str->type);
}

//
// lightflash_t
//
static void saveg_read_lightflash_t(lightflash_t *str)
{
    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // int count
    str->count = saveg_read32();

    // int maxlight
    str->maxlight = saveg_read32();

    // int minlight
    str->minlight = saveg_read32();

    // int maxtime
    str->maxtime = saveg_read32();

    // int mintime
    str->mintime = saveg_read32();
}

static void saveg_write_lightflash_t(lightflash_t *str)
{
    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // int count
    saveg_write32(str->count);

    // int maxlight
    saveg_write32(str->maxlight);

    // int minlight
    saveg_write32(str->minlight);

    // int maxtime
    saveg_write32(str->maxtime);

    // int mintime
    saveg_write32(str->mintime);
}

//
// strobe_t
//
static void saveg_read_strobe_t(strobe_t *str)
{
    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // int count
    str->count = saveg_read32();

    // int minlight
    str->minlight = saveg_read32();

    // int maxlight
    str->maxlight = saveg_read32();

    // int darktime
    str->darktime = saveg_read32();

    // int brighttime
    str->brighttime = saveg_read32();
}

static void saveg_write_strobe_t(strobe_t *str)
{
    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // int count
    saveg_write32(str->count);

    // int minlight
    saveg_write32(str->minlight);

    // int maxlight
    saveg_write32(str->maxlight);

    // int darktime
    saveg_write32(str->darktime);

    // int brighttime
    saveg_write32(str->brighttime);
}

//
// glow_t
//
static void saveg_read_glow_t(glow_t *str)
{
    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // int minlight
    str->minlight = saveg_read32();

    // int maxlight
    str->maxlight = saveg_read32();

    // int direction
    str->direction = saveg_read32();
}

static void saveg_write_glow_t(glow_t *str)
{
    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // int minlight
    saveg_write32(str->minlight);

    // int maxlight
    saveg_write32(str->maxlight);

    // int direction
    saveg_write32(str->direction);
}

static void saveg_read_fireflicker_t(fireflicker_t *str)
{
    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // int count
    str->count = saveg_read32();

    // int minlight
    str->minlight = saveg_read32();

    // int maxlight
    str->maxlight = saveg_read32();
}

static void saveg_write_fireflicker_t(fireflicker_t *str)
{
    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // int count
    saveg_write32(str->count);

    // int minlight
    saveg_write32(str->minlight);

    // int maxlight
    saveg_write32(str->maxlight);
}

static void saveg_read_elevator_t(elevator_t *str)
{
    // floor_e type
    str->type = (floor_e)saveg_read_enum();

    // sector_t *sector
    str->sector = &sectors[saveg_read32()];

    // int direction
    str->direction = saveg_read32();

    // fixed_t floordestheight
    str->floordestheight = saveg_read32();

    // fixed_t ceilingdestheight
    str->ceilingdestheight = saveg_read32();

    // fixed_t speed
    str->speed = saveg_read32();
}

static void saveg_write_elevator_t(elevator_t *str)
{
    // floor_e type
    saveg_write_enum(str->type);

    // sector_t *sector
    saveg_write32(str->sector - sectors);

    // int direction
    saveg_write32(str->direction);

    // fixed_t floordestheight
    saveg_write32(str->floordestheight);

    // fixed_t ceilingdestheight
    saveg_write32(str->ceilingdestheight);

    // fixed_t speed
    saveg_write32(str->speed);
}

static void saveg_read_scroll_t(scroll_t *str)
{
    // fixed_t dx
    str->dx = saveg_read32();

    // fixed_t dy
    str->dy = saveg_read32();

    // int affectee
    str->affectee = saveg_read32();

    // int control
    str->control = saveg_read32();

    // fixed_t last_height
    str->last_height = saveg_read32();

    // fixed_t vdx
    str->vdx = saveg_read32();

    // fixed_t vdy
    str->vdy = saveg_read32();

    // int accel
    str->accel = saveg_read32();

    // enum type
    str->type = saveg_read_enum();
}

static void saveg_write_scroll_t(scroll_t *str)
{
    // fixed_t dx
    saveg_write32(str->dx);

    // fixed_t dy
    saveg_write32(str->dy);

    // int affectee
    saveg_write32(str->affectee);

    // int control
    saveg_write32(str->control);

    // fixed_t last_height
    saveg_write32(str->last_height);

    // fixed_t vdx
    saveg_write32(str->vdx);

    // fixed_t vdy
    saveg_write32(str->vdy);

    // int accel
    saveg_write32(str->accel);

    // enum type
    saveg_write_enum(str->type);
}

static void saveg_read_pusher_t(pusher_t *str)
{
    // enum type
    str->type = saveg_read_enum();

    // int x_mag
    str->x_mag = saveg_read32();

    // int y_mag
    str->y_mag = saveg_read32();

    // int magnitude
    str->magnitude = saveg_read32();

    // int radius
    str->radius = saveg_read32();

    // int x
    str->x = saveg_read32();

    // int y
    str->y = saveg_read32();

    // int affectee
    str->affectee = saveg_read32();
}

static void saveg_write_pusher_t(pusher_t *str)
{
    // enum type
    saveg_write_enum(str->type);

    // int x_mag
    saveg_write32(str->x_mag);

    // int y_mag
    saveg_write32(str->y_mag);

    // int magnitude
    saveg_write32(str->magnitude);

    // int radius
    saveg_write32(str->radius);

    // int x
    saveg_write32(str->x);

    // int y
    saveg_write32(str->y);

    // int affectee
    saveg_write32(str->affectee);
}

static void saveg_read_button_t(button_t *str)
{
    // line_t *line
    str->line = &lines[saveg_read32()];

    // bwhere_e where
    str->where = (bwhere_e)saveg_read32();

    // int btexture
    str->btexture = saveg_read32();

    // int btimer
    str->btimer = saveg_read32();
}

static void saveg_write_button_t(button_t *str)
{
    // line_t *line
    saveg_write32(str->line - lines);

    // bwhere_e where
    saveg_write32((int)str->where);

    // int btexture
    saveg_write32(str->btexture);

    // int btimer
    saveg_write32(str->btimer);
}

//
// Write the header for a savegame
//
void P_WriteSaveGameHeader(char *description)
{
    char        name[VERSIONSIZE];
    int         i;

    for (i = 0; description[i] != '\0'; ++i)
        saveg_write8(description[i]);
    for (; i < SAVESTRINGSIZE; ++i)
        saveg_write8(0);

    memset(name, 0, sizeof(name));
    strcpy(name, PACKAGE_SAVEGAMEVERSIONSTRING);

    for (i = 0; i < VERSIONSIZE; ++i)
        saveg_write8(name[i]);

    saveg_write8(gameskill);
    saveg_write8(gameepisode);
    saveg_write8(gamemap);
    saveg_write8(gamemission);

    saveg_write8((leveltime >> 16) & 0xFF);
    saveg_write8((leveltime >> 8) & 0xFF);
    saveg_write8(leveltime & 0xFF);
}

//
// Read the header for a savegame
//
dboolean P_ReadSaveGameHeader(char *description)
{
    int         i;
    byte        a, b, c;
    char        vcheck[VERSIONSIZE];
    char        read_vcheck[VERSIONSIZE];

    for (i = 0; i < SAVESTRINGSIZE; ++i)
        description[i] = saveg_read8();

    for (i = 0; i < VERSIONSIZE; ++i)
        read_vcheck[i] = saveg_read8();

    memset(vcheck, 0, sizeof(vcheck));
    strcpy(vcheck, PACKAGE_SAVEGAMEVERSIONSTRING);
    if (strcmp(read_vcheck, vcheck))
    {
        menuactive = false;
        consoleheight = 1;
        consoledirection = 1;
        C_Warning("This savegame requires %s.", read_vcheck);
        return false;   // bad version
    }

    gameskill = (skill_t)saveg_read8();
    gameepisode = saveg_read8();
    gamemap = saveg_read8();

    saveg_read8();

    // get the times
    a = saveg_read8();
    b = saveg_read8();
    c = saveg_read8();
    leveltime = (a << 16) + (b << 8) + c;

    return true;
}

//
// Read the end of file marker. Returns true if read successfully.
//
dboolean P_ReadSaveGameEOF(void)
{
    return (saveg_read8() == SAVEGAME_EOF);
}

//
// Write the end of file marker
//
void P_WriteSaveGameEOF(void)
{
    saveg_write8(SAVEGAME_EOF);
}

//
// P_ArchivePlayers
//
void P_ArchivePlayers(void)
{
    saveg_write_pad();

    saveg_write_player_t(&players[0]);
}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers(void)
{
    saveg_read_pad();

    P_InitCards(&players[0]);

    saveg_read_player_t(&players[0]);

    // will be set when unarchiving thinker
    players[0].mo = NULL;
    players[0].message = NULL;
    players[0].attacker = NULL;
}

//
// P_ArchiveWorld
//
void P_ArchiveWorld(void)
{
    int         i;
    int         j;
    sector_t    *sec;
    line_t      *li;
    side_t      *si;

    // do sectors
    for (i = 0, sec = sectors; i < numsectors; i++, sec++)
    {
        saveg_write16(sec->floorheight >> FRACBITS);
        saveg_write16(sec->ceilingheight >> FRACBITS);
        saveg_write16(sec->floorpic);
        saveg_write16(sec->ceilingpic);
        saveg_write16(sec->lightlevel);
        saveg_write16(sec->special);
        saveg_write16(sec->tag);
    }

    // do lines
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
        saveg_write16(li->flags);
        saveg_write16(li->special);
        saveg_write16(li->tag);
        for (j = 0; j < 2; j++)
        {
            if (li->sidenum[j] == NO_INDEX)
                continue;

            si = &sides[li->sidenum[j]];

            saveg_write16(si->textureoffset >> FRACBITS);
            saveg_write16(si->rowoffset >> FRACBITS);
            saveg_write16(si->toptexture);
            saveg_write16(si->bottomtexture);
            saveg_write16(si->midtexture);
        }
    }
}

//
// P_UnArchiveWorld
//
void P_UnArchiveWorld(void)
{
    int         i;
    int         j;
    sector_t    *sec;
    line_t      *li;
    side_t      *si;

    // do sectors
    for (i = 0, sec = sectors; i < numsectors; i++, sec++)
    {
        sec->floorheight = saveg_read16() << FRACBITS;
        sec->ceilingheight = saveg_read16() << FRACBITS;
        sec->floorpic = saveg_read16();
        sec->ceilingpic = saveg_read16();
        sec->lightlevel = saveg_read16();
        sec->special = saveg_read16();
        sec->tag = saveg_read16();
        sec->ceilingdata = NULL;
        sec->floordata = NULL;
        sec->lightingdata = NULL;
        sec->soundtarget = NULL;
    }

    // do lines
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
        li->flags = saveg_read16();
        li->special = saveg_read16();
        li->tag = saveg_read16();
        for (j = 0; j < 2; j++)
        {
            if (li->sidenum[j] == NO_INDEX)
                continue;

            si = &sides[li->sidenum[j]];

            si->textureoffset = saveg_read16() << FRACBITS;
            si->rowoffset = saveg_read16() << FRACBITS;
            si->toptexture = saveg_read16();
            si->bottomtexture = saveg_read16();
            si->midtexture = saveg_read16();
        }
    }
}

//
// Thinkers
//
typedef enum
{
    tc_end,
    tc_mobj,
    tc_bloodsplat
} thinkerclass_t;

//
// P_ArchiveThinkers
//
void P_ArchiveThinkers(void)
{
    thinker_t   *th;
    int         i;

    // save off the current thinkers
    for (th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
    {
        saveg_write8(tc_mobj);
        saveg_write_pad();
        saveg_write_mobj_t((mobj_t *)th);
    }

    // save off the bloodsplats
    for (i = 0; i < numsectors; ++i)
    {
        mobj_t   *mo = sectors[i].thinglist;

        while (mo)
        {
            if (mo->type == MT_BLOODSPLAT)
            {
                saveg_write8(tc_bloodsplat);
                saveg_write_pad();
                saveg_write_mobj_t(mo);
            }
            mo = mo->snext;
        }
    }

    // add a terminating marker
    saveg_write8(tc_end);
}

//
// killough 11/98
//
// Same as P_SetTarget() in p_tick.c, except that the target is nullified
// first, so that no old target's reference count is decreased (when loading
// savegames, old targets are indices, not really pointers to targets).
//
static void P_SetNewTarget(mobj_t **mop, mobj_t *targ)
{
    *mop = NULL;
    P_SetTarget(mop, targ);
}

//
// P_UnArchiveThinkers
//
void P_UnArchiveThinkers(void)
{
    thinker_t   *currentthinker = thinkercap.next;
    thinker_t   *next;
    int i;

    // remove all the current thinkers
    while (currentthinker != &thinkercap)
    {
        next = currentthinker->next;

        if (currentthinker->function == P_MobjThinker)
        {
            P_RemoveMobj((mobj_t *)currentthinker);
            P_RemoveThinkerDelayed(currentthinker);     // fix mobj leak
        }
        else
            Z_Free(currentthinker);

        currentthinker = next;
    }

    P_InitThinkers();

    // remove the remaining bloodsplats and shadows
    for (i = 0; i < numsectors; ++i)
    {
        mobj_t   *mo = sectors[i].thinglist;

        while (mo)
        {
            P_RemoveMobj(mo);
            mo = mo->snext;
        }
    }
    r_bloodsplats_total = 0;

    // read in saved thinkers
    while (1)
    {
        byte    tclass = saveg_read8();
        mobj_t  *mobj;

        switch (tclass)
        {
            case tc_end:
                return;         // end of list

            case tc_mobj:
                saveg_read_pad();
                mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
                saveg_read_mobj_t(mobj);

                P_SetThingPosition(mobj);
                mobj->info = &mobjinfo[mobj->type];

                mobj->thinker.function = P_MobjThinker;
                mobj->colfunc = mobj->info->colfunc;
                mobj->projectfunc = R_ProjectSprite;

                if (mobj->flags2 & MF2_SHADOW)
                {
                    P_SpawnShadow(mobj);

                    if (mobj->flags2 & MF2_MIRRORED)
                        mobj->shadow->flags2 |= MF2_MIRRORED;
                }
                P_AddThinker(&mobj->thinker);
                break;

            case tc_bloodsplat:
                saveg_read_pad();
                mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
                saveg_read_mobj_t(mobj);

                if (r_bloodsplats_total < r_bloodsplats_max)
                {
                    P_SetThingPosition(mobj);
                    mobj->info = &mobjinfo[mobj->type];

                    if (mobj->blood == FUZZYBLOOD)
                    {
                        mobj->flags = MF_FUZZ;
                        mobj->colfunc = fuzzcolfunc;
                    }
                    else
                        mobj->colfunc = bloodsplatcolfunc;
                    mobj->projectfunc = R_ProjectBloodSplat;

                    ++r_bloodsplats_total;
                }
                break;

            default:
                I_Error("P_UnArchiveThinkers: Unknown tclass %i in savegame", tclass);
        }
    }
}

// By Fabian Greffrath. See http://www.doomworld.com/vb/post/1294860.
uint32_t P_ThinkerToIndex(thinker_t *thinker)
{
    thinker_t   *th;
    uint32_t    i;

    if (!thinker)
        return 0;

    for (th = thinkerclasscap[th_mobj].cnext, i = 1; th != &thinkerclasscap[th_mobj];
        th = th->cnext, ++i)
        if (th == thinker)
            return i;

    return 0;
}

thinker_t *P_IndexToThinker(uint32_t index)
{
    thinker_t   *th;
    uint32_t    i;

    if (!index)
        return NULL;

    for (th = thinkerclasscap[th_mobj].cnext, i = 1; th != &thinkerclasscap[th_mobj];
        th = th->cnext, ++i)
        if (i == index)
            return th;

    return NULL;
}

void P_RestoreTargets(void)
{
    thinker_t   *th;

    for (th = thinkerclasscap[th_mobj].cnext; th != &thinkerclasscap[th_mobj]; th = th->cnext)
    {
        mobj_t      *mo = (mobj_t *)th;

        P_SetNewTarget(&mo->target, (mobj_t *)P_IndexToThinker((uintptr_t)mo->target));
        P_SetNewTarget(&mo->tracer, (mobj_t *)P_IndexToThinker((uintptr_t)mo->tracer));
        P_SetNewTarget(&mo->lastenemy, (mobj_t *)P_IndexToThinker((uintptr_t)mo->lastenemy));
    }
}

//
// P_ArchiveSpecials
//
enum
{
    tc_ceiling,
    tc_door,
    tc_floor,
    tc_plat,
    tc_flash,
    tc_strobe,
    tc_glow,
    tc_elevator,        // jff 2/22/98 new elevator type thinker
    tc_scroll,          // killough 3/7/98: new scroll effect thinker
    tc_pusher,          // phares 3/22/98:  new push/pull effect thinker
    tc_fireflicker,     // killough 10/4/98
    tc_button,
    tc_endspecials
} specials_e;

void P_ArchiveSpecials(void)
{
    thinker_t   *th;
    int         i;
    button_t    *button_ptr;

    // save off the current thinkers
    for (th = thinkerclasscap[th_misc].cnext; th != &thinkerclasscap[th_misc]; th = th->cnext)
    {
        if (!th->function)
        {
            dboolean            done_one = false;

            ceilinglist_t       *ceilinglist;
            platlist_t          *platlist;

            for (ceilinglist = activeceilings; ceilinglist; ceilinglist = ceilinglist->next)
                if (ceilinglist->ceiling == (ceiling_t *)th)
                {
                    saveg_write8(tc_ceiling);
                    saveg_write_pad();
                    saveg_write_ceiling_t((ceiling_t *)th);
                    done_one = true;
                    break;
                }

            // [jeff-d] save height of moving platforms
            for (platlist = activeplats; platlist; platlist = platlist->next)
                if (platlist->plat == (plat_t *)th)
                {
                    saveg_write8(tc_plat);
                    saveg_write_pad();
                    saveg_write_plat_t((plat_t *)th);
                    done_one = true;
                    break;
                }

            if (done_one)
                continue;
        }

        if (th->function == T_MoveCeiling)
        {
            saveg_write8(tc_ceiling);
            saveg_write_pad();
            saveg_write_ceiling_t((ceiling_t *)th);
            continue;
        }

        if (th->function == T_VerticalDoor)
        {
            saveg_write8(tc_door);
            saveg_write_pad();
            saveg_write_vldoor_t((vldoor_t *)th);
            continue;
        }

        if (th->function == T_MoveFloor)
        {
            saveg_write8(tc_floor);
            saveg_write_pad();
            saveg_write_floormove_t((floormove_t *)th);
            continue;
        }

        if (th->function == T_PlatRaise)
        {
            saveg_write8(tc_plat);
            saveg_write_pad();
            saveg_write_plat_t((plat_t *)th);
            continue;
        }

        if (th->function == T_LightFlash)
        {
            saveg_write8(tc_flash);
            saveg_write_pad();
            saveg_write_lightflash_t((lightflash_t *)th);
            continue;
        }

        if (th->function == T_StrobeFlash)
        {
            saveg_write8(tc_strobe);
            saveg_write_pad();
            saveg_write_strobe_t((strobe_t *)th);
            continue;
        }

        if (th->function == T_Glow)
        {
            saveg_write8(tc_glow);
            saveg_write_pad();
            saveg_write_glow_t((glow_t *)th);
            continue;
        }

        if (th->function == T_FireFlicker)
        {
            saveg_write8(tc_fireflicker);
            saveg_write_pad();
            saveg_write_fireflicker_t((fireflicker_t *)th);
            continue;
        }

        if (th->function == T_MoveElevator)
        {
            saveg_write8(tc_elevator);
            saveg_write_pad();
            saveg_write_elevator_t((elevator_t *)th);
            continue;
        }

        if (th->function == T_Scroll)
        {
            saveg_write8(tc_scroll);
            saveg_write_pad();
            saveg_write_scroll_t((scroll_t *)th);
            continue;
        }

        if (th->function == T_Pusher)
        {
            saveg_write8(tc_pusher);
            saveg_write_pad();
            saveg_write_pusher_t((pusher_t *)th);
            continue;
        }
    }

    button_ptr = buttonlist;
    i = MAXBUTTONS;
    do
    {
        if (button_ptr->btimer != 0)
        {
            saveg_write8(tc_button);
            saveg_write_pad();
            saveg_write_button_t(button_ptr);
        }
        button_ptr++;
    } while (--i);

    // add a terminating marker
    saveg_write8(tc_endspecials);
}

void P_StartButton(line_t *line, bwhere_e w, int texture, int time);

//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials(void)
{
    ceiling_t           *ceiling;
    vldoor_t            *door;
    floormove_t         *floor;
    plat_t              *plat;
    lightflash_t        *flash;
    strobe_t            *strobe;
    glow_t              *glow;
    fireflicker_t       *fireflicker;
    elevator_t          *elevator;
    scroll_t            *scroll;
    pusher_t            *pusher;
    button_t            *button;

    // read in saved thinkers
    while (1)
    {
        byte            tclass = saveg_read8();

        switch (tclass)
        {
            case tc_endspecials:
                return;          // end of list

            case tc_ceiling:
                saveg_read_pad();
                ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVEL, NULL);
                saveg_read_ceiling_t(ceiling);
                ceiling->sector->ceilingdata = ceiling;
                ceiling->thinker.function = T_MoveCeiling;
                P_AddThinker(&ceiling->thinker);
                P_AddActiveCeiling(ceiling);
                break;

            case tc_door:
                saveg_read_pad();
                door = Z_Malloc(sizeof(*door), PU_LEVEL, NULL);
                saveg_read_vldoor_t(door);
                door->sector->ceilingdata = door;
                door->thinker.function = T_VerticalDoor;
                P_AddThinker(&door->thinker);
                break;

            case tc_floor:
                saveg_read_pad();
                floor = Z_Malloc(sizeof(*floor), PU_LEVEL, NULL);
                saveg_read_floormove_t(floor);
                floor->sector->floordata = floor;
                floor->thinker.function = T_MoveFloor;
                P_AddThinker(&floor->thinker);
                break;

            case tc_plat:
                saveg_read_pad();
                plat = Z_Malloc(sizeof(*plat), PU_LEVEL, NULL);
                saveg_read_plat_t(plat);
                plat->sector->floordata = plat;
                P_AddThinker(&plat->thinker);
                P_AddActivePlat(plat);
                break;

            case tc_flash:
                saveg_read_pad();
                flash = Z_Malloc(sizeof(*flash), PU_LEVEL, NULL);
                saveg_read_lightflash_t(flash);
                flash->thinker.function = T_LightFlash;
                P_AddThinker(&flash->thinker);
                break;

            case tc_strobe:
                saveg_read_pad();
                strobe = Z_Malloc(sizeof(*strobe), PU_LEVEL, NULL);
                saveg_read_strobe_t(strobe);
                strobe->thinker.function = T_StrobeFlash;
                P_AddThinker(&strobe->thinker);
                break;

            case tc_glow:
                saveg_read_pad();
                glow = Z_Malloc(sizeof(*glow), PU_LEVEL, NULL);
                saveg_read_glow_t(glow);
                glow->thinker.function = T_Glow;
                P_AddThinker(&glow->thinker);
                break;

            case tc_fireflicker:
                saveg_read_pad();
                fireflicker = Z_Malloc(sizeof(*fireflicker), PU_LEVEL, NULL);
                saveg_read_fireflicker_t(fireflicker);
                fireflicker->thinker.function = T_FireFlicker;
                P_AddThinker(&fireflicker->thinker);
                break;

            case tc_elevator:
                saveg_read_pad();
                elevator = Z_Malloc(sizeof(*elevator), PU_LEVEL, NULL);
                saveg_read_elevator_t(elevator);
                elevator->sector->ceilingdata = elevator;
                elevator->thinker.function = T_MoveElevator;
                P_AddThinker(&elevator->thinker);
                break;

            case tc_scroll:
                saveg_read_pad();
                scroll = Z_Malloc(sizeof(*scroll), PU_LEVEL, NULL);
                saveg_read_scroll_t(scroll);
                scroll->thinker.function = T_Scroll;
                P_AddThinker(&scroll->thinker);
                break;

            case tc_pusher:
                saveg_read_pad();
                pusher = Z_Malloc(sizeof(*pusher), PU_LEVEL, NULL);
                saveg_read_pusher_t(pusher);
                pusher->thinker.function = T_Pusher;
                pusher->source = P_GetPushThing(pusher->affectee);
                P_AddThinker(&pusher->thinker);
                break;

            case tc_button:
                saveg_read_pad();
                button = Z_Malloc(sizeof(*button), PU_LEVEL, NULL);
                saveg_read_button_t(button);
                P_StartButton(button->line, button->where, button->btexture, button->btimer);
                break;

            default:
                I_Error("P_UnarchiveSpecials: unknown tclass %i in savegame", tclass);
        }
    }
}

//
// P_ArchiveMap
//
void P_ArchiveMap(void)
{
    saveg_write32(automapactive);
    saveg_write32(markpointnum);

    if (markpointnum)
    {
        int     i;

        for (i = 0; i < markpointnum; ++i)
        {
            saveg_write32(markpoints[i].x);
            saveg_write32(markpoints[i].y);
        }
    }
}

//
// P_UnArchiveMap
//
void P_UnArchiveMap(void)
{
    automapactive = saveg_read32();
    markpointnum = saveg_read32();

    if (automapactive || mapwindow)
        AM_Start(automapactive);

    if (markpointnum)
    {
        int     i;

        while (markpointnum >= markpointnum_max)
        {
            markpointnum_max = (markpointnum_max ? markpointnum_max << 1 : 16);
            markpoints = Z_Realloc(markpoints, markpointnum_max * sizeof(*markpoints));
        }

        for (i = 0; i < markpointnum; ++i)
        {
            markpoints[i].x = saveg_read32();
            markpoints[i].y = saveg_read32();
        }
    }
}
