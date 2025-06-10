![](https://github.com/bradharding/www.doomretro.com/raw/master/releasenotes.png)

### DOOM Retro v5.7

* *DOOM Retro* is now built using v17.14.4 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.32.8*](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.8), [*SDL_mixer v2.8.1*](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.1) and [*SDL_image v2.8.8*](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.8).
* Several changes have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Very long player messages now wrap to a second line when the `vid_widescreen` CVAR is `on`.
* Minor improvements have been made to the support of [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960).
* John Romero’s [*SIGIL II*](https://romero.com/sigil) will now autoload correctly if named `SIGIL2.WAD` and placed in the `autoload` folder.
* Additional PWADs will now autoload alongside *SIGIL*, *SIGIL II* and *No Rest For The Living* by placing them in the relevant `autoload` subfolder.
* A crash no longer occurs at startup if `-autoload` or `-autoloaddir` is used on the command-line.
* Per-column lighting is now cast on sprites using the new `r_percolumnlighting` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* A bug is fixed whereby pressing <kbd><b>SPACE</b></kbd> to accelerate the intermission screen wouldn’t always work.
* Blood splats no longer spawn when lost souls are killed using the `kill` CCMD.
* These changes have been made to the support of `DEHACKED` lumps:
  * The [*MBF*](https://doomwiki.org/wiki/MBF)-compatible codepointer `A_LineEffect` now works correctly with non-repeatable line specials.
  * The [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible codepointer `A_RadiusDamage` now always inflicts damage as intended.
  * Weapons that use the [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible codepointers `A_WeaponProjectile`, `A_WeaponBulletAttack` and `A_WeaponMeleeAttack` now recoil if fired when the `weaponrecoil` CVAR is `on`.
  * A bug is fixed whereby things spawned using the [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible codepointer `A_SpawnObject` couldn’t be made translucent.
  * `Retro bits = MOREBLOOD` can now be used to spawn blood splats around a thing with a custom sprite at the start of a map when the `r_corpses_moreblood` CVAR is `on`.
  * A bug is fixed whereby the last line of `DEHACKED` lumps wasn’t being parsed.
* These changes have been made to [*ID24*](https://doomwiki.org/wiki/ID24) compatibility:
  * Flats can now be offset and rotated using line specials 2,048 to 2,056.
  * Music can now be changed using line specials 2,057 to 2,068 and 2,087 to 2,098.
  * The player’s inventory can now be reset when exiting a map using line specials 2,069 to 2,074.
  * Colormaps can now be applied to individual sectors using line specials 2,075 to 2,081.
  * These changes have been made to the support for the [`SKYDEFS`](https://doomwiki.org/wiki/SKYDEFS) lump:
    * Skies can now be scaled using `scalex`.
    * The inverted screen effect is now always applied to the sky when the player has an invulnerability power-up.
    * Fire skies defined using this lump now aren’t rendered when vanilla mode is enabled.
    * Skies can now be specified using `flatmapping`. This fixes the sky in the hole the player falls into at the end of MAP20 in *DOOM II*.
* The `IDMUS` cheat can now be used in the console when not playing a game.
* Cheats now become redacted when entered in the console before pressing <kbd><b>ENTER</b></kbd>.
* A crash no longer occurs when using the `mapstats` CCMD in the console in some rare instances.
* A bug is fixed whereby the console would open when using the mouse pointer to adjust sliders in the options menu.
* The `vid_widescreen` CVAR no longer resets to `off` at startup if `-warp` is used on the command-line.
* A bug is fixed whereby all shootable things, and not just monsters, would be hidden when using the `nomonsters` CCMD or if `-nomonsters` was used on the command-line.
* The mouse wheel can now be bound to actions other than `+nextweapon` and `+prevweapon` using the `wheelup` and `wheeldown` controls with the `bind` CCMD in the console.
* A bug is fixed whereby the `+prevweapon` action wouldn’t work if bound to a key.
* These changes have been made to the support for *Final DOOM: The Plutonia Experiment* and *TNT - Evilution*:
  * Custom map names now always display correctly when playing a PWAD.
  * PWADs placed in the autoload folder are now always automatically loaded.
* These changes have been made to [*BOOM*](https://doomwiki.org/wiki/Boom) compatibility:
  * Brightmaps specified using a `BRGHTMPS` lump no longer remove the translucency of translucent wall textures.
  * Sky textures specified using an `ANIMATED` lump now animate.
  * The offsets of scrolling floor and ceiling textures are now remembered in savegames.
  * The correct player message is now always displayed when trying to open a generalized locked door.
  * Improvements have been made to the colors used in the `WATERMAP` lump.
* A bug is fixed whereby the sky wouldn’t render correctly in some rare instances when the `weaponrecoil` CVAR was turned `on` in the console.
* The support for *Direct 3D 11* has been removed. The `vid_scaleapi` CVAR can no longer be set to `direct3d9` or `direct3d11`, but `direct3d` instead.
* The positioning of tall skies has improved in some instances when the `freelook` CVAR is `off`.
* Improvements have been made to fixing the offsets of sprites when the `r_fixspriteoffsets` CVAR is `on`.
* Playing sound effects in full as things are removed from a map can now be disabled using the new `s_fullsfx` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* The volume of MP3 and Ogg Vorbis music lumps has increased slightly.
* The player’s controller now rumbles when they land after a fall using the new `joy_rumble_fall` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* The `vid_showfps` CVAR no longer resets to `off` at startup.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, January 18, 2025

### DOOM Retro v5.6.2

* *DOOM Retro* is now built using v17.12.4 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* Changes have been made to further improve the overall stability of *DOOM Retro*.
* A bug is fixed whereby a sprite’s shadow would be positioned incorrectly in some rare instances when the `r_shadows` CVAR was `on`.
* Minor improvements have been made to the support of [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960) and [*Smooth DOOM MBF21*](https://www.doomworld.com/forum/topic/133318/).
* The `save` CCMD now always works as intended.
* The `r_detail` and `r_lowpixelsize` CVARs no longer affect the appearance of the menu’s background.
* A bug is fixed whereby colored blood would turn red after opening and closing the menu.
* Further improvements have been made to the clipping of sprites in liquid sectors when the `r_liquid_clipsprites` CVAR is `on`.
* The vertical offsets of the sprites of cacodemons, lost souls and pain elementals have improved when the `r_fixspriteoffsets` CVAR is `on`.
* Minor improvements have been made to the fuzz effect applied to spectres.
* The `vid_scaleapi` CVAR is now back to `direct3d9` by default.
* These changes have been made to the support of `DEHACKED` lumps:
  * `Retro bits = CRUSHABLE` now works as intended.
  * The `Hit points` of decorative corpses have been changed from `0` back to `1000` by default.
* A bug is fixed whereby monsters wouldn’t be alerted to the player firing their weapon in some rare instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, January 4, 2025

### DOOM Retro v5.6.1

* *DOOM Retro* now uses [*SDL v2.30.11*](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.11) and [*SDL_image v2.8.4*](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.4).
* Changes have been made to further improve the overall stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* An error is no longer displayed when trying to load the *BFG Edition* of *DOOM II: Hell On Earth*.
* Weapons that use the [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible `A_ConsumeAmmo` code pointer no longer consume ammo when fired if infinite ammo is enabled using the `infiniteammo` CCMD.
* Animated skies defined using the [*ID24*](https://doomwiki.org/wiki/ID24)-compatible [`SKYDEFS`](https://doomwiki.org/wiki/SKYDEFS) lump are now fully supported.
* Improvements have been made to the clipping of sprites in liquid sectors when the `r_liquid_clipsprites` CVAR is `on`.
* The correct `INTERPIC` lump is now always displayed when playing a PWAD for *Final DOOM: The Plutonia Experiment* or *TNT - Evilution*.
* A soft lock no longer occurs in *MAP05: The Waste Tunnels* when playing v1.666 of `DOOM2.WAD` and the `r_fixmaperrors` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, December 10, 2024

### DOOM Retro v5.6

* *DOOM Retro* is now built using v17.12.3 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.30.10*](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.10).
* Several changes have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Minor improvements have been made to text autocompleted in the console by pressing the <kbd><b>TAB</b></kbd> key.
* A bug is fixed whereby textures wouldn’t load from a PWAD in some rare instances.
* These changes have been made when a PWAD is loaded that has a custom character set:
  * The message displayed when the player finds a secret is now gold if the `secretmessages` CVAR is `on`.
  * The message displayed when using the `IDCLEV` cheat or `map` CCMD to warp to a secret map is now gold.
  * The title in the automap is now gold if the map is a secret map.
  * The title in the automap now has a drop shadow.
* A bug is fixed whereby the screen wasn’t being immediately updated after changing the `r_hud` CVAR in the console.
* The health of the player when they are dead if the `negativehealth` CVAR is `on` no longer affects voodoo dolls.
* Extensive improvements have been made to the support of [*Legacy Of Rust*](https://doomwiki.org/wiki/Legacy_of_Rust):
  * Partial support has been introduced for the new [*ID24* specification](https://doomwiki.org/wiki/ID24) so that:
    * The sky in some maps is now on fire.
    * An animated intermission screen is now displayed when finishing a map.
    * Maps are now denoted as “E*x*M*y*” rather than “MAP*xx*” in the automap.
  * Stats for killing banshees, ghouls, mindweavers, shocktroopers, tyrants and vassagos, for picking up fuel, and for firing the incinerator and calamity blade, are now all displayed by the `playerstats` CCMD.
  * A fuel can icon now appears in the widescreen HUD when the player has their incinerator or calamity blade equipped.
  * Fire from the player’s incinerator and calamity blade is now randomly mirrored and translucent.
  * *DOOM Retro’s* higher resolution status bar is now displayed, with “FUEL” replacing “CELL”, rather than the `STBAR` lump in `ID1.WAD`.
  * The help screen has been updated to include the incinerator and calamity blade.
  * The positions of shadows cast by some monsters have improved.
  * The new monsters no longer bob up and down when in a liquid sector.
  * Adaptive translucency is now applied to projectiles fired by ghouls when the `r_sprites_translucency` CVAR is `on`.
  * A bug is fixed whereby the calamity blade was inflicting too much damage.
* A bug is fixed whereby [Andrew Hulshult’s](https://www.hulshult.com/) *IDKFA* soundtrack wasn’t being played in some maps when [`extras.wad`](https://doomwiki.org/wiki/Extras.wad) was autoloaded.
* The volume of MP3 and Ogg Vorbis music lumps has increased slightly.
* Minor improvements have been made to the support of [*Master Levels*](https://doomwiki.org/wiki/Master_Levels_for_Doom_II), [*SIGIL*](https://romero.com/sigil),  [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest), [*Chex Quest 2*](https://doomwiki.org/wiki/Chex_Quest#Chex_Quest_2), [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens) and [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960).
* These changes have been made to the support of `MAPINFO` lumps:
  * The order in which multiple `MAPINFO` lumps are loaded is now correct.
  * `EPISODE` is now parsed in more instances.
  * Up to 10 episodes can now be specified using `EPISODE` and will display correctly in the episode menu.
* These changes have been made to the support of `DEHACKED` lumps:
  * A bug is fixed whereby a line wouldn’t be parsed in some instances if the previous line was blank.
  * If the fists or chainsaw are replaced using `WEAPON`, that weapon can now consume ammo.
  * Blood splats are no longer spawned if the states of blood have been changed.
* The [fake contrast](https://doomwiki.org/wiki/Fake_contrast) applied to walls at different angles is now more varied.
* A bug is fixed whereby the player’s aim was slightly lower when the `freelook` CVAR was `on` and the `r_screensize` CVAR was `8`.
* These changes have been made to the alternate widescreen HUD:
  * The weapon silhouette now accurately reflects the pickup sprite for each weapon.
  * Certain elements have now either shifted position, increased in size and/or cast a slight shadow.
  * The health bar no longer flashes white while it is red and the player picks up some health.
* Pressing the <kbd><b>PRINTSCREEN</b></kbd> key no longer advances the finale.
* The mouse wheel can no longer be used to advance *DOOM II’s* cast sequence.
* The amount of ammo the player has is no longer momentarily translucent in the widescreen HUD at the start of each map if the `animatedstats` CVAR is `on`.
* These changes have been made to the help screen:
  * A bug is fixed whereby the help screen wouldn’t be fully displayed when the `vid_widescreen` CVAR was `off`.
  * The `TITLEPIC` lump is now always used as the help screen’s background when not playing a game.
  * The help screen now displays better in some PWADs that have custom `PLAYPAL` lumps.
* Using the mouse pointer to move the scroll bar in the console is now more responsive when the `m_pointer` CVAR is `on`.
* Drop shadows are now always displayed correctly for all elements on the intermission screen.
* A bug is fixed whereby the wrong `INTERPIC` lump would be displayed in some instances.
* The `vid_widescreen` CVAR no longer resets to `off` at startup if `-nosplash` is used on the command-line.
* A bug is fixed whereby the z-coordinates of things bobbing in liquid sectors were altered upon loading a savegame when the `r_liquid_bob` CVAR was `on`.
* Very short sprites are no longer clipped in liquid sectors when the `r_liquid_clipsprites` CVAR is `on`.
* These changes have been made to the automap:
  * Aspect ratio correction is now applied. This feature can be toggled off using the new `am_correctaspectratio` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
  * In the original *DOOM*, if the player moved around while the automap was open, walls seen by the player for the first time weren’t mapped. This behavior can now be restored by disabling the new `am_dynamic` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
  * The automap now displays correctly when zooming out in very large maps.
  * The grid now always covers the entire screen when the `am_rotatemode` CVAR is `on`.
  * A bug is fixed whereby the player’s path would stop being drawn after the player teleported and the `am_rotatemode` CVAR was `off`.
  * A bug is also fixed whereby the player’s path would display incorrectly at the start of a map in some instances.
* The fuzz effect from spectres and the partial invisibility power-up now:
  * Doesn’t cause the status bar to bleed into the player’s view while the console is open.
  * Freezes along with everything else when freeze mode is enabled.
* Flying monsters no longer nudge corpses beneath them when the `r_corpses_nudge` CVAR is `on`.
* Scrolling textures are now smoother when the `vid_capfps` CVAR is a value other than `35`.
* The interpolation of moving floors and ceilings is now smoother in some instances when the `vid_capfps` CVAR is a value other than `35`.
* Semi-colons are now parsed correctly when using the `alias` CCMD in the console.
* All actions can now be bound to a second key using the `bind` CCMD in the console.
* If the key bound to the `+followmode` action is bound to another action, pressing that key now works outside of the automap.
* Maps that use *BOOM’s* `WATERMAP` lump are now supported.
* A bug is fixed whereby the next map wouldn’t load after displaying a text screen during intermission in some rare instances.
* The `episode` CVAR is now reset to its default if the WAD loaded in the launcher is different than last time.
* Minor improvements have been made to the support for [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible WADs.
* A bug is fixed whereby Wolfenstein SS were replaced by zombiemen at the start of each map in some rare instances.
* The behavior of lost souls spawned by pain elementals is now consistent with the original *DOOM II*.
* The drop shadows of menu items displayed using a `DBIGFONT` lump no longer overlap.
* Torched trees no longer bob when in liquid sectors.
* The player and monsters now traverse liquid sectors at different heights better.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, August 11, 2024

### DOOM Retro v5.5.1

* *DOOM Retro* now uses [*SDL v2.30.6*](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.6).
* Minor changes have been made to text that is output to the console.
* Player messages that don’t fit on the screen when the `vid_widescreen` CVAR is `off` now wrap to a second line rather than being truncated with an ellipsis.
* These changes have been made when loading a savegame:
  * The date and time a savegame was saved, and whether it has changed the skill level, is now displayed in the console.
  * The `episode` and `skilllevel` CVARs are now updated as necessary.
  * A bug is fixed whereby certain floor heights, ceiling heights and wall texture offsets could be slightly off after loading a savegame in some rare instances.
* A crash no longer occurs when killing something that has both an `Exploding frame` and `Bits = NOBLOOD` specified in a `DEHACKED` lump, and the `r_blood_gibs` CVAR is `on`.
* These changes have been made to the support of `MAPINFO` lumps:
  * A bug is fixed whereby a `MAPINFO` lump wouldn’t be parsed at all in some instances.
  * Actions specified using `BOSSACTION` now always trigger as intended.
  * `INTERTEXT = CLEAR` is now parsed.
  * Up to 9 episodes can now be specified using `EPISODE` and will display correctly in the episode menu.
* A bug is fixed whereby the player wouldn’t telefrag monsters.
* The player now equips their fists if they pick up a berserk power-up and they already have one.
* Player and voodoo doll corpses can now trigger line specials again.
* A bug is fixed whereby the mouse pointer couldn’t be used to move the text caret or scroll bar in the console when the `m_pointer` CVAR was `on` and the `vid_widescreen` CVAR was `off`.
* These changes have been made in response to id Software’s [rerelease of *DOOM* and *DOOM II*](https://store.steampowered.com/app/2280/DOOM__DOOM_II/) during [QuakeCon](https://quakecon.bethesda.net/) on August 8, 2024:
  * When *DOOM Retro* is run for the first time, the WAD launcher will now also look for the IWADs included with installations of this release.
  * A bug is fixed whereby several wrong lumps would be displayed when loading a PWAD alongside the new *DOOM* or *DOOM II* IWADs.
  * Several compatibility fixes have been implemented for [*Legacy Of Rust*](https://doomwiki.org/wiki/Legacy_of_Rust).
  * If `extras.wad` is found alongside `DOOM.WAD` or `DOOM2.WAD`, it is now autoloaded, and [Andrew Hulshult’s](https://www.hulshult.com/) *IDKFA* soundtrack is played instead of the regular MIDI music.
* Minor improvements have been made to the support of [*REKKR*](https://www.doomworld.com/idgames/levels/doom/megawads/rekkr) and [*REKKR: Sunken Land*](https://store.steampowered.com/app/1715690/REKKR_Sunken_Land/).
* The `STTMINUS` lump used in the status bar and widescreen HUD when the `negativehealth` CVAR is `on` and the player is dead, is now positioned correctly in instances where its vertical offset is missing.
* A bug is fixed whereby a string CVAR couldn’t be changed in the console to the same string but with a different case.
* The length of the `playername` CVAR is now limited to 16 characters.
* Blood splats now move with everything else on *BOOM*-compatible scrolling floors.
* If the `+alwaysrun` action is bound to the <kbd>CAPSLOCK</kbd> key, and the `alwaysrun` CVAR is `on`, the <kbd>CAPSLOCK</kbd> key is now toggled off when the console is opened, and then toggled back on when it closes.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, July 26, 2024

### DOOM Retro v5.5

* *DOOM Retro* is now built using v17.10.5 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.30.5*](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.5).
* Several changes have been made to further improve the overall performance and stability of *DOOM Retro*.
* Several changes have been made to text that is output to the console.
* The mouse no longer has any effect on *DOOM Retro’s* window when it doesn’t have focus.
* Changing the `r_mirroredweapons` CVAR no longer affects the random colors of marine corpses when the `r_corpses_color` CVAR is `on`.
* These changes have been made to the support of `MAPINFO` lumps:
  * A bug is fixed whereby map names wouldn’t be parsed correctly in some instances.
  * `EPISODE` is now parsed correctly in more instances.
  * Newline characters in `INTERTEXT` and `INTERTEXTSECRET` strings are now parsed.
* These changes have been made to the support of `DEHACKED` lumps:
  * The values specified using `Initial health` and `Initial bullets` will now be used at the start of each map if the `pistolstart` CCMD or `-pistolstart` command-line parameter are used.
  * A bug is fixed whereby the health of spiderdemons would always be 3,000 even if a value was specified using `Hit points`.
* These changes have been made to the support for controllers:
  * Improvements have been made to the rumbling of controllers when the player uses certain weapons and the `joy_rumble_weapons` CVAR is `on`.
  * A bug is fixed whereby the player would continually turn to their right if the `joy_analog` CVAR was `off` (including when vanilla mode was enabled).
  * Hotplugging controllers now always works.
  * Pressing the left thumbstick of a controller is no longer bound to the `+alwaysrun` action by default.
  * Controllers no longer rumble if the mouse or keyboard are being used by the player instead.
* The aspect ratio of the display when in widescreen mode can now be forced by changing the new `vid_aspectratio` CVAR from its default `auto` to `16:9`, `16:10`, `21:9` or `32:9`.
* Pressing the <kbd><b>ENTER</b></kbd> key to close the help screen no longer causes the previous player message to be displayed.
* The `save` and `load` CCMDs can now be used to save and load games by specifying numbers `1` to `8`.
* The player’s currently equipped weapon is now translucent when they have a partial invisibility power-up and the `r_textures` CVAR is `off`.
* A bug is fixed, present in the original *DOOM*, whereby sprites wouldn’t be rendered if the sector they were in wasn’t in the player’s line of sight.
* Sprites also no longer disappear when drawn against the left and right edges of ultra-wide displays.
* A small amount of blood is now sprayed when a monster is gibbed. This can be disabled using the new `r_blood_gibs` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* The `playername` CVAR is now `“”` rather than `“you”` by default.
* A bug is fixed whereby the ammo the player had for their equipped weapon wasn’t displayed in the widescreen HUD while they were dead.
* The power-up bar is now displayed in the alternate widescreen HUD when the player enters the `IDBEHOLDL` cheat.
* These changes have been made to the menu:
  * Minor improvements have been made to the highlighting of items selected in the menu.
  * The highlighting of items selected in the menu can now be disabled using the new `menuhighlight` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
  * Shadows cast by items in the menu can now be disabled using the new `menushadow` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
  * The `episode`, `expansion` and `skilllevel` CVARs are now always updated whenever the selected episode, expansion or skill level are changed in the menu.
  * `DBIGFONT` lumps are now supported. This will allow more menu items to be displayed in the correct font, and `CONSOLE...` to be displayed in the options menu, in some instances.
  * These changes have been made to the load and save game menus:
    * The text caret in the save game menu is now larger and flashes quicker.
    * A bug is fixed whereby the text caret could become momentarily stuck at the end of a truncated savegame description in some instances.
    * To be consistent with every other menu, pressing a key that corresponds with the first letter of a savegame description now navigates to that savegame slot.
* The sound effect heard when opening and closing the console by pressing the <kbd><b>~</b></kbd> key is now slightly louder.
* Further improvements have been made to rendering liquid sectors when the `r_liquid_bob` CVAR is `on`.
* The screen’s brightness can now be adjusted using the new `vid_brightness` CVAR. This CVAR can be between `-100%` and `100%`, and is `0%` by default and when vanilla mode is enabled.
* The screen’s contrast can now be adjusted using the new `vid_contrast` CVAR. This CVAR can be between `-100%` and `100%`, and is `0%` by default and when vanilla mode is enabled.
* The screen’s red, green and blue levels can now be adjusted using the new `vid_red`, `vid_green` and `vid_blue` CVARs. These CVARs can be between `-100%` and `100%`, and are `0%` by default and when vanilla mode is enabled.
* The `r_saturation` CVAR is renamed `vid_saturation`, is now a value between `-100%` and `100%` and is `0%` by default.
* The `vid_scaleapi` CVAR is now `direct3d11` by default.
* The map title in the automap is now always positioned correctly when the `am_external` CVAR is `on`.
* Additional brightness can now be applied to all of the lighting in the current map using the new `r_levelbrightness` CVAR. This CVAR can be between `0%` and `100%`, and is `0%` by default and when vanilla mode is enabled.
* The `infiniteammo` and `regenhealth` CCMDs can no longer be entered when a game isn’t being played.
* The color of the bottom edge of the console now changes to reflect the color of the digits in the status bar, (or in some cases, the text in the menu), if the relevant lumps have been replaced by the currently loaded PWAD.
* The mouse pointer may now be used to move the scroll bar in the console when the `m_pointer` CVAR is `on`.
* Minor changes have been made to text in the status bar and alternate widescreen HUD.
* The `r_shake_damage` CVAR is now a value of `on` or `off` and is `on` by default. Improvements have been made to the shake effect when this CVAR is `on`.
* A confirmation message is now displayed when entering the `endgame` CCMD in the console.
* A bug is fixed whereby a controller’s left thumbstick would be too sensitive while the player was running.
* A minor improvement has been made to the rumble of controllers when the player picks something up and the `joy_rumble_pickup` CVAR is `on`.
* A bug is fixed whereby advancing the intermission screen wouldn’t work in some instances.
* A crash no longer occurs if a texture is missing a patch.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, May 2, 2024

### DOOM Retro v5.4

* *DOOM Retro* is now built using v17.9.6 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.30.3*](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.3).
* Minor changes have been made to further improve the overall performance and stability of *DOOM Retro*.
* Several changes have been made to text that is output to the console.
* A slightly brighter color is now chosen for the surrounding pillarboxes when displaying certain lumps from PWADs that are only 320 pixels wide and the `vid_widescreen` CVAR is `on`.
* A bug is fixed whereby grouping wasn’t being applied to the previous player message when displayed by pressing the <kbd><b>ENTER</b></kbd> key if both the `messages` and `groupmessages` CVARs were `on`.
* Minor improvements have been made to certain animations on the intermission screen.
* The status bar now shows when the player has picked up both a keycard and a skull key of the same color by introducing support for *BOOM’s* `STKEYS6`, `STKEYS7` and `STKEYS8` lumps.
* Improvements have been made to drawing thing triangles in the automap when the `IDDT` cheat has been entered.
* The underscores of the message displayed when the `IDBEHOLD` cheat has been entered now always line up.
* The shadow cast by the status bar in the automap is now low detail when the `r_detail` CVAR is `low`.
* The color of the player stats shown in the automap when the `am_playerstats` CVAR is `on` can now be changed using the new `am_playerstatscolor` CVAR, which is `4` by default.
* These changes have been made to the support of `MAPINFO` lumps:
  * Scrolling skies are now supported if a value is specified with `SKY1`.
  * `ALLOWMONSTERTELEFRAGS = 0` now works on all maps.
  * A bug is fixed whereby map names wouldn’t be used in some instances.
* Minor improvements have been made to the parsing of `DEHACKED` lumps.
* The status bar and widescreen HUD are now always correctly updated when the `give` or `take` CCMDs are used to give or take keycards and skull keys to or from the player. The order in which they are given has also improved.
* The `DSSAWUP` sound effect is no longer heard in its entirety when the player equips their chainsaw.
* If a sound effect lump in a PWAD is 0 bytes in size, now rather than playing the IWAD’s lump it replaces, nothing is played.
* Explosions now cast shadows if the `r_shadows` CVAR is `on`.
* These changes have been made to intermission text screens:
  * A sound effect is now played when a key is pressed to advance the text.
  * A crash no longer occurs if the text is too long.
* The <kbd><b>F1</b></kbd> key no longer interrupts the splash screen if pressed.
* The console can no longer be opened while a menu confirmation message is displayed.
* Further improvements have been made to the position of player messages in some instances.
* These changes have been made to the load and save game menus:
  * If any <code>STCFN<i>xxx</i></code> lumps are present in a PWAD, the savegame descriptions are now correctly positioned vertically.
  * The correct savegame description is now always displayed in the confirmation message when pressing <kbd><b>DEL</b></kbd> to delete a savegame.
* The `mouselook` CVAR and the `+mouselook` action have been renamed `freelook` and `+freelook`.
* These changes have been made when using the `resetall` CCMD:
  * A bug is fixed whereby certain actions bound to a mouse button weren’t being reset.
  * The automap no longer closes if it was open.
* A bug is fixed whereby the sky wouldn’t be stretched if the `+freelook` action was bound to a button on a controller.
* The text in the console has been brought in slightly from the left and right edges of the screen when the `vid_widescreen` CVAR is `on`, and even more so on ultra-wide displays.
* Improvements have been made to the position of some elements of the alternate widescreen HUD.
* The game may now be ended during intermission by either using the `endgame` CCMD in the console or selecting “End Game” in the options menu.
* The number of keycards and skull keys the player picks up is now displayed by the `playerstats` CCMD.
* The translucency of the trail of smoke behind rockets fired by the player and cyberdemons can now be toggled separately from other sprites by using the new `r_rockettrails_translucency` CVAR. This CVAR is `on` by default and `off` when vanilla mode is enabled.
* Boss actions now occur when using the `kill` CCMD in the console.
* Improvements have been made to displaying the disk icon when the `r_diskicon` CVAR is `on`.
* Improvements have been made to the contents of files created using the `condump` CCMD.
* Liquid sectors are now rendered correctly when their floor height is in line with the player’s view height and the `r_liquid_bob` CVAR is `on`.
* The `help` CCMD has been renamed `wiki`.
* Player messages no longer cast shadows when vanilla mode is enabled.
* An infinite amount of ammo for all of the player’s weapons can now be enabled by using the new `infiniteammo` CCMD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, March 3, 2024

### DOOM Retro v5.3

* *DOOM Retro* is now built using v17.9.2 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.30.0*](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.0) and [*SDL_mixer v2.8.0*](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.0).
* Changes have been made to *DOOM Retro’s* splash screen.
* Several changes have been made to further improve the overall performance and stability of *DOOM Retro*.
* Support has been added for maps with [compressed, extended nodes](https://zdoom.org/wiki/Node#ZDoom_extended_nodes).
* A warning is now displayed in the console at startup if a lump in a PWAD is an unsupported PNG image, since trying to display it would cause *DOOM Retro* to crash.
* Minor changes have been made to text that is output to the console.
* Improvements have been made when the player moves using a controller.
* A bug is fixed whereby corpses weren’t being nudged whenever the player or a monster walked over them and the `r_corpses_nudge` CVAR was `on`.
* The blood splats spawned around a corpse at the start of each map when the `r_corpses_moreblood` CVAR is `on` are no longer spawned outside of the map if the corpse happens to be too close to a wall.
* These changes have been made when entering the `IDCLEV` cheat:
  * If it is used to restart the current map, the player message displayed is now always correct.
  * If invalid parameters are entered, such as if `IDCLEV01` is entered while playing *DOOM* instead of *DOOM II*, the player no longer equips their fists because the <kbd>1</kbd> key was pressed.
* The `IDMUS` cheat now works as intended.
* The thing triangles of spectres in the automap are now translucent when the `IDDT` cheat has been entered.
* Improvements have been made to parsing custom monster names in `DEHACKED` lumps.
* These changes have been made to the support of `MAPINFO` lumps:
  * `BOSSACTION` now always works as intended.
  * `NEXT` and `SECRETNEXT` now always work as intended.
  * Custom episodes greater than episode 4 now work as intended when playing *DOOM*.
  * A crash no longer occurs during the finale when playing *DOOM II* and `ENDGAME = TRUE` is present.
* If the music lumps `D_E4M1` to `D_E4M9` are found in a PWAD, they are now heard when playing maps in episode 4 of *DOOM*.
* These changes have been made to the weapon silhouettes in the alternate widescreen HUD:
  * The `DRHUDWP0` lump can now be changed in a PWAD to display a weapon silhouette when the player has their fists equipped.
  * If any of the player weapon sprites have changed in a PWAD, the weapon silhouettes can now be displayed if the `DRHUDWP0` to `DRHUDWP8` lumps are also changed.
* Several improvements have been made to the position, translucency and truncation of player messages and the current map’s title in the automap.
* When playing *DOOM (Shareware)*, files no longer attempt to autoload from the `autoload` folder during startup.
* A bug is fixed whereby the presence of `SIGIL_SHREDS.WAD` in the `autoload` folder would cause a crash in some instances.
* Restoring a lesser known feature from the original *DOOM*, pressing the <kbd><b>ENTER</b></kbd> key now displays the previous player message if the `messages` CVAR is `on`. Also, pressing the <kbd><b>ENTER</b></kbd> key while a player message is displayed will now hide that message.
* Further improvements have been made to the support of [*Freedoom: Phase 1*](https://freedoom.github.io/) and [*Freedoom: Phase 2*](https://freedoom.github.io/).
* Solid walls now always clip correctly against the left and top edges of the automap.
* The puffs of smoke that trail behind rockets fired by the player and cyberdemons when the `r_rockettrails` CVAR is `on` are now randomly mirrored.
* The `spawn` CCMD no longer spawns something if the ceiling in front of the player is too low.
* The stairs in *MAP30: Last Call* of *Final DOOM: TNT - Evilution* now rise up to their correct heights.
* A bug is fixed whereby the player’s view angle would be slightly off after they teleported.
* Both the player and monsters can no longer move under corpses hanging from the ceiling if the `infiniteheight` CVAR is `on`.
* The number of power-ups the player picks up is now displayed by the `playerstats` CCMD.
* The use of `DEHACKED` and `MAPINFO` lumps is now displayed by the `mapstats` CCMD.
* A bug is fixed whereby the screen would glitch slightly as the player exited a map if the `vid_scaleapi` CVAR was `direct3d11`.
* The `vid_scaleapi` CVAR is now `direct3d9` by default.
* Minor improvements have been made to the position and translucency of several elements on the intermission screen.
* The total amount of time played is now displayed in the console each time the player exits a map.
* The amount the screen shakes when the player receives damage has been reduced if the `r_shake_damage` CVAR is greater than `0%`.
* These changes have been made when using the `kill` CCMD in the console:
  * The distance a monster slides upon their death, and the amount of blood splats that are spawned, are now consistent regardless of the size of the monster.
  * Friendly monsters no longer affect the total number of monsters killed in the current map.
  * A bug is fixed whereby invalid obituaries would be displayed in some instances.
* Entering `explode barrels` in the console now works as intended.
* Entering `explode missiles` in the console now also explodes any projectiles that the player has fired.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 12, 2024

### DOOM Retro v5.2.1

* *DOOM Retro* is now built using v17.8.4 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses *Direct3D 11* by default rather than *Direct3D 9* to scale every frame on the screen, resulting in a considerable boost in performance. The `vid_scaleapi` CVAR can now be `direct3d9`, `direct3d11`, `opengl` or `software`.
* IT, S3M, XM and MOD music lumps now play again.
* These changes have been made in the automap:
  * The player’s path is now drawn correctly if the `am_path` CVAR is `on` and they have just teleported.
  * A controller’s left and right shoulder buttons can now be held down to zoom in and out.
  * The subtle shadow cast by the status bar is now only visible when the `am_backcolor` CVAR is its default of `0`.
  * Minor improvements have been made to drawing marks added by pressing the <kbd><b>M</b></kbd> key.
* The `con_obituaries` CVAR has been renamed to just `obituaries`.
* When the `r_rockettrails` CVAR is `on`:
  * A bug is fixed whereby no smoke was trailing behind rockets fired by the player and cyberdemons.
  * Smoke trailing behind rockets fired by cyberdemons won’t be spawned if certain states or sprites have been changed in a `DEHACKED` lump.
* A sound effect is now played when using a control that has been bound to the `toggle` CCMD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 5, 2024

### DOOM Retro v5.2

* *DOOM Retro* now uses [*SDL_image v2.8.2*](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.2).
* Minor optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* If the player dies, their obituary is now displayed in the console in red when the `con_obituaries` CVAR is `on`, and also as a player message when the `messages` CVAR is `on`.
* A bug is fixed whereby Buckethead’s music wouldn’t be autoloaded when `SIGIL_SHREDS.WAD` was present.
* The z-coordinate displayed by the `IDMYPOS` cheat is now correct when the automap is open and the `am_followmode` CVAR is `off`.
* These improvements have been made to the support for John Romero’s [*SIGIL II*](https://romero.com/sigil):
  * A warning is now displayed in the console when attempting to load *SIGIL II* extracted from the official *DOOM* port. Please download *SIGIL II* from [here](https://romero.com/sigil) instead.
  * Par times are now displayed on the intermission screen.
  * The correct intermission music is now played when both *SIGIL* and *SIGIL II* are loaded.
  * The `IDMUS` cheat can now be used to change to *SIGIL II’s* music if it is available.
* Right-clicking the mouse in the menu now hides the mouse pointer if the `m_pointer` CVAR is `on`.
* Minor improvements have been made to the help screen opened by pressing the <kbd><b>F1</b></kbd> key.
* A bug is fixed whereby the width of monsters was being miscalculated in some rare instances, which in turn could cause nearby line specials not to trigger as intended.
* These improvements have been made to the parsing of `DEHACKED` lumps:
  * All things declared now have their sprites clipped in liquid sectors if the `r_liquid_clipsprites` CVAR is `on`.
  * All monsters declared now cast a shadow if the `r_shadows` CVAR is `on`.
  * If a name is specified in parentheses when declaring a thing, it is now used in obituaries when the `con_obituaries` CVAR is `on`.
  * The smoke that trails rockets fired by cyberdemons when the `r_rockettrails` CVAR is `on` is no longer spawned if certain states or sprites have been changed.
* The `mapstats` CCMD now correctly indicates if a map is <i>`BOOM`</i>, <i>`MBF`</i> or <i>`MBF21`</i> compatible.
* Improvements have been made to the console’s background.
* If map titles are obtained from a `MAPINFO` lump:
  * The `maplist` CCMD now lists them correctly.
  * When exiting a map and the `autosave` CVAR is `on`, the description of the current savegame is now updated with the title of the next map.
* The `.wad` file extension no longer needs to be included when using the `-iwad` or `-file` command-line parameters.
* The `weapon` CVAR can no longer be changed while the player is dead.
* [*Helper dogs*](https://doomwiki.org/wiki/Helper_dog) spawned using the `spawn` CCMD in the console are now friendly by default. Use `spawn unfriendly dog` to spawn a dog that will attack the player.
* The player may now use the term `unfriendly` to refer to monsters that will attack the player when entering the `kill`, `name`, `resurrect` and `spawn` CCMDs in the console.
* Using the `kill` CCMD with a value specifying a type of monster no longer thrusts those monsters away from the player.
* A bug is fixed whereby midtextures weren’t being clipped in some rare instances.
* The `thinglist` CCMD now displays the angle and flags of every thing in the current map.
* Opening the console while entering a savegame description in the save game menu now works correctly.
* When deleting a savegame in the save or load game menus using the <kbd><b>DEL</b></kbd> key, the closest remaining savegame in the menu is now selected.
* A crash no longer occurs if a map’s title needs to be truncated in the automap.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, December 16, 2023

### DOOM Retro v5.1.3

* *DOOM Retro* now uses [*SDL_image v2.8.1*](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.1).
* Minor changes have been made to text that is output to the console.
* The MD5 hash value of the current map’s WAD is now displayed by the `mapstats` CCMD.
* Further improvements have been made to the parsing of `bossaction` in `MAPINFO` lumps.
* A bug is fixed whereby selecting an episode in the menu wouldn’t change the `episode` CVAR in some instances.
* *SIGIL* is now still automatically loaded if found and *SIGIL II* is manually loaded.
* Improvements have been made to recognizing the *SIGIL* and *SIGIL II* WAD files if they have been renamed.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, December 14, 2023

### DOOM Retro v5.1.2

* Minor changes have been made to text that is output to the console.
* These improvements have been made to the support for John Romero’s [*SIGIL II*](https://romero.com/sigil):
  * “SIGIL” can no longer be selected in the episode menu if *SIGIL* isn’t loaded but *SIGIL II* is.
  * The Spider Mastermind’s health in *E6M8: Abyss Of Despair* is now correct.
  * [Thorr’s](https://music.apple.com/us/artist/thorr/1325613495) music is now louder.
* If the current map is from a supported WAD, its author is now displayed by the `mapstats` CCMD.
* A bug is fixed whereby the corpses of monsters could remain solid, blocking the player’s path, in some rare instances.
* Minor improvements have been made to text autocompleted in the console by pressing the <kbd><b>TAB</b></kbd> key.
* Minor improvements have been made to the parsing of `enterpic` in `MAPINFO` lumps.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, December 11, 2023

### DOOM Retro v5.1.1

* These improvements have been made to the support for John Romero’s [*SIGIL II*](https://romero.com/sigil):
  * The registered version of *SIGIL II* is now supported.
  * The correct sky is now displayed.
  * The correct background is now displayed on the intermission screen, and a crash no longer occurs if *SIGIL* hasn’t also been loaded.
  * Improvements have been made to the “SIGIL II” text in the episode menu.
  * The player now warps to the correct map after finishing the secret map.
  * The titles of [Thorr’s](https://music.apple.com/us/artist/thorr/1325613495) music are now displayed by the `mapstats` CCMD when playing the registered version of *SIGIL II*.
* Minor improvements have been made to the parsing of `bossaction` in `MAPINFO` lumps.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, December 10, 2023

### DOOM Retro v5.1

* *DOOM Retro* is now built using v17.8.3 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.28.5*](https://github.com/libsdl-org/SDL/releases/tag/release-2.28.5) and [*SDL_image v2.8.0*](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.0).
* Support has been added for John Romero’s newly released [*SIGIL II*](https://romero.com/sigil).
* Several changes have been made to text that is output to the console.
* The automap is now also shown in low detail when the `r_detail` CVAR is `low`.
* A bug is fixed whereby the effects of changing a color CVAR for the automap wouldn’t be immediate.
* Explosions from rockets fired by the player and cyberdemons are now randomly mirrored.
* These changes have been made when vanilla mode is enabled:
  * Extra blood is no longer spawned when a monster is injured.
  * Blood and bullet puffs are no longer randomly mirrored.
* Monsters now only become non-solid at the end of their death sequence rather than at the start.
* When the `r_shadows` CVAR is `on`, if a monster is fullbright when they attack, the shadow they cast now momentarily becomes slightly lighter.
* The player’s crosshair now displays correctly when the `crosshair` CVAR is `cross` and the `r_hud_translucency` CVAR is `off`.
* Minor improvements have been made to the parsing of `DEHACKED`, `MAPINFO`, `MUSINFO` and `BRGHTMPS` lumps.
* The `-dog` parameter, as well as `-dogs 1` to `-dogs 8`, can now be used on the command-line to spawn one or more [*MBF*-compatible helper dogs](https://doomwiki.org/wiki/Helper_dog) at the start of the game. Those dogs that survive each map will then follow the player into the next.
* These changes have been made when pressing the <kbd><b>TAB</b></kbd> key to autocomplete text in the console:
  * The case of the autocompleted text now always matches the text that’s already been entered.
  * Any text to the left of a semi-colon is no longer cleared.
* The border of the menu’s background is no longer displayed if the `vid_fullscreen` CVAR is `off` and the `vid_widescreen` CVAR is `on`.
* The spin of the player’s view in the menu’s background can now be disabled by changing the new `menuspin` CVAR, which is `on` by default.
* The help screen can now be shown when the menu is open by pressing the <kbd><b>F1</b></kbd> key.
* The window’s caption now includes the current episode or expansion when playing a game.
* *DOOM Retro* no longer remains paused once its window regains focus.
* A bug is fixed whereby the main window would lose focus and the splash screen would be corrupted if an external automap was successfully created at startup because the `am_external` CVAR was `on`.
* When the `animatedstats` CVAR is `on`:
  * The player’s health now animates correctly if they are telefragged.
  * The player’s health, armor and ammo now animate correctly when loading a savegame.
* A bug is fixed whereby numbers in the alternate widescreen HUD could be positioned incorrectly in some rare instances.
* Minor improvements have been made to the support of [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest), [*Chex Quest 2*](https://doomwiki.org/wiki/Chex_Quest#Chex_Quest_2), [*Harmony Compatible*](https://www.doomworld.com/idgames/levels/doom2/Ports/g-i/harmonyc) and [*REKKR*](https://www.doomworld.com/idgames/levels/doom/megawads/rekkr).
* A bug is fixed whereby monsters would respawn only once when playing on *Nightmare!* or if the `respawnmonsters` CCMD was used.
* The “Map explored” stat displayed by the `playerstats` CCMD is now accurate.
* The `mapstats` CCMD now shows the number of voodoo dolls in the current map.
* The `play` CCMD now also accepts a music title as its value, such as `atdoomsgate` or `runningfromevil`.
* A bug is fixed whereby screenshots taken by pressing the <kbd><b>PRINTSCREEN</b></kbd> key would become corrupted in some rare instances.
* Actions may now be bound to the numeric keypad using the `bind` CCMD with a value `numpad0` to `numpad9`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, October 21, 2023

### DOOM Retro v5.0.7

* A crash no longer occurs when a spectre touches the bottom of the screen in some instances.
* Minor changes have been made to text that is output to the console.
* Minor improvements have been made to the border of the menu’s background.
* The help screen is now shown when pressing the <kbd><b>F1</b></kbd> key while playing the registered version of *DOOM*.
* A bug is fixed whereby stairs rising up from the floor wouldn’t rise to their correct height in some instances.
* Changing the `vid_capfps` CVAR to `35` now caps the framerate at 35 FPS again.
* If the `vid_capfps` CVAR is `off` or greater than `60`, and the `vid_vsync` CVAR is `off`, the framerate is now capped at 60 FPS when on the title screen, in the menu, in the console, or the game is paused.
* The frames per second are now displayed in the menu when the `vid_showfps` CVAR is `on`.
* Minor improvements have been made to determining when to lower the player’s view if the `r_liquid_lowerview` CVAR is `on`.
* A bug is fixed whereby entering `kill monsters` in the console could then affect splash damage to the player.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, October 14, 2023

### DOOM Retro v5.0.6

* *DOOM Retro* is now built using v17.7.5 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses *Direct3D 11* rather than *Direct3D 9* when the `vid_scaleapi` CVAR is `direct3d`, resulting in a considerable boost in performance.
* The handling of errors caused by [*SDL*](https://www.libsdl.org/) has improved.
* The capping of the framerate when the `vid_capfps` CVAR is not `35` has improved.
* Minor changes have been made to text that is output to the console.
* The <kbd><b>ALT</b></kbd> key can no longer be used to open the menu from the title screen.
* Item and teleport fogs no longer appear when using the `spawn` CCMD in the console while freeze mode is on.
* A crash no longer occurs when the player explodes a barrel using their BFG-9000 in some instances and the `con_obituaries` CVAR is `on`.
* Minor improvements have been made to the support for `DEHACKED` lumps.
* The “Games loaded” stat now updates correctly in the output of the `playerstats` CCMD.
* A bug is fixed whereby PWADs wouldn’t autoload when placed in the `autoload` folder.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, October 7, 2023

### DOOM Retro v5.0.5

* *DOOM Retro* now uses [*SDL v2.28.4*](https://github.com/libsdl-org/SDL/releases/tag/release-2.28.4).
* Minor changes have been made to text that is output to the console.
* Minor changes have been made to the branding in the console.
* The `if` CCMD works with boolean CVARs again.
* Blood splats are now visible in the top half of the player’s view if not completely obscured by the map’s geometry.
* If the player enters `kill player` in the console while buddha mode is enabled, their health is now reduced to 1%.
* `SIGIL.WAD` is no longer autoloaded if a PWAD is loaded that contains an `E1M1` lump.
* If `SIGIL.WAD` or `NERVE.WAD` have been placed in the `autoload` folder:
  * They are now loaded before any other PWADs in the `autoload` folder,
  * But not if the PWADs already loaded contain any conflicting lumps.
* `NERVE.WAD` now loads correctly if specified using the `-file` parameter on the command-line.
* Minor improvements have been made to the highlight effect in the menu in some rare instances.
* A bug is fixed whereby music changer objects would no longer work once loading a savegame.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 30, 2023

### DOOM Retro v5.0.4

* These changes have been made when gibbing a corpse and the `r_corpses_gib` CVAR is `on`:
  * Gibbing already gibbed corpses now always works as intended.
  * Certain gibbed corpses spawned at the start of the map can now be gibbed further.
  * Obituaries are no longer displayed in the console for when a corpse is gibbed.
  * Gibbed corpses are now only randomly mirrored if the `r_corpses_mirrored` CVAR is also `on`.
* The intermission and finale screens now pause if *DOOM Retro’s* window loses focus.
* The player no longer continues to move across the map if moving when the automap is opened and the `am_followmode` CVAR is `off`.
* A bug is fixed whereby the amount of cells the player had would be brighter than usual in the widescreen HUD when their BFG-9000 was equipped.
* The player’s view is no longer affected when walking over certain linedefs in liquid sectors and the `r_liquid_lowerview` CVAR is `on`.
* Buckethead’s music is now heard when playing *SIGIL* if `SIGIL_SHREDS.WAD` happens to be loaded before `SIGIL.WAD` at startup.
* Obituaries displayed in the console when the player explodes a barrel using their BFG-9000 are now always correct.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, September 24, 2023

### DOOM Retro v5.0.3

* Further improvements have been made when toggling widescreen using the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys while the menu is open.
* Minor improvements have been made to the help screen shown when pressing the <kbd><b>F1</b></kbd> key.
* A crash no longer occurs when moving the mouse while the help screen is open.
* A bug is fixed whereby some elements in the menu would only cast their shadow on the left side of the screen in some instances.
* A warning is now displayed in the console if a savegame couldn’t be deleted in the save or load game menus when pressing the <kbd><b>DEL</b></kbd> key.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 23, 2023

### DOOM Retro v5.0.2

* Minor changes have been made to text that is output to the console.
* The values of all CVARs that have been changed from their defaults are now highlighted when entering the `cvarlist` CCMD in the console.
* Pressing the <kbd><b>ENTER</b></kbd> key now closes the console if nothing has been typed in.
* These changes have been made to [voodoo dolls](https://doomwiki.org/wiki/Voodoo_doll):
  * They now move at the correct speed when on very slow scrolling floors.
  * The angle they face no longer changes once they teleport.
* A bug is fixed whereby the BFG-9000’s secondary projectiles would fire off in the wrong direction in some instances.
* The `english` CVAR now uses a value of `british` rather than `international`.
* Several improvements have been made to translating certain words when the `english` CVAR is `british`.
* Gibbed corpses now gib even more when reacting to further splash damage if the `r_corpses_gib` CVAR is `on`.
* The vertical position of an arch-vile’s shadow when the `r_shadows` CVAR is `on` has improved.
* A crash no longer occurs when pressing certain keys while the help screen is open.
* The player’s health, armor and ammo in the alternate widescreen HUD will now quickly count up from zero when entering a map if the `animatedstats` CVAR is `on`.
* A bug is fixed whereby the melee attack of monsters specified in `DEHACKED` lumps wouldn’t work if they used thing types 150 to 249.
* The vertical positions of the text on the intermission screen have improved.
* The vertical positions of the monsters’ shadows in *DOOM II’s* cast sequence have improved when the `r_shadows` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 16, 2023

### DOOM Retro v5.0.1

* A crash no longer occurs when loading a savegame from a [*DSDHacked*](https://doomwiki.org/wiki/DSDHacked)-compatible WAD.
* A bug is fixed whereby no music or sound effects would play when opening *DOOM Retro* for the first time after installing it.
* Panning around the automap using a controller’s left thumbstick when the `am_followmode` CVAR is `off` now works as intended.
* These changes have been made when playing [*Smooth DOOM MBF21*](https://www.doomworld.com/forum/topic/133318/):
  * Bullet casings no longer disappear and leave blood splats on the floor.
  * Blood splats now appear around corpses spawned at the start of a map when the `r_corpses_moreblood` CVAR is `on`.
* The `savegame` CVAR is now:
  * Updated when deleting a savegame in the save or load game menus using the <kbd><b>DEL</b></kbd> key.
  * No longer updated if the player cancels entering a savegame description in the save game menu.
* A bug is fixed whereby WADs placed in the `autoload` folder were causing a “W_GetNumForName: PLAYPAL not found!” error at startup.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, September 15, 2023

### DOOM Retro v5.0

* *DOOM Retro* is now built using v17.7.4 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.28.3*](https://github.com/libsdl-org/SDL/releases/tag/release-2.28.3).
* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Any `.cfg` files placed in the `autoload` folder are now autoloaded when *DOOM Retro* starts.
* *DOOM Retro* now parses any response files specified on the command-line.
* When using the WAD launcher to load a PWAD, if that PWAD doesn’t contain any conflicting maps, `SIGIL.WAD` or `NERVE.WAD` will now still be autoloaded if found.
* *E1M4B: Phobos Mission Control* and *E1M8B: Tech Gone Bad* are no longer available if either E1M4 or E1M8 have been replaced by a PWAD.
* These changes have been made to the support for `DEHACKED` lumps:
  * Support has been added for [*DSDHacked*](https://doomwiki.org/wiki/DSDHacked), allowing an unlimited number of things, states, sprites and sound effects.
  * A bug is fixed whereby custom monsters wouldn’t bleed in some instances.
* Minor changes have been made to *DOOM Retro’s* splash screen.
* Extensive changes have been made to text that is output to the console.
* Minor improvements have been made to text autocompleted in the console by pressing the <kbd><b>TAB</b></kbd> key.
* The caret may now be repositioned in the console’s input using the mouse pointer when the `m_pointer` CVAR is `on`.
* The effects of changing the `r_gamma` and `r_color` CVARs are now applied to *DOOM Retro’s* splash screen.
* The `r_color` CVAR has been renamed `r_saturation`, and can now be set to a value over `100%` to a maximum of `200%`, allowing colors on the screen to be oversaturated.
* A crash no longer occurs when trying to display certain patches greater than 200 pixels in height.
* These changes have been made to the menu:
  * Menu entries are now subtly highlighted when selected.
  * Improvements have been made when toggling widescreen using the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys while the menu is open.
  * The fade to black when quitting *DOOM Retro* from the menu is no longer affected by the player having either a berserk or radiation shielding suit power-up.
  * Minor improvements have been made to the edges of the menu’s background.
  * The skull cursor is no longer positioned incorrectly in the main menu in some rare instances.
  * When navigating the menu using the mouse pointer if the `m_pointer` CVAR is `on`:
    * The `episode`, `expansion`, `savegame` and `skilllevel` CVARs are now only updated when necessary.
    * The precision of selecting menu items has improved.
    * The *Nightmare!* skill level can now always be selected.
    * Clicking outside of the selected savegame slot when entering text in the save game menu now works as expected.
    * The mouse pointer now becomes visible after having just used a controller to navigate the menu.
  * “New Game” is now selected in the main menu after deleting the only savegame in the save or load game menus using the <kbd><b>DEL</b></kbd> key.
  * Minor improvements have been made to the vertical positioning of elements in the load and save game menus.
  * The skull cursor’s eyes are now always on while entering a savegame description in the save game menu.
  * The <kbd><b>F5</b></kbd> key can no longer be used to toggle the graphic detail while the menu is open and a game isn’t being played.
  * The console can now be opened using the <kbd><b>~</b></kbd> key while the menu is open.
  * The console can now also be opened by selecting the new “Console” entry in the options menu.
  * The shadows cast by the sliders in the options menu have improved in some rare instances.
  * The positions of `ON`/`OFF` and `HIGH`/`LOW` in the options menu have been fixed in some instances.
  * Right-clicking the mouse or pressing the <kbd><b>BACKSPACE</b></kbd> key on the title screen no longer opens the menu.
* The `r_supersampling` CVAR has been renamed `r_antialiasing` and is now `off` by default.
* Minor changes have been made to the help screen shown when pressing the <kbd><b>F1</b></kbd> key.
* The `r_lowpixelsize` CVAR can now also be set to `1×2` or `2×1`.
* A bug is fixed whereby sometimes the player wouldn’t pick up an item in a liquid sector when the `r_liquid_clipsprites` CVAR was `on`.
* The player’s view is no longer affected when walking over certain linedefs in a liquid sector and the `r_liquid_lowerview` CVAR is `on`.
* Improvements have been made to equipping either the chainsaw or fists when the player presses the <kbd><b>1</b></kbd> key.
* When a monster is killed in a liquid sector and the `r_liquid_clipsprites` CVAR is `on`:
  * The monster’s death animation is now at a consistent height.
  * The item they drop is no longer clipped before it reaches the liquid if the `tossdrop` CVAR is `on`.
* A bug is fixed whereby the player’s field of view would sometimes be wrong when changing the `r_fov` CVAR.
* Minor improvements have been made to the support of [*REKKR: Sunken Land*](https://store.steampowered.com/app/1715690/REKKR_Sunken_Land/).
* Floors and ceilings far away from the player are now rendered better.
* The sprites of multiple things that occupy the same coordinates now appear in the correct order, and also no longer flicker in some instances.
* The bobbing of sprites in liquid sectors can now be toggled using the new `r_liquid_bobsprites` CVAR, which is `on` by default and `off` when vanilla mode is enabled. This is separate to the existing `r_liquid_bob` CVAR, which now only toggles the bobbing of the liquid sectors themselves.
* Walls that have been incorrectly marked as two-sided are now rendered as intended.
* When the `IDCLEV` cheat has been entered and vanilla mode is enabled, the player now starts the map with 100% health, no armor, and only a pistol with 50 bullets.
* A slightly darker color is now chosen for the surrounding pillarboxes when displaying certain lumps from PWADs that are only 320 pixels wide and the `vid_widescreen` CVAR is `on`.
* A [bug](https://doomwiki.org/wiki/Stairs_create_unknown_sector_types) is fixed, present in the original *DOOM*, whereby stairs rising up from the floor would fail in some rare instances.
* If regenerating health is enabled using the `regenhealth` CCMD, a sound is now played and the “Health picked up” stat is updated every second the player’s health increases by 1% until it is 100% again.
* Several improvements have been made to the player’s health in the status bar when they are dead and the `negativehealth` CVAR is `on`.
* The `r_althud` CVAR is now `off` by default again.
* These changes have been made to the widescreen HUD:
  * The translucency has been reduced slightly when the `r_hud_translucency` CVAR is `on`.
  * Several improvements have been made to the flashing of the player’s health, armor and ammo when they change.
* All lines in the automap can now be anti-aliased by enabling the new `am_antialiasing` CVAR, which is `off` by default and when vanilla mode is enabled.
* The player’s crosshair when the `crosshair` CVAR is `on`, as well as the crosshair in the automap when the `am_followmode` CVAR is `off`, are no longer visible in the background while the console is open.
* These improvements have been made to the support for [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible WADs:
  * The `A_RadiusDamage` codepointer now correctly uses `Args1` rather than `Args2` for the amount of damage inflicted.
  * Things spawned using the `A_SpawnObject` codepointer now inherit the blood color of the spawner.
* These improvements have been made to the support for `MAPINFO` lumps:
  * A bug is fixed whereby many compatibility flags weren’t being parsed correctly.
  * `MAPINFO` lumps from other PWADs are now parsed if either `NERVE.WAD` or `SIGIL.WAD` are also loaded.
  * `episode` is now parsed correctly.
  * `bossaction` is now parsed.
  * The `compat_stairs` and `compat_zombie` compatibility flags are now parsed.
* Friendly monsters spawned using the `spawn` CCMD now follow the player into the next map.
* The mouse pointer is no longer displayed on the intermission or finale screens when the player moves the mouse and the `m_pointer` CVAR is `on`.
* When the `mouselook` CVAR is `on`:
  * Minor improvements have been made when the player aims vertically.
  * The player can now see slightly further when looking up or down.
  * A crash no longer occurs if a blood splat happens to touch the top of the screen while the player is looking down.
  * The vertical direction the player is looking is no longer reset in the help screen’s background when pressing the <kbd><b>F1</b></kbd> key.
* Blood splats far away from the player no longer flicker in some instances.
* The text in the automap is now always positioned correctly when the `am_external` CVAR is `on`.
* The default value of the `am_gridcolor` CVAR is now `6` instead of `111`.
* The player no longer exits the map if they are dead when a timer set using the `timer` CCMD runs out.
* Support has been added for the lumps `STFXDTH0` to `STFXDTH9`. If included in a PWAD, the player’s face in the status bar now animates when they are gibbed. This is a feature revived from the old [3DO](https://doomwiki.org/wiki/3DO), [Jaguar](https://doomwiki.org/wiki/Atari_Jaguar) and [PSX](https://doomwiki.org/wiki/Sony_PlayStation) *DOOM* ports.
* The accompanying readme file for the currently loaded PWAD can now be displayed by entering the new `readme` CCMD in the console.
* These extensive changes have been made when using a controller:
  * Using a controller in the menu is now more responsive.
  * The right trigger can now be used to select menu items.
  * The left and right thumbsticks, as well as the directional pad, can now be used to scroll the output in the console.
  * The B button can now be used to close the console.
  * The speed at which the player turns using a controller no longer increases while running.
  * Turning and moving using a controller is now more precise when only nudging the thumbsticks.
  * Controllers now rumble slightly when the player picks something up. This can be disabled by the new `joy_rumble_pickup` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
  * When the `joy_rumble_weapons` CVAR is `on`:
    * Controllers now rumble again when the player uses their chainsaw.
    * The amounts of rumble for each weapon the player fires have been adjusted.
  * The left and right shoulder buttons now equip the player’s previous and next weapons by default.
  * The default values of the `joy_deadzone_left` and `joy_deadzone_right` CVARs are now both `15%` rather than `25%`, and their maximum values are now both `30%` rather than `100%`.
* A bug is fixed whereby the `vid_fullscreen` CVAR couldn’t be changed in the console.
* The player’s weapon now displays correctly when it touches the right edge of the screen and the `vid_capfps` CVAR is not `35`.
* A bug is fixed whereby the flash of the player’s weapon could be drawn incorrectly if they fired it while falling and the `weaponbounce` CVAR was `on`.
* Movement of the player’s weapon is no longer blurred when the `r_detail` CVAR is `low`.
* The `weapon` CVAR can now also be set to the values `1` to `7`.
* A bug is fixed whereby the `iwadfolder` CVAR wouldn’t be updated in some instances.
* The `iwadfolder` CVAR has been renamed to just `wadfolder`.
* Improvements have been made to the minimum and maximum levels the player can zoom in the automap.
* A crash no longer occurs when starting a map that has things 4001 to 4004 (player starts 5 to 8).
* The message displayed when using the `IDCLEV` cheat or `map` CCMD to warp to a secret map is now gold.
* Minor improvements have been made to translating certain words when the `english` CVAR is `international`.
* A character no longer appears in the input when opening the console for the first time in some instances.
* The times on the intermission screen now have shadows.
* The brightmap of the `SW2STONE` texture has improved slightly when the `r_brightmaps` CVAR is `on`.
* The WAD being played is now displayed in the window caption again.
* A bug is fixed whereby certain messages were not being displayed in some instances.
* The player now falls at the correct speed when the clipping or freeze modes are enabled.
* All power-ups can now be given to the player by entering `give powerups` in the console.
* Conversely, all power-ups the player currently has can now be taken away from them by entering `take powerups` in the console.
* Widescreen mode can now be toggled using the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys on the intermission and finale screens.
* All actions may now be bound to all controls using the `bind` CCMD.
* Skies can now be drawn horizontally linear by enabling the new `r_linearskies` CVAR, which is `off` by default and when vanilla mode is enabled.
* The automap is now always updated when the `r_screensize` CVAR is changed while it is open.
* A bug is fixed whereby any player movement wouldn’t be canceled when opening the automap and the `am_followmode` CVAR was `off`.
* Savegames that were saved in maps `ExM10` or `ExM11` no longer crash when loaded.
* The console may now be closed by right-clicking the mouse.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, May 12, 2023

### DOOM Retro v4.9.2

* *DOOM Retro* is now built using v17.5.5 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro’s* window caption now always includes the text “DOOM Retro”.
* Changes have been made to text that is output to the console.
* Minor improvements have been made to the text that can be autocompleted in the console by pressing the <kbd><b>TAB</b></kbd> key.
* The menu’s background is now rendered correctly if *DOOM Retro’s* window is extremely wide.
* Adjusting the sliders in the options and sound volume menus using the mouse pointer is now more responsive when the `m_pointer` CVAR is `on`.
* Widescreen mode can now be toggled using the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys while the menu is open.
* The `sucktime` CVAR now shows as being `off` when set to `0`, and now has a maximum value of `24` hours.
* The time displayed on the intermission screen is now positioned better if the player takes more than an hour to complete a map and the `sucktime` CVAR is greater than its default of `1` hour.
* The map’s title in the automap is now gold if the player is in the secret map of *DOOM II: No Rest For The Living*.
* When the `flashkeys` CVAR is `on`:
  * Flashing keycards and skull keys are now positioned correctly in the alternate widescreen HUD.
  * Improvements have been made to flashing keycards and skull keys in the status bar when the player tries to open a *BOOM*-compatible door that requires all six keys.
* A bug is fixed whereby the amount of armor the player had was positioned incorrectly in the alternate widescreen HUD in some instances.
* Improvements have been made to the console’s background when the player has an invulnerability power-up or the `r_textures` CVAR is `off`.
* The number of monsters left to kill in a map, displayed in the automap when the `am_playerstats` CVAR is `on`, is now always correct after loading a savegame.
* The bounce of the player’s weapon when they land after a fall is now slightly faster when the `weaponbounce` CVAR is `on`.
* The default value of the `am_markcolor` CVAR is now `89`.
* The default values of the `joy_deadzone_left` and `joy_deadzone_right` CVARs are now both `25%`.
* These changes have been made to vanilla mode toggled using the `vanilla` CCMD:
  * The `+strafe` action is now bound to the `mouse2` control again, allowing the player to strafe with the mouse, and to also double-click the button to use doors and switches.
  * The speed the player can move forward and back, and strafe left and right, has been reduced.
* Improvements have been made to the alternate widescreen HUD when the `r_hud_translucency` CVAR is `off` and either the `r_textures` CVAR is also `off` or the player has an invulnerability power-up.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, April 22, 2023

### DOOM Retro v4.9.1

* *DOOM Retro* is now built using v17.5.4 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Several changes have been made to text that is output to the console.
* When playing *DOOM (Shareware)*, games are now saved in a separate folder to the full version of *DOOM*.
* A bug is fixed whereby the music’s volume wasn’t being set correctly in some instances.
* A crash no longer occurs when a corpse is crushed into gibs by a lowering sector in some instances.
* When the `m_pointer` CVAR is `on`:
  * Adjusting the sliders in the options and sound volume menus using the mouse pointer is now more responsive.
  * The mouse pointer is now visible in the console if the player moves the mouse while it is open. Clicking anywhere below the console during a game now closes it.
* The map’s title in the automap is now gold if the player is in a secret map.
* The smoke trailing rockets when the `r_rockettrails` CVAR is `on` is now colored correctly even if a custom `PLAYPAL` lump is loaded.
* When loading a savegame:
  * A warning is now displayed in the console if a WAD is missing.
  * A bug is fixed whereby the number of items to be picked up in the map was wrong in some instances.
* The player now slides more smoothly against walls at certain angles.
* These changes have been made to the radiation shielding suit power-up:
  * The screen no longer briefly flashes yellow when the player picks up the power-up and the `r_pickupeffect` CVAR is `on`.
  * If the `r_radsuiteffect` CVAR is `off`:
    * The screen now briefly flashes green when the player picks up the power-up and the `r_pickupeffect` CVAR is `on`.
    * The screen now flashes green again to indicate when the power-up is about to run out.
* The screen no longer briefly flashes yellow when the player picks up a berserk power-up and the `r_pickupeffect` CVAR is `on`.
* When the player uses a controller, it now rumbles when they use their chainsaw, there’s no target, and the `joy_rumble_weapons` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, April 7, 2023

### DOOM Retro v4.9

* *DOOM Retro* is now built using v17.5.3 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.26.5*](https://github.com/libsdl-org/SDL/releases/tag/release-2.26.5).
* Several optimizations have been made to further improve the overall performance, stability and memory usage of *DOOM Retro*.
* Several changes have been made to text that is output to the console.
* When the `m_pointer` CVAR is `on`:
  * The mouse pointer may now be used to move:
    * The sliders in the options and sound volume menus.
    * The text caret when entering a savegame description in the save game menu.
  * Selecting menu items using the mouse pointer is now accurate when the `vid_widescreen` CVAR is `off`.
  * The mouse pointer is no longer shown on the help screen if the mouse is moved.
* The positions of `ON`/`OFF` and `HIGH`/`LOW` in the options menu have been fixed in some instances.
* A bug is fixed whereby when changing the `r_pickupeffect` CVAR in the console, the `r_damageeffect` CVAR would be changed instead.
* The player’s health, armor and ammo in both the status bar and widescreen HUD now animate when they change. This can be disabled by the new `animatedstats` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* These changes have been made to the alternate widescreen HUD:
  * The position of the player’s health is now fixed when 0% and the `negativehealth` CVAR is `off`.
  * Every part of the HUD is now black, silhouetted against the bright background, when the player has an invulnerability power-up or the `r_textures` CVAR is `off`.
  * The vertical positions of some of the weapon silhouettes have changed.
  * Minor improvements have been made to the armor bar when the player has blue armor.
* Crashes no longer occur when:
  * Changing the `health` CVAR to `0%` or less.
  * Using the `take` CCMD to take more health from the player than they have.
* A bug is fixed whereby widescreen mode wouldn’t be restored correctly when turning vanilla mode off using the `vanilla` CCMD.
* `NEXT` and `SECRETNEXT` now work as intended in `MAPINFO` lumps.
* Partial translucency effects when the `r_sprites_translucency` CVAR is `on`, and smoke that trails rockets when the `r_rockettrails` CVAR is `on`, now appear again if a custom `PLAYPAL` lump is loaded.
* The player’s “Health picked up” stat now updates correctly when the player picks up a soul sphere.
* A bug is fixed whereby the number of monsters in the map may have been wrong after loading a savegame in some instances.
* Blood splats are no longer overly bright when the `r_textures` CVAR is `off`.
* Masked midtextures with a brightmap specified in a [`BRGHTMPS`](https://raw.githubusercontent.com/bradharding/doomretro/master/res/BRGHTMPS) lump now display correctly when the `r_brightmaps` CVAR is `on`.
* Bound controls are now grouped by type when saved in `doomretro.cfg` and in the output of the `bindlist` CCMD.
* The triangles in the automap that represent every blood splat, displayed when using the `IDDT` cheat, are now slightly larger.
* Blood is now shown when shooting a lost soul and the `r_blood` CVAR is `red`.
* The movement of some sectors is now smoother when the `vid_capfps` CVAR is not `35`.
* A bug is fixed whereby pressing a key or mouse button while a controller is rumbling could cause the rumble to continue indefinitely in some instances.
* These changes have been made to the fuzz effect applied to both the player’s weapon when they have a partial invisibility power-up, and to spectres:
  * A crash no longer occurs if the sprite of a spectre touches the top of the screen.
  * The effect is no longer frozen when freeze mode is on.
* The `vid_borderlesswindow` CVAR is now `on` by default.
* The `vid_vsync` CVAR is now `off` by default.
* Minor improvements have been made to translating certain words when the `english` CVAR is `international`.
* Thing triangles no longer move in the automap when the `IDDT` cheat has been entered and freeze mode is on.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, March 8, 2023

### DOOM Retro v4.8.1

* *DOOM Retro* now uses [*SDL v2.26.4*](https://www.libsdl.org/).
* Minor optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* A bug is fixed whereby if the `alwaysrun` CVAR was `on`, the <kbd><b>CAPSLOCK</b></kbd> key wouldn’t always turn off as intended when quitting *DOOM Retro*.
* A bug is fixed whereby a wrong player message would be displayed each time the player picked up an item in some instances when the `messages` CVAR was `off`.
* Minor improvements have been made to the color of some elements in the alternate widescreen HUD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, March 7, 2023

### DOOM Retro v4.8

* *DOOM Retro* is now built using v17.5.1 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.26.3*](https://www.libsdl.org/), [*SDL_mixer v2.6.3*](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.6.3) and [*SDL_image v2.6.3*](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.6.3).
* Extensive optimizations have been made to further improve the overall performance, stability and memory usage of *DOOM Retro*.
* When *DOOM Retro* is run for the first time, the WAD launcher will now also look for the IWADs included with installations of [*DOOM Eternal*](https://store.steampowered.com/app/782330/DOOM_Eternal/).
* The [*id Software*](https://www.idsoftware.com) logo has been removed from the title screen to be consistent with the latest rereleases of *DOOM* and *DOOM II*.
* Extensive changes have been made to text that is output to the console.
* Many additional characters, including those with diacritics, have been added to the font set used in the console.
* [Alt codes](https://en.wikipedia.org/wiki/Alt_code) may now be entered in the console.
* These changes have been made when opening and closing the console using the <kbd><b>~</b></kbd> key:
  * A subtle sound effect now plays.
  * The console now scrolls to and from the top of the screen, rather than fading in and out, when on the title screen.
  * The console’s scrolling animation is now slightly faster.
  * The scrollbar now displays correctly when the `r_screensize` CVAR is less than `7`.
* Minor improvements have been made to the text that can be autocompleted in the console by pressing the <kbd><b>TAB</b></kbd> key.
* Each press of the <kbd><b>TAB</b></kbd> key to autocomplete text entered in the console can now be undone by pressing <kbd><b>CTRL</b></kbd> + <kbd><b>Z</b></kbd>.
* Pressing the <kbd><b>HOME</b></kbd> key in the console no longer scrolls past the top of the console in some instances.
* Scrolling up and down in the console using the <kbd><b>PGUP</b></kbd> and <kbd><b>PGDN</b></kbd> keys now gets faster the longer the keys are held down.
* Thousands-delimiting commas may now be used when changing the value of certain CVARs in the console.
* The `toggle` CCMD now only works with boolean CVARs.
* When the `m_pointer` CVAR is `on`:
  * The mouse wheel may now be used to scroll among menu items again.
  * If the mouse pointer is hidden, pressing a mouse button without moving the mouse now opens the menu from the title screen, or selects the currently highlighted item if the menu is open.
* The fade effect when opening the menu is now slightly faster than closing it when the `fade` CVAR is `on`.
* Minor improvements have been made to the menu’s background when the `r_detail` CVAR is `low`.
* Two additional savegame slots are now available in the load and save game menus.
* A random sound effect now plays when quitting the game by entering the `quit` CCMD in the console or pressing <kbd><b>ALT</b></kbd> + <kbd><b>F4</b></kbd>.
* Improvements have been made to parsing [`BRGHTMPS`](https://raw.githubusercontent.com/bradharding/doomretro/master/res/BRGHTMPS) lumps.
* These changes have been made to the `playerstats` CCMD:
  * The number of times the player loads a savegame is now displayed.
  * The number of monsters gibbed by the player is now displayed.
  * The number of cells the player has picked up is no longer displayed when playing *DOOM (Shareware)*.
  * The number of times the player has died is now corrected when they are resurrected using either the `resurrect` CCMD or the `IDDQD` cheat.
* These improvements have been made to the support for `DEHACKED` lumps:
  * Cheats that have been changed are now also able to be entered in the console.
  * Specifying a weapon name will no longer cause a crash.
  * A bug is fixed whereby par times wouldn’t be changed in some instances.
* These improvements have been made to the support for `UMAPINFO` lumps:
  * The finale’s background will now display correctly if `interbackdrop` specifies a larger patch rather than a 64×64 flat texture.
  * `secretnext` now works as intended.
* *DOOM’s* stimpacks and medikits no longer appear in [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest).
* Minor improvements have been made to the support of [*REKKR*](https://www.doomworld.com/idgames/levels/doom/megawads/rekkr).
* The player’s health, armor and ammo are now displayed better in the status bar and widescreen HUD if they start with a `1`.
* The widescreen HUD is now brought in from the left and right edges of ultra-wide displays.
* Many changes have been made to the alternate widescreen HUD:
  * The position of the player’s health is now fixed when 0%.
  * Improvements have been made to the display of the armor bar when the player has blue armor, and also when the player has a blue keycard or skull key.
  * An empty ammo bar is now displayed if the player has their fists or chainsaw equipped.
  * The number of notches in the ammo bar no longer doubles when the player has a backpack. Instead, if the player has more than the usual maximum amount of ammo for their currently equipped weapon, a second bar overlaps the first to show the difference.
  * Several elements have been moved slightly.
  * Several elements are now slightly less translucent.
  * A bug is fixed whereby the silhouette of the player’s currently equipped weapon wouldn’t be displayed in some rare instances.
  * When the `r_hud_translucency` CVAR is `off`:
    * The player’s health, armor and ammo now flash when they change.
    * The white elements of the HUD are now not as bright.
    * The armor icon is now a darker gray when the player has no armor.
    * The armor bar is now a darker green when the player has 100% armor or less.
    * The power-up bar now updates when depleting.
* The truncation of player messages when the `vid_widescreen` CVAR is `off` has improved in some instances.
* The position of player messages, and the map’s title in the automap, has improved in some instances.
* The vertical positions of the monsters’ shadows in *DOOM II’s* cast sequence have improved when the `r_shadows` CVAR is `on`.
* The position of text displayed in the top right of the screen has improved in some instances.
* The correct map now opens when entering `map phobosmissioncontrol` or `map techgonebad` in the console.
* Minor improvements have been made to translating certain words, as well as the status bar, when the `english` CVAR is `international`.
* The fuzz effect applied to both the player’s weapon when they have a partial invisibility power-up, and to spectres, has been redesigned to be more pixelated.
* Certain partial translucency effects when the `r_sprites_translucency` CVAR is `on` are no longer applied if a custom `PLAYPAL` lump is present in a PWAD.
* When the `r_rockettrails` CVAR is `on`:
  * The smoke trailing behind rockets fired by the player and cyberdemons is now a lighter gray.
  * Smoke now doesn’t trail behind rockets fired by the player and cyberdemons if a custom `PLAYPAL` lump is present in a PWAD.
  * The smoke trailing behind homing missiles fired by revenants is now the same as the smoke behind rockets fired by the player and cyberdemons, but not if a custom `PLAYPAL` lump is present in a PWAD.
* The LEDs on *DualShock 4* and *DualSense* and *DualSense Edge* controllers now turn red again when connected.
* When the `vid_fullscreen` CVAR is `off`:
  * The status bar and widescreen HUD are now displayed better when the window is resized to be very narrow.
  * The mouse pointer is now displayed while the console is open.
* The mouse pointer is now hidden before the screen goes black at startup when the `vid_fullscreen` CVAR is `on`.
* A crash no longer occurs when displaying patches taller than 200 pixels.
* The disk icon displayed in the top right of the screen when the `r_diskicon` CVAR is `on` is now double the size when the `r_detail` CVAR is `low`.
* The player’s view no longer bounces once they land after a fall if either no clipping mode or freeze mode are enabled.
* The player’s health, armor, ammo and weapon are no longer reset by the `resetall` CCMD.
* The `kill` CCMD can no longer be used to kill the player if either god mode or buddha mode are enabled, or if they have an invulnerability power-up.
* The *BOOM*-compatible line special of 251 (“Scroll floor according to line vector”) now works correctly if used with a liquid sector and the `r_liquid_current` CVAR is `on`.
* The underscores in the message displayed by entering the `IDBEHOLD` cheat now always align correctly again.
* The shadows cast by the corpses of monsters spawned at the start of the map are now positioned better when the `r_shadows` CVAR is `on`.
* If the `ammo` CVAR is changed in the console to be greater than the maximum ammo for the player’s currently equipped weapon and they don’t have a backpack, they are now given one.
* The console no longer closes when the `ammo`, `armor` or `health` CVARs are changed, unless necessary.
* When the `autouse` CVAR is `on`:
  * The automatic use of doors and switches is now more responsive.
  * The player no longer grunts repeatedly when standing in front of a locked door that they don’t have the key for, or a reusable switch that has just been turned on.
  * A player message now only appears once when the player is standing in front of a locked door that they don’t have the key for.
  * The player may now still use the `+use` action.
  * The automatic use of doors and switches no longer occurs when no clipping mode is enabled.
* The randomization of decorations that animate in a map has improved in some instances when the `r_randomstartframes` CVAR is `on`.
* When the `pistolstart` CCMD or `-pistolstart` command-line parameter are used:
  * Games are now still autosaved if the `autosave` CVAR is `on`, and autoloaded if the `autoload` CVAR is `on`.
  * A warning is now displayed in the console at the start of each map indicating the player now has “100% health, no armor, and only a pistol with 50 bullets”.
  * The 100% health and 50 bullets can no longer be changed in a `DEHACKED` lump.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, December 17, 2022

### DOOM Retro v4.7.2

* *DOOM Retro* is now built using v17.4.3 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* Minor optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Exploding barrels are now lit correctly again.
* A bug is fixed whereby digits in the alternate widescreen HUD could be positioned incorrectly in some rare instances.
* The options menu now fits horizontally on the screen when using a controller and the `vid_widescreen` CVAR is `off`.
* A crash no longer occurs when exiting a map in the 4th or 5th episode of a PWAD.
* The position of text displayed in the top right of the screen has improved when the `vid_widescreen` CVAR is `off`.
* The grouping of identical player messages when the `groupmessages` CVAR is `on` has improved.
* A bug is fixed whereby strings containing underscores in `DEHACKED` lumps were being read incorrectly.
* `TEXTURE` entries in `BRGHTMPS` lumps no longer require `DOOM`, `DOOM2` or `DOOM|DOOM2` to be specified.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, December 11, 2022

### DOOM Retro v4.7.1

* Minor optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The health bar in the alternate widescreen HUD now still turns white when the player’s health is less than 100% and buddha mode is enabled.
* A bug is fixed whereby blood splats could be displayed in the wrong color in some instances.
* A bug is fixed whereby certain CVARs would always reset to their defaults at startup.
* A crash no longer occurs when shooting a spectre after loading a savegame in some instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, December 10, 2022

### DOOM Retro v4.7

* *DOOM Retro* is now built using v17.4.2 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* *DOOM Retro* now uses [*SDL v2.26.1*](https://www.libsdl.org/).
* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Changes have been made to the animation of *DOOM Retro’s* logo on the splash screen.
* A mouse pointer is now displayed while the menu is open, and the mouse is moved, that can be used to select menu items. It may be disabled by the new `m_pointer` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* Extensive changes have been made to text that is output to the console.
* A bug is fixed whereby one or more lines of text at the top the console wouldn’t be completely displayed.
* Minor improvements have been made to the console’s autocomplete feature.
* Minor improvements have been made to the help screen displayed by pressing the <kbd><b>F1</b></kbd> key.
* CCMDs in the console that can only be used while playing a game, or while the player is alive, may now be entered at any time, and a description of the CCMD along with a warning about their usage is displayed.
* The colors of certain elements of the console have been improved.
* The game is now paused as intended when the window loses focus.
* These changes have been made to the external automap:
  * It now goes to black rather than dark blue while the help screen is displayed.
  * `00:00` is no longer displayed in the top right corner before a game is started if the `am_playerstats` CVAR is `on`.
* The vertical position of the current map’s title in the automap has changed in some instances.
* A bug is fixed whereby the current map’s title in the automap would be positioned incorrectly after adjusting the screen size in the options menu in some instances.
* These changes have been made when pressing the <kbd><b>F9</b></kbd> key to quickload a savegame:
  * The load game menu now opens if the game hasn’t been saved yet.
  * The background of the confirmation message now displays correctly.
  * There is no longer a misplaced fade transition when pressing the <kbd><b>Y</b></kbd> key and the `fade` CVAR is `on`.
* The horizontal position of the title in the options and sound volume menus has improved in some instances.
* The shadows of the red text used in the menu have improved slightly.
* A fade transition is now applied when opening the sound volume menu in the options menu if the `fade` CVAR is `on`.
* A fade transition is no longer applied when toggling the graphic detail, either in the options menu or by pressing the <kbd><b>F5</b></kbd> key, if the `fade` CVAR is `on`.
* When toggling widescreen mode by pressing the <kbd><b>+</b></kbd> key, the player’s vertical field of view now zooms in slightly.
* An `english` CVAR has been implemented that toggles the use of American or International English. It can be either `american` or `international`, and is `american` by default and when vanilla mode is enabled.
* To allow greater precision, the `joy_sensitivity_horizontal`, `joy_sensitivity_vertical` and `m_senstivity` CVARs can now be changed to non-integer values.
* Turning the `r_corpses_mirrored` or `r_mirroredweapons` CVARs `on` in the console now works correctly.
* The effects of changing the `r_randomstartframes` CVAR in the console are now immediate.
* A `NOMIRROREDCORPSE` flag can now be used in a thing’s `Retro bits` in `DEHACKED` lumps to force a monster’s corpse not to be randomly mirrored even if the `r_corpses_mirrored` CVAR is `on`.
* Support has been added for [*Harmony*](https://slayersclub.bethesda.net/en/article/E5gTsCSptkfJC43vh9Pey/new-add-on-available-harmony).
* Further improvements have been made to the support of [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest), [*REKKR*](https://www.doomworld.com/idgames/levels/doom/megawads/rekkr) and [*REKKR: Sunken Land*](https://store.steampowered.com/app/1715690/REKKR_Sunken_Land/).
* These changes have been made to the *BFG Edition* and latest rerelease of *DOOM* and *DOOM II*:
  * The `TITLEPIC` lump from *The Ultimate DOOM* is no longer used as the title screen of *DOOM*.
  * *DOOM* is no longer referred to as *The Ultimate DOOM* in the window’s caption or in the console.
  * The `DMENUPIC` lump is no longer used as the title screen of *DOOM II*.
  * Unless replaced in a PWAD, all stimpacks, medikits and berserk power-ups now always show a red cross, rather than a green cross or a pill.
* Even though it can’t be played, the fourth episode of *DOOM* is now displayed in the episode menu of *DOOM (Shareware)*.
* The horizontal position of player messages has now improved, and is consistent when toggling widescreen.
* Any player message is now cleared when opening the automap.
* Player messages are no longer displayed when a [voodoo doll](https://doomwiki.org/wiki/Voodoo_doll) picks something up.
* The position of text displayed in the top right of the screen has improved when the `vid_widescreen` CVAR is `off`.
* When loading a savegame that was saved using certain previous versions of *DOOM Retro*, all blood splats are now displayed correctly.
* When to spawn blood splats around a decoration while the `r_corpses_moreblood` CVAR is `on` has now improved if the decoration’s sprite is replaced in a PWAD.
* A bug is fixed whereby the wrong `M_DOOM` lump was displayed in the main menu in some instances.
* Minor improvements have been made when adjusting the music volume.
* The MIDI device used to play music is now displayed in the console at startup.
* Bullet puffs are now spawned again when barrels are shot at.
* The vertical position of blood and bullet puffs spawned is now more random.
* The player’s view now shakes when they punch something and have a berserk power-up. This feature may be toggled using the new `r_shake_berserk` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* A bug is fixed whereby game controllers wouldn’t rumble when the player punched a monster and the `joy_rumble_damage` CVAR was `on`.
* A crash no longer occurs if the `save` CCMD is bound to a control using the `bind` CCMD.
* The corpses of monsters no longer all slide in the same direction when using the `kill` CCMD to kill them.
* The obituary displayed in the console when the player is killed by a damaging sector that isn’t liquid is now fixed.
* Any input in the console is now cleared when a cheat is entered while the console is closed.
* Improvements have been made to the synchronization of animated wall textures and flats.
* The brightmap of the `COMPUTE1` texture has improved when the `r_brightmaps` CVAR is `on`.
* Brightmaps are no longer applied to the `SLADRIP1`, `SLADRIP2` and `SLADRIP3` textures when the `r_brightmaps` CVAR is `on`.
* A bug is fixed whereby masked midtextures could in some instances be rendered incorrectly while the player had a light amplification visor power-up.
* The `r_skycolor` CVAR has been removed.
* The default of the `am_pathcolor` CVAR is now `89`.
* Blood splats are now displayed in the automap as very small triangles when using the `IDDT` cheat. Their color can be changed using the new `am_bloodsplatcolor` CVAR, which is `124` by default and `0` when vanilla mode is enabled.
* The color of corpses in the automap when using the `IDDT` cheat can be changed using the new `am_corpsecolor` CVAR, which is `116` by default and `112` when vanilla mode is enabled.
* Several improvements have been made to the size and angle of thing triangles in the automap when using the `IDDT` cheat.
* The player is now given double ammo again when entering the `IDFA` and `IDKFA` cheats.
* These changes have been made to the `playergender` CVAR:
  * It can now be changed and its values displayed correctly again in the console.
  * A value of `nonbinary` is now used rather than `other`.
* Controls are no longer reset to their defaults when vanilla mode is enabled using the `vanilla` CCMD.
* These changes have been made to the `vid_capfps` CVAR:
  * Its minimum value is now `35` rather than `10`.
  * A bug is fixed whereby it couldn’t be set to `off` in the console.
* These changes have been made to the widescreen HUD:
  * All numbers are now monospaced, appearing the same way as they appear in the status bar.
  * The amount of ammo the player has no longer flashes when switching weapons.
  * The player’s health no longer flashes when less than `10%` while buddha mode is enabled.
* These changes have been made to the alternate widescreen HUD:
  * The notches are now slightly less translucent.
  * The left and right edges of the health and ammo bars are now slightly brighter.
  * The portions of the player’s health and armor greater than `100%` are now slightly brighter in the health and armor bars.
  * The player’s health, armor and ammo now flash when they change.
  * The player’s health no longer turns red when less than `10%` while buddha mode is enabled.
* There are no longer any anomalies in *E1M4B: Phobos Mission Control* when the `r_fixmaperrors` CVAR is `on`.
* Fixes have been made to some textures in *MAP31: Wolfenstein* when the `r_fixmaperrors` CVAR is `on`.
* These changes have been made to the support of `MAPINFO` lumps:
  * `compat_floormove` can now be used to allow floors to move up past their ceilings, and ceilings to move down past their floors, like in *Vanilla DOOM*.
  * `compat_useblocking` can now be used to cause any line with a special to intercept the player’s use action and not allow any lines behind it to trigger, like in *Vanilla DOOM*.
  * The use of `allowmonstertelefrags` has improved.
* The player’s angle and position are now displayed in the top right of the screen, rather than as a persistent player message, when the `IDMYPOS` cheat is entered.
* A `-solonet` command-line parameter has been implemented that toggles all things usually intended for multiplayer to spawn at the start of each map, and the player to respawn when they die.
* A `sucktime` CVAR has been implemented that sets the amount of time in hours the player must complete the current map before “SUCKS!” is shown on the intermission screen. This also affects the output from the `am_playerstats` CVAR and the `playerstats` CCMD. If this CVAR is `0`, “SUCKS!” is never shown. This CVAR is `1` by default and when vanilla mode is enabled.
* The intermission screen now pauses as intended when the menu or console is open, or the game is paused.
* The artist and title of the currently playing music are now displayed by the `mapstats` CCMD when playing *DOOM (Shareware)* or *DOOM II: No Rest For The Living*.
* Everything spawned by a monster spawner (such as during *MAP30: Icon Of Sin*) now counts towards the player’s stats.
* The upward momentum applied to items dropped by monsters once they are killed has increased when the `tossdrop` CVAR is `on`.
* A warning is now displayed in the console at startup when a control is unbound from an action because it is already bound to another action.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, September 1, 2022

### DOOM Retro v4.6.2

* *DOOM Retro* is now built using v17.3.3 of [*Microsoft Visual Studio Community 2022*](https://visualstudio.microsoft.com/vs/community/).
* Several optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The amount of time the player has been in the current map is now displayed in the automap if the `am_playerstats` CVAR is `on`.
* A `maptime` CVAR has been implemented that shows the amount of time the player has been in the current map. This CVAR is read-only.
* The position of the current map’s title in the automap has changed in some instances.
* The colors of the power-up bar in the alternate widescreen HUD are now inverted while the player has an invulnerability power-up or the `r_textures` CVAR is `off`.
* A brightmap is now applied to the `SIGIL` texture in *SIGIL* if the `r_brightmaps` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, August 27, 2022

### DOOM Retro v4.6.1

* *DOOM Retro* is now built using v17.3.2 of [*Microsoft Visual Studio Community 2022*](https://www.visualstudio.com/vs/).
* Minor optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The menu’s background is now displayed correctly when pressing the <kbd><b>F9</b></kbd> key to quickload a savegame.
* A bug is fixed whereby savegames saved using versions of *DOOM Retro* earlier than v4.6 would crash in some instances.
* *SIGIL* is now recognized if it is named `SIGIL_v1_0.wad`.
* Minor improvements have been made to the playback of MIDI music.
* The following changes have been made to the alternate widescreen HUD:
  * The power-up bar now depletes from left to right.
  * The amount of time the power-up bar is visible is now more accurate.
  * The blue color used in the armor bar and for keycards and skull keys is now slightly lighter.
  * The number of notches in the ammo bar has increased by one.
  * The number of notches in the ammo bar now doubles if the player picks up a backpack.
  * The power-up bar, as well as any keycards or skull keys the player has picked up, are no longer displayed while the player is dead.
* The player’s super shotgun will now always be positioned correctly.
* A crash no longer occurs in the automap when the player turns and the `am_followmode` CVAR is `off` and the `am_rotatemode` CVAR is `on`.
* The position of player messages has shifted in some instances.
* A bug is fixed whereby entering a cheat would not work correctly in some instances.
* Minor improvements have been made when specifying `.deh` and `.bex` files on the command-line.
* Any `.wad`, `.deh` or `.bex` files are now autoloaded if placed in a subfolder of the `autoload` folder based on the current IWAD, even if a PWAD is also loaded.
* The default of the `am_allmapcdwallcolor` CVAR is now `109`.
* Improvements have been made to unmapped areas of the automap when the player has a computer area map power-up.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, August 21, 2022

### DOOM Retro v4.6

* *DOOM Retro* is now built using v17.3.1 of [*Microsoft Visual Studio Community 2022*](https://www.visualstudio.com/vs/).
* *DOOM Retro* now uses [*SDL v2.24.0*](https://www.libsdl.org), [*SDL_mixer v2.6.2*](https://www.libsdl.org/SDL_mixer) and [*SDL_image v2.6.2*](https://www.libsdl.org/SDL_image).
* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* no longer crashes if run from a folder that contains non-Latin characters in its path.
* Several changes have been made to text that is output to the console.
* Minor improvements have been made to the console’s autocomplete feature.
* The amount of blood splats displayed now depends on their distance from the player again.
* A message is now displayed with an accompanying sound effect whenever the player finds a secret. This feature may be toggled using the new `secretmessages` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* Secrets are no longer considered to be found if the player happens to walk into one while no clipping mode or freeze mode is enabled.
* The position of player messages has shifted in some instances.
* Several improvements have been made to the alternate widescreen HUD.
* The `r_althud` CVAR is now `on` by default.
* The following changes have been made to the menu’s background:
  * It now has a border with curved corners.
  * The player’s field of view is now increased to `135°`.
  * The player’s view is now lowered to the ground.
  * The player’s view is now vertically centered if the `mouselook` CVAR is `on`.
  * The spin of the player’s view now sometimes reverses direction if the menu is closed, the player doesn’t move, and the menu is then reopened.
  * Any animated textures or lighting effects are now slowed down.
  * The automap is no longer used as the background if the menu is opened while it is displayed.
  * Unmapped lines in the automap are no longer mapped while the player’s view spins.
* The text caret is now always positioned correctly when entering a savegame description in the savegame menu.
* The following changes have been made to the help screen displayed by pressing the <kbd><b>F1</b></kbd> key:
  * Subtle noise is now applied to the background.
  * `HELP`, `HELP1` and `HELP2` lumps in PWADs that are wider than 320 pixels are now displayed correctly.
  * The correct background is now displayed if the help screen is opened while the automap is displayed.
* If it doesn’t already exist, an `autoload` folder is now created during startup. Any `.wad`, `.deh` or `.bex` files that are placed in this folder, (or in the subfolder based on the name of the current WAD), will then always be automatically loaded during startup.
* The number of gamma correction levels set by the `r_gamma` CVAR that are brighter than `1.0` has been reduced.
* The `r_translucency` CVAR has been split into the following two CVARs:
  * The new `r_sprites_translucency` CVAR toggles the translucency of certain sprites. This CVAR is `on` by default and `off` when vanilla mode is enabled.
  * The new `r_textures_translucency` CVAR toggles the translucency of certain [*BOOM*](https://doomwiki.org/wiki/Boom)-compatible wall textures. This CVAR is also `on` by default and `off` when vanilla mode is enabled.
* Blood is now spawned during melee attacks from monsters. This feature may be toggled using the new `r_blood_melee` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* Bullet puffs are no longer spawned instead of blood when the `r_blood` CVAR is `none`.
* The following changes have been made when the `r_liquid_clipsprites` CVAR is `on`:
  * The bottoms of monster sprites are no longer momentarily still clipped as they move out of liquid.
  * The bottoms of monster sprites are no longer clipped when in liquid if their top offset makes them too high.
* A crash no longer occurs:
  * When entering the `IDBEHOLD` cheat while the alternate widescreen HUD is displayed.
  * When entering the `map` CCMD in the console with an invalid parameter.
  * During intermission if the `WIENTER` or `WIF` lumps are taller than the screen.
* Minor improvements have been made to the playback of MIDI music.
* The `IDCHOPPERS` cheat is now canceled properly when the player exits a map.
* The following changes have been made in the automap:
  * The angles of thing triangles when the `IDDT` cheat is used are now interpolated when the `vid_capfps` CVAR is a value other than `35`.
  * The current map’s title is now in italics when the `r_althud` and `vid_widescreen` CVARs are `on`.
  * All text displayed now has the same amount of translucency when the `r_althud` and `vid_widescreen` CVARs are `on`.
  * The position of the current map’s title has changed in some instances.
  * There are no longer fade transitions when using certain controls and the `fade` CVAR is `on`.
* The status bar and widescreen HUD are now displayed correctly when *DOOM Retro* is paused by pressing the <kbd><b>PAUSE</b></kbd> key.
* The chainsaw has been shifted to the left slightly when the `r_althud` and `vid_widescreen` CVARs are `on`.
* The following changes have been made to the support of `DEHACKED` lumps:
  * The player’s super shotgun is now positioned correctly if the offsets of any of its frames are changed.
  * Using the `SHADOW` flag in `Bits` now always works as intended.
  * A crash no longer occurs when attempting to display the player’s health in the widescreen HUD if `Initial Health` or `Max Health` are changed to a value greater than `999`.
  * Gibbing corpses are no longer affected if `Exploding frame` is changed.
  * `Blood color` may now be used to change the color of blood spilled by monsters. This is set to a value between `0` and `8` (representing the colors red, gray, green, blue, yellow, black, purple, white and orange).
  * If the `SHADOW` flag is used in `Bits`, the monster now spills fuzzy blood if the `r_blood` CVAR is `all`.
  * `Blood` in `Thing` blocks is no longer used.
  * The `TRANSLUCENT_REDTOGREEN_33`, `TRANSLUCENT_REDTOBLUE_33`, `REDTOGREEN` and `REDTOBLUE` flags in `Retro bits` are no longer used.
  * Pain elementals, lost souls and barrels are no longer translucent when exploding if any of their states have been changed.
  * The player’s view now shakes if a thing other than a barrel (but not a missile) uses the `A_Explode` codepointer and the `r_shake_barrels` CVAR is `on`.
  * If a thing is changed in any way, the offsets of all of its sprite’s frames are no longer corrected even if the `r_fixspriteoffsets` CVAR is `on`.
  * The values of `Green Armor Class` and `Blue Armor Class` in `Misc` blocks are now always used.
  * By adding an underscore before and after text in a player message, that text will appear italicized in the console and when the `r_althud` and `vid_widescreen` CVARs are `on`.
* The following changes have been made to the support of `MAPINFO` lumps:
  * `nofreelook` and `nojump` now work as intended.
  * `enterpic` now works as intended.
  * `exitpic` can now be used to specify the lump displayed when the player exits a map.
  * `compat_nopassover` can now be used to override the effects of the `infiniteheight` CVAR when `off`.
* Minor improvements have been made to the support of [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible WADs.
* The following changes have been made to brightmaps when the `r_brightmaps` CVAR is `on`:
  * A new [`BRGHTMPS`](https://raw.githubusercontent.com/bradharding/doomretro/master/res/BRGHTMPS) lump has been introduced that allows brightmaps to be changed in a PWAD.
  * Minor improvements have been made to the brightmaps of the `COMPUTE2` and `COMPUTE3` textures.
  * Brightmaps can now be applied to masked midtextures.
* The existing feature of randomizing the starting frame of certain sprites may now be toggled using the new `r_randomstartframes` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* The player’s speed when strafing has been reduced when moving the mouse while the <kbd><b>ALT</b></kbd> key is held down, and is more consistent with the speed when using the <kbd><b>A</b></kbd> and <kbd><b>D</b></kbd> keys to strafe.
* The following changes have been made when vanilla mode is enabled using the `vanilla` CCMD:
  * The `am_grid` CVAR is no longer turned `off`.
  * Red blood is now spawned when shooting a lost soul.
  * The value of the `vid_showfps` CVAR is now remembered when disabling vanilla mode.
* When a monster or its corpse is crushed, the color of the resulting gibs now matches the monster’s blood.
* The following changes have been made when playing *DOOM (Shareware)*:
  * `.deh` and `.bex` files may no longer be loaded.
  * *MBF*-compatible helper dogs may no longer be spawned using the `spawn` CCMD.
  * Entering `map random` in the console will no longer warp the player to *E1M4B: Phobos Mission Control* or *E1M8B: Tech Gone Bad*.
* The `con_backcolor` and `con_edgecolor` CVARs have been removed.
* A bug is fixed whereby the colormap applied to monsters could be wrong in some rare instances.
* Sectors without thinkers are no longer interpolated if the `vid_capfps` CVAR is a value other than `35`.
* The screen will now smoothly fade to black if the player has a berserk or radiation shielding suit power-up, the console is opened, the player uses the `quit` CCMD, and the `fade` CVAR is `on`.
* The `vid_borderlesswindow` CVAR is now `off` by default.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, May 29, 2022

### DOOM Retro v4.5.1

* Minor changes have been made to the menu’s background.
* A crash no longer occurs when the `r_ditheredlighting` CVAR is `off`.
* Obituaries displayed when a monster is killed by an explosion are now correct.
* Minor improvements have been made to the color chosen for the surrounding pillarboxes when certain lumps from a PWAD that are only 320 pixels wide are displayed and the `vid_widescreen` CVAR is `on`.
* Blood splats are no longer spawned when using the `kill` CCMD to kill a spectre, or if the `r_corpses_moreblood` CVAR is `off`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, May 28, 2022

### DOOM Retro v4.5

* *DOOM Retro* is now compiled using v17.2.2 of [*Microsoft Visual Studio Community 2022*](https://www.visualstudio.com/vs/).
* *DOOM Retro* now uses [*SDL v2.0.22*](https://www.libsdl.org).
* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Moving the mouse to turn the player is now even smoother.
* Several changes have been made to text that is output to the console.
* Minor improvements have been made to the console’s autocomplete feature.
* Minor changes have been made to the menu’s background.
* The following changes have been made to the external automap:
  * The display used to show the external automap can now be specified using the new `am_display` CVAR, which is `2` by default.
  * Zooming in and out of the external automap is now only possible if the `+zoomin` and `+zoomout` actions are rebound from the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys.
* The player’s stats shown in the automap when the `am_playerstats` CVAR is `on` are now positioned correctly if there are no monsters in the map.
* Text displayed in the top right of the screen while the automap is open is now slightly more translucent.
* The colors of keycards and skull keys in the automap when the `IDDT` cheat is used can now be changed using the new `am_redkeycolor`, `am_yellowkeycolor` and `am_bluekeycolor` CVARs. These CVARs are all `112` by default (the same as the `am_thingcolor` CVAR) and when vanilla mode is enabled.
* When a pain elemental spawns a lost soul, the number of monsters the player has left to kill now increases as intended.
* Blood splats are no longer spawned when a lost soul is killed using the `kill` CCMD.
* Bullet puffs are now positioned correctly when shooting at a lost soul and the `r_blood` CVAR is `none`.
* The `am_followmode` CVAR no longer changes when enabling vanilla mode using the `vanilla` CCMD.
* Changing the `r_fov` CVAR from its default of `90`° is now effective again when the `vid_widescreen` CVAR is `off`.
* The effects of changing the `r_corpses_mirrored` and `r_mirroredweapons` CVARs in the console are now immediate.
* The following changes have been made when the player enters a cheat:
  * The display of the cheat in the console is now redacted.
  * The cheat is now skipped in the console’s input history when pressing the <kbd><b>&uarr;</b></kbd> or <kbd><b>&darr;</b></kbd> keys.
  * A warning is now displayed in the console indicating that the player has cheated.
  * Fade effects are no longer applied when entering most cheats if the `fade` CVAR is `on`.
  * More time has been given to the player to enter the `IDBEHOLDx` cheat.
* The pausing and then resuming of any liquid sectors in view when opening and then closing the console is now smoother.
* Improvements have been made in determining whether something is in liquid or not.
* Dithering is no longer applied to fade transitions if both the `fade` and `r_ditheredlighting` CVARs are `on`.
* The branding in the console is now positioned correctly again when the `vid_widescreen` CVAR is `on`.
* The swirl of liquid sectors when the `r_liquid_swirl` CVAR is `on` has been slowed down slightly to better sync with their bob when the `r_liquid_bob` CVAR is also `on`.
* The player’s currently equipped weapon can now be changed using the new `weapon` CVAR. It can be `fists`, `chainsaw`, `pistol`, `shotgun`, `supershotgun`, `chaingun`, `rocketlauncher`, `plasmarifle` or `bfg9000`.
* The bobbing of power-ups when the `r_floatbob` CVAR is `on` is no longer affected if the ceiling above them is too low.
* The following changes have been made to blood splats:
  * Extensive optimizations have been made to the rendering of blood splats.
  * The amount of blood splats rendered no longer depends on their distance from the player.
  * Blood splats now retain their random shades of color if the `r_blood` CVAR is changed in the console.
  * The translucency of blood splats when the `r_textures` CVAR is `off` now depends on the `r_bloodsplats_translucency` CVAR rather than the `r_translucency` CVAR.
  * Blood splats are now left on the floor as intended if blood falls on a moving sector.
* Centered messages are now spaced better vertically.
* The player’s face in the status bar and widescreen HUD now always looks forward while the console is open.
* Displaying the player’s health as less than `0%` when they die can now be toggled using the new `negativehealth` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* When the player tries to open a locked door that they don’t have the keycard or skull key for, that key now flashes in the status bar as it already does in the widescreen HUD. This feature can also now be disabled using the new `flashkeys` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* A timer set using the `timer` CCMD can now be turned off by using `off` as a parameter.
* Minor improvements have been made to how *MBF*-compatible helper dogs are displayed.
* The following improvements have been made to the support of [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible WADs:
  * The `DMGIGNORED` and `FULLVOLSOUNDS` flags now work correctly.
  * The `JumpIfFlagsSet`, `AddFlags` and `RemoveFlags` code pointers now work correctly.
  * The player can now walk over certain linedefs in *E2M7: Spawning Vats* again.
* The “automap opened” stat shown by the `playerstats` CCMD is now reset at the start of each map as intended.
* The `playerstats` CCMD now displays how many monsters have been telefragged and also how many have respawned.
* The following improvements have been made to obituaries displayed in the console then the `con_obituaries` CVAR is `on`:
  * Obituaries are now displayed whenever the player or a monster is telefragged.
  * Obituaries displayed when a corpse is gibbed now indicate the monster is dead.
* A crash no longer occurs when:
  * An arch-vile is attacked while resurrecting a monster.
  * The player telefrags a monster.
* The player’s plasma rifle is now lit correctly when fired.
* The `map` CCMD can now be used to warp the player to maps up to <code>E9M99</code> and <code>MAP99</code>.
* The `mapstats` CCMD now displays how many linedefs have line specials in the current map.
* Commander Keens are no longer spawned at the start of a map when the `nomonsters` CCMD has been entered in the console, or the `-nomonsters` parameter has been specified on the command-line.
* The following changes have been made to the support of `MAPINFO` lumps:
  * `compat_light` can now be used so when a light level changes to the highest light level found in neighboring sectors, the search is made only for the first tagged sector, like in *Vanilla DOOM*.
  * `nograduallighting` can now be used to disable the effects of the `r_graduallighting` CVAR.
  * `compat_vileghosts` can now be used instead of `compat_coprsegibs`.
* The following changes have been made to the support of `DEHACKED` lumps:
  * A bug is fixed whereby the bounding box of monsters wouldn’t change when using `Width`.
  * `REDTOBLUE` and `REDTOGREEN` now work.
  * If Wolfenstein SS or Commander Keen are changed, and a new `Name` isn’t specified, “monster” will be used in their obituaries in the console.
  * Multiple `DEHACKED` lumps are now parsed in the correct order.
* Flying monsters now spawn at the correct height when using the `spawn` CCMD.
* Sliding corpses can now nudge other corpses when the `r_corpses_nudge` CVAR is `on`.
* Corpses in liquid sectors can now be nudged again when the `r_corpses_nudge` CVAR is `on`.
* Minor improvements have been made to the sliding of decorative corpses and barrels that are too close to an edge.
* Now only the alert and death sounds of cyberdemons and spider masterminds are at full volume.
* Savegame descriptions in the console and player messages are no longer truncated when loading or saving a game.
* Further improvements have been made to the support of [*Freedoom: Phase 1*](https://freedoom.github.io/) and [*Freedoom: Phase 2*](https://freedoom.github.io/).
* The window is now positioned correctly when changing the `vid_windowpos` CVAR to `centered` in the console and the `vid_fullscreen` CVAR is `off`.
* A bug is fixed whereby when the window regains focus, input could be disabled in some instances.
* Blood splats are no longer removed when using the `remove` CCMD to remove all corpses.
* Brightmaps are now applied to more textures when the `r_brightmaps` CVAR is `on`.
* Pressing a mouse button bound to the `+left` action will now cause the player to turn left rather than right.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, March 19, 2022

### DOOM Retro v4.4.10

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Moving the mouse to turn the player is now even smoother.
* Minor changes have been made to text that is output to the console.
* The obituary displayed when a monster kills themselves by exploding a barrel is now correct when the `con_obituaries` CVAR is `on`.
* The movement of the mouse now always affects the direction the menu’s background spins.
* The following improvements have been made to the automap:
  * The automap will now update correctly again if the player moves around while it is open.
  * Text displayed in the top right of the screen while the automap is open is now less translucent.
  * A subtle shadow is now applied to the bottom edge of the automap when the `r_screensize` CVAR is `7` or less.
* The following improvements have been made to the `remove` CCMD:
  * Item and teleport fogs now appear for all things that are removed.
  * Corpses will now be removed along with everything else when the `everything` parameter is used.
  * A type of item may now be specified as a parameter. For example, `remove healthbonuses` will remove all health bonuses from the map.
* Compatibility fixes have been implemented that add blood splats to decorative corpses, and correct the offsets of sprites, in [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens).
* If the player picks up multiples of the same item, and they are all in exactly the same position, the resulting message and gold effect will now be the same as if they picked up only one item.
* If the `r_fov` CVAR is changed from its default of `90`°, it is now only effective when the `vid_widescreen` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, March 14, 2022

### DOOM Retro v4.4.9

* *DOOM Retro* is now compiled using v17.1.1 of [*Microsoft Visual Studio Community 2022*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The amount of blood splats drawn now decreases the further away they are from the player.
* Blood and bullet puffs are now translucent when the `r_textures` CVAR is `off` and the `r_translucency` CVAR is `on`.
* Minor changes have been made to text that is output to the console.
* The player and monsters will now move between two liquid sectors of differing heights smoothly again.
* Sprites in liquid sectors now bob correctly if in view when the player is first spawned into a map and the `r_liquid_bob` CVAR is `on`.
* The main menu now displays correctly when certain PWADs with custom menu lumps are loaded.
* All sound effects now stop playing the moment either the menu or console are opened, or the game is paused.
* The red effect when the player is injured now fades if the menu is opened.
* The red effect when the player is injured now fades quicker than before if the console is opened.
* Use of the mouse is no longer lost if the external automap is shown on another display when the `am_external` CVAR is `on`.
* The new `explode` CCMD is now used instead of the `kill` CCMD to explode all `barrels` or `missiles` in the map.
* There is also a new `remove` CCMD that may be used to remove all `items`, `decorations`, `corpses`, `bloodsplats`, or `everything` in the map.
* Long player messages are now centered rather than truncated when the `r_screensize` CVAR is `7` and the `vid_widescreen` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, March 4, 2022

### DOOM Retro v4.4.8

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Using the mouse to turn the player is now significantly smoother.
* The current map’s title in the automap is no longer translucent when the `r_hud_translucency` CVAR is `off`.
* Minor changes have been made to text that is output to the console.
* Minor improvements have been made to the support of `DEHACKED` lumps.
* Further improvements have been made to the support of [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible WADs.
* Further improvements have been made to the support of [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest).
* *DOOM Retro* now recognizes the presence of `compat_corpsegibs` and `compat_limitpain` in `MAPINFO` lumps.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, February 26, 2022

### DOOM Retro v4.4.7

* Optimizations have been made to further improve the overall performance of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The lighting cast on walls and sprites now appears correctly again when the `r_ditheredlighting` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, February 24, 2022

### DOOM Retro v4.4.6

* Optimizations have been made to further improve the overall performance of *DOOM Retro*.
* The following changes have been made to the support of `MAPINFO` lumps:
  * Instead of only parsing the first `MAPINFO`, `UMAPINFO` or `RMAPINFO` lump that is found in a PWAD, now all of these lumps are parsed if found.
  * `ZMAPINFO` lumps are now also parsed.
* Minor changes have been made to text that is output to the console.
* `.deh` and `.bex` files specified on the command-line are now always parsed correctly.
* Further improvements have been made to the support for [*DOOM 4 VANILLA*](https://www.doomworld.com/forum/topic/108725).
* The widescreen status bar now appears correctly on display resolutions with an aspect ratio less than 16:9.
* The bottom of sprites are no longer clipped while falling above a liquid sector if the `r_liquid_clipsprites` CVAR is `on`.
* The music’s volume is now lowered if the <kbd><b>PAUSE</b></kbd> key is pressed and the `s_lowermusicvolume` CVAR is `on`.
* Music now always stops playing in the background when *DOOM Retro’s* window loses focus and the `s_musicinbackground` CVAR is `off`.
* The `+back`, `+left`, `+right`, `+strafeleft` and `+straferight` actions may now be bound to a mouse button using the `bind` CCMD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, February 18, 2022

### DOOM Retro v4.4.5

* *DOOM Retro* is now compiled using v17.1 of [*Microsoft Visual Studio Community 2022*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance of *DOOM Retro*.
* Crashes no longer occur in the following instances:
  * Sometimes when the player moves at the start of a map while the automap is open and the `am_path` CVAR is `on`.
  * When running *DOOM Retro* as a non-*Steam* game through *Steam* and a controller is connected.
  * When the player exits a map after the music has been changed using a `MUSINFO` lump.
  * When a map contains a linedef with an invalid special.
* Minor changes have been made to text that is output to the console.
* The diagonal pattern of the console’s background now shifts with the rest of the console as it opens and closes.
* Music can now continue to play in the background when *DOOM Retro’s* window loses focus by enabling the new `s_musicinbackground` CVAR, which is `off` by default.
* Further improvements have been made to the support for [*DOOM 4 VANILLA*](https://www.doomworld.com/forum/topic/108725).
* Use of the mouse is no longer lost if the external automap is shown on another display when the `am_external` CVAR is `on`.
* Monsters spawned using the `spawn` CCMD now face the player.
* The intensity of the red effect when the player is injured and the `r_damageeffect` is `on` has increased to be more consistent with *Vanilla DOOM*.
* Dithered lighting is now applied to missing wall textures if the `r_ditheredlighting` CVAR is `on`.
* The gold effect when the player picks up an item now appears correctly in [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest), [*Freedoom*](https://freedoom.github.io/), [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/) and [*REKKR*](http://manbitesshark.com/).
* *DOOM Retro* now recognizes the presence of `allowmonstertelefrags` in `MAPINFO` lumps.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, February 6, 2022

### DOOM Retro v4.4.4

* Optimizations have been made to further improve the overall performance of *DOOM Retro*.
* Controllers connected after *DOOM Retro* has started are now recognized again.
* A bug is fixed whereby the wrong message was displayed when the player picked up ammo in some instances.
* If the player runs out of shells while firing their shotgun, they will now automatically equip their shotgun again rather than their super shotgun when later picking up more shells.
* Minor improvements have been made to the support of `DEHACKED` lumps.
* The widescreen HUD is now positioned slightly higher.
* The following changes have been made when the `r_textures` CVAR is `off` and the `r_translucency` CVAR is `on`:
  * The muzzle flash of the player’s weapon is now translucent.
  * The player’s weapon is now translucent when they have a partial invisibility power-up.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, January 29, 2022

### DOOM Retro v4.4.3

* Optimizations have been made to further improve the overall performance of *DOOM Retro*.
* Crashes no longer occur in the following instances:
  * When loading two or more PWADs without an IWAD using the WAD launcher.
  * When loading a PWAD containing a PNG lump.
  * When sometimes enabling widescreen during a game by pressing the <kbd><b>+</b></kbd> key.
* MIDI music will now actually pause rather than just be muted when *DOOM Retro’s* window loses focus or the <kbd><b>PAUSE</b></kbd> key is pressed.
* Further improvements have been made to the support for controllers:
  * The LEDs on *PS4 DualShock 4* and *PS5 DualSense* controllers now change back to blue when quitting *DOOM Retro*.
  * Left thumbsticks are now bound to the `+alwaysrun` action by default.
  * The paddles on *Xbox Elite* controllers can now be bound to an action by using `paddle1` to `paddle4` with the `bind` CCMD.
  * The touchpad on *PS4 DualShock 4* and *PS5 DualSense* controllers can now be bound to an action by using `touchpad` with the `bind` CCMD.
  * The *Xbox Series X* controller’s share button, the *PS5 DualSense* controller’s microphone button and the *Nintendo Switch* pro controller’s capture button can now be bound to an action by using `misc1` with the `bind` CCMD.
* Minor improvements have been made to the support of [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible WADs.
* Minor changes have been made to text that is output to the console.
* Fade transitions are no longer applied if the console is open and the `fade` CVAR is `on`.
* The player’s teleport fog is now positioned correctly when using the `teleport` CCMD.
* The `armortype` CVAR is now set to `none` if the `armor` CVAR is set to `0` in the console.
* The player’s health, armor or ammo will now always flash in the widescreen HUD if changed using the `health`, `armor` or `ammo` CVARs in the console.
* Removing a behavior originally implemented in [*MBF*](https://doomwiki.org/wiki/MBF), monsters no longer back away from the player if they get too close with their fists or chainsaw equipped.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 14, 2022

### DOOM Retro v4.4.2

* *DOOM Retro* is now compiled using v17.0.5 of [*Microsoft Visual Studio Community 2022*](https://www.visualstudio.com/vs/).
* *DOOM Retro* now uses [*SDL v2.0.20*](https://www.libsdl.org).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A crash no longer occurs if the player makes a typo while entering a filename in the WAD launcher.
* Improvements have been made to the support of [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible maps and mods.
* Improvements have been made to the playback of MIDI music.
* Lowering the music’s volume in the menu and console may now be disabled using the new `s_lowermenumusic` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* Extensive improvements have been made to the support for controllers:
  * Rumble now works again for those controllers that support it.
  * The LEDs on *PS4 DualShock 4* and *PS5 DualSense* controllers now turn red when connected.
  * The `gp_vibrate_barrels`, `gp_vibrate_damage` and `gp_vibrate_weapons` CVARs have been renamed `joy_rumble_barrels`, `joy_rumble_damage` and `joy_rumble_weapons`.
  * All other CVARs that start with `gp_` now start with `joy_`.
  * The `gamepad1` to `gamepad4` parameters used by the `bind` CCMD have been renamed `button1` to `button4`.
* Several minor changes have been made to text that is output to the console.
* Minor improvements have been made to the console’s autocomplete feature.
* A crash no longer occurs when ending a game from the options menu.
* The screen size is now always correct if changed from the options menu while no game is being played.
* The following improvements have been made to the support of [*REKKR*](http://manbitesshark.com/) and [*REKKR: Sunken Land*](https://store.steampowered.com/app/1715690/REKKR_Sunken_Land/):
  * Blood splats now appear around corpses spawned at the start of a map when the `r_corpses_moreblood` CVAR is `on`.
  * The souls of former humans are now translucent when the `r_translucency` CVAR is `on`.
* The corpses of monsters now always fall when close to an edge.
* The shadows of *MBF*-compatible helper dogs are now positioned correctly.
* When the player is injured, their health now flashes in the widescreen HUD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, December 8, 2021

### DOOM Retro v4.4.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor improvements have been made to the support of [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible maps and mods.
* Minor improvements have been made to the support of `DEHACKED` lumps.
* The screen now flashes green as the player’s radiation shielding suit power-up runs out while they also have a berserk power-up and their fists equipped.
* Minor changes have been made to text that is output to the console.
* If the player takes an hour or more to finish a map, “SUCKS” is now positioned correctly on the intermission screen.
* Some elements in the options menu are now positioned correctly for certain PWADs.
* A bug is fixed whereby music wouldn’t play correctly in some instances if the `s_musicvolume` CVAR was set high enough.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, December 3, 2021

### DOOM Retro v4.4

* *DOOM Retro* is now compiled using v17.0.2 of [*Microsoft Visual Studio Community 2022*](https://www.visualstudio.com/vs/).
* *DOOM Retro* now uses [*SDL v2.0.18*](https://www.libsdl.org).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* now supports [*MBF21*](https://doomwiki.org/wiki/MBF21)-compatible maps and mods.
* *DOOM Retro* no longer crashes at startup when trying to load a PWAD containing a PNG lump.
* A bug is fixed whereby `.deh`, `.bex` and `.cfg` files wouldn’t load if selected in the WAD launcher.
* If the player makes a typo when entering a filename in the WAD launcher, and *DOOM Retro* is unable to find what they intended, that text now remains when the launcher reopens so the player may easily correct it.
* The file `midiproc.exe` that allows the volume of MIDI music to be adjusted independently of the sound effects volume is no longer necessary and therefore no longer distributed with *DOOM Retro*.
* The volume of non-MIDI music has been lowered to be consistent with the volume of MIDI music.
* Several changes have been made to text that is output to the console.
* Minor changes have been made to the positioning of the input and branding in the console.
* Further improvements have been made to the support of [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest), [*Freedoom*](https://freedoom.github.io/), [*REKKR*](http://manbitesshark.com/) and [*REKKR: Sunken Land*](https://store.steampowered.com/app/1715690/REKKR_Sunken_Land/).
* The `r_berserkintensity` CVAR is renamed `r_berserkeffect` and is now `3` rather than `2` by default.
* The red effect when the player is injured can now be toggled using the new `r_damageeffect` CVAR, which is both `on` by default and when vanilla mode is enabled.
* The red effect when the player is injured is now also smoother when the new `r_damageeffect` CVAR is `on`.
* The red effect when the player is killed is now more prominent when the new `r_damageeffect` CVAR is `on`.
* If the player is injured when the `r_blood` CVAR is `green` and the new `r_damageeffect` CVAR is `on`, the screen now flashes green.
* The gold effect when the player picks up an item can now be toggled using the new `r_pickupeffect` CVAR, which is both `on` by default and when vanilla mode is enabled.
* The green effect when the player has a radiation shielding suit power-up can now be toggled using the new `r_radsuiteffect` CVAR, which is both `on` by default and when vanilla mode is enabled. If this CVAR is `off`, the screen flashes green to indicate when the power-up is about to run out.
* Fixing a possible oversight in *Vanilla DOOM*, if the player picks up a megasphere when they have 200% green armor, that armor is now upgraded to blue armor.
* The player no longer grunts when trying to open a door that is currently open.
* The player now smoothly slides against two-sided textures.
* Minor improvements have been made to the alternate widescreen HUD.
* Things altered in `DEHACKED` lumps will now always behave correctly.
* Scrolling skies specified in `MAPINFO` lumps are now smoother.
* The number of times the automap is opened by the player is now displayed by the `playerstats` CCMD.
* Player messages are now always grouped in the console, regardless of the time between them, when the `groupmessages` CVAR is `on`.
* The behavior of friendly monsters has improved. They no longer always crowd the player when there are no other monsters nearby.
* Thing triangles in the automap that represent *MBF*-compatible helper dogs are now the correct size again when using the `IDDT` cheat.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, October 15, 2021

### DOOM Retro v4.3

* *DOOM Retro* is now compiled using v16.11.5 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* When *DOOM Retro* is run for the first time, the WAD launcher will now look for installations of *The Ultimate DOOM* and *DOOM II: Hell On Earth* purchased and downloaded using the [*Bethesda.net Launcher*](https://bethesda.net/en/game/bethesda-launcher).
* The WAD launcher will now display and launch files with an `.IWAD` or `.PWAD` extension.
* Minor changes have been made to the splash screen.
* Minor changes have been made to text that is output to the console.
* Minor improvements have been made to the console’s autocomplete feature.
* Further improvements have been made to the support for [*REKKR*](http://manbitesshark.com/).
* Support has also been added for the commercial release, [*REKKR: Sunken Land*](https://store.steampowered.com/app/1715690/REKKR_Sunken_Land/).
* A new `am_playerstats` CVAR has been implemented that toggles player stats in the automap. When this CVAR is `on`, the number of monsters the player has killed, the items they have picked up and the secrets they have found will be displayed while the automap is open. This CVAR is both `off` by default and when vanilla mode is enabled.
* The distance the player has traveled in the current map is now displayed in the top right corner of the external automap when the `am_path` CVAR is `on`.
* Rotating the automap is now smoother when the `am_rotate` CVAR is `on`.
* All lines in the automap will now be displayed correctly when the player has used the `IDDT` cheat.
* A bug has been fixed whereby some sound effect lumps in the [WAV format](https://en.wikipedia.org/wiki/WAV) wouldn’t play at all.
* The following changes have been made when a monster is crushed to death by a lowering ceiling:
  * Cacodemons, barons of hell and hell knights will now leave gibs.
  * All gibs will now be green if the `r_blood` CVAR is `green`.
  * The gibs of spectres will now be red if the `r_blood` CVAR is `red` or `nofuzz`.
  * An obituary is now displayed in the console when the `con_obituaries` CVAR is `on`.
* The title in the automap has been moved slightly in some instances.
* A dead player’s negative health will now be displayed in the status bar as it is in the widescreen HUD.
* The items that monsters drop when they are killed will now also move slightly if walked over when the `r_corpses_nudge` CVAR is `on`.
* The number of monsters resurrected is now displayed by the `playerstats` CCMD.
* The “monsters killed by infighting” stat is now reset at the start of each map as intended.
* A bug has been fixed whereby the `facebackcolor` CVAR wouldn’t be reset to its default of `5` if vanilla mode was enabled.
* The background of the player’s face will now be a better size when the `facebackcolor` CVAR is a value other than its default of `5`.
* When vanilla mode is enabled, any timer set using the `timer` CCMD will now be reset to `0`, rather than being disabled altogether.
* The external automap will now be displayed correctly when the `vid_borderlesswindow` CVAR is `off`.
* The `vid_scaleapi` CVAR is now `“direct3d”` by default.
* Minor improvements have been made to the support of `MAPINFO` lumps.
* Blood splats are now translucent when the `r_translucency` CVAR is `on` and the `r_textures` CVAR is `off`.
* Turning left and right using a gamepad is now smoother.
* A bug has been fixed whereby switching from a window to fullscreen by pressing <kbd><b>ALT</b></kbd> + <kbd><b>ENTER</b></kbd> would affect the aspect ratio if the `vid_borderlesswindow` CVAR was `off`.
* Translucent things now cast more translucent shadows.
* Dithered lighting is now cast on *BOOM*-compatible translucent wall textures.
* Things on scrolling floors will no longer continue to move while the menu is open.
* When the player drops down from a great height, their weapon will no longer move too far downwards when the `weaponbounce` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, September 19, 2021

### DOOM Retro v4.2.3

* *DOOM Retro* is now compiled using v16.11.3 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The splash screen’s animation is now smoother.
* Minor changes have been made to the font used in the console, and the player messages in the alternate widescreen HUD.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the support for [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest), [*Freedoom*](https://freedoom.github.io/) and [*REKKR*](http://manbitesshark.com/).
* The bounce of the player’s weapon when dropping down from a greater height is now slightly slower when the `weaponbounce` CVAR is `on`.
* When clicking a mouse button that is bound to the `+screenshot` action, the screen will now flash and a sound will now be heard.
* The console’s background will no longer be affected when entering the `vanilla` CCMD.
* A fade transition is now applied after saving a game in the save game menu and the `fade` CVAR is `on`.
* Further improvements have been made to when the player’s view should be lowered if the `r_liquid_lowerview` CVAR is `on`.
* A bug has been fixed whereby the title in the automap could be positioned incorrectly in some rare instances.
* The player’s arrow will now be positioned correctly in the automap when the `am_rotate` CVAR is `on` and the `am_followmode` CVAR is `off`.
* The following changes have been made when the `am_path` CVAR is `on`:
  * The player’s path in the automap will now still be updated when either freeze mode or no clipping mode is enabled.
  * The player’s path will now be positioned correctly in the automap when the `am_rotate` CVAR is `on` and the `am_followmode` CVAR is `off`.
  * The distance the player has traveled in the current map is now displayed in the top right corner of the automap.
* A bug has been fixed whereby the timer displayed when using the `timer` CCMD could be positioned incorrectly in some rare instances.
* The size of the crosshair has been reduced when the `crosshair` CVAR is `cross` and the `r_detail` CVAR is `low`.
* The message displayed when entering the `IDMYPOS` cheat will now be visible when the `fade` CVAR is `on`.
* The sky will now be stretched if the `weaponrecoil` CVAR is `on` and the `r_screensize` CVAR is `8`.
* Sprites in liquid sectors now bob correctly when the player is first spawned into a map and the `r_liquid_bob` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, September 3, 2021

### DOOM Retro v4.2.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The menu’s background animation is now smoother.
* Further improvements have been made to the support for [*REKKR*](http://manbitesshark.com/).
* A bug has been fixed whereby the automap wouldn’t be updated as the player moves around while it is open.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, September 1, 2021

### DOOM Retro v4.2.1

* *DOOM Retro* is now compiled using v16.11.2 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The framerate will no longer drop to 35 frames per second if the game is paused, or the menu or console is open.
* A slight dithered effect is now applied to fade transitions when the `fade` CVAR is `on`.
* A fade transition will now always be applied when exiting the help screen and the `fade` CVAR is `on`.
* Translucency is now applied to megaspheres when the `r_translucency` CVAR is `on`.
* The following changes have been made when the `r_shake_damage` CVAR is greater than `0%` and god mode is enabled:
  * The maximum amount of time the screen will shake when the player is attacked has been reduced.
  * The screen will no longer shake if the player is in a sector with special 16 (“Damage -10% or -20% health”) or 4 (“Damage -10% or -20% health and light blinks (0.5 sec.)”).
* Improvements have been made to the clipping of the bottom of sprites when in liquid and the `r_liquid_clipsprites` CVAR is `on`.
* A bug has been fixed whereby an action couldn’t be bound to or unbound from the <kbd><b>;</b></kbd> key using the `bind` and `unbind` CCMDs.
* The background of the player’s face will now be positioned correctly when the `facebackcolor` CVAR is a value other than its default of `5` and the `vid_widescreen` CVAR is `on`.
* Zooming in and out of the automap is now more responsive.
* The `M_LGTTL` and `M_SGTTL` lumps will now be used as the titles in the load and savegame menus if replaced in a PWAD.
* Further improvements have been made to the support for [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest) and [*REKKR*](http://manbitesshark.com/).
* A bug has been fixed whereby par times weren’t being displayed on the intermission screen in some instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, August 12, 2021

### DOOM Retro v4.2

* *DOOM Retro* is now compiled using v16.11.0 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* *DOOM Retro* now uses [*SDL v2.0.16*](https://www.libsdl.org).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The following changes have been made to the dithered lighting cast when the `r_ditheredlighting` CVAR is `on`:
  * The dithered lighting cast on textures is now more consistent regardless of the player’s viewing angle.
  * Dithered lighting is now also cast on sprites.
  * Dithered lighting is now cast when the `r_detail` CVAR is `low`.
  * Dithered lighting is now cast on *BOOM*-compatible translucent wall textures if the `r_translucency` CVAR is `off`.
* The text in the console has been brought in from the left and right edges of the screen when the `vid_widescreen` CVAR is `on`.
* Minor changes have been made to the character set used in the console.
* Several changes have been made to text that is output to the console.
* Minor improvements have been made to the console’s autocomplete feature.
* The following changes have been made to the support for [*REKKR*](http://manbitesshark.com/):
  * `rekkrsa.wad` is now treated as an IWAD (that is, `DOOM.WAD` will no longer be automatically loaded with it).
  * The correct monster and weapon names are now used in obituaries in the console when the `con_obituaries` CVAR is `on`.
  * Sorrows and skelly bellies will now only bleed red blood.
  * Gamepads will no longer vibrate when the axe is equipped and idle, and the `gp_vibrate_weapons` CVAR is `on`.
* A bug has been fixed whereby any palette effects wouldn’t be restored when unpausing a game by pressing the <kbd><b>ESC</b></kbd> key rather than <kbd><b>PAUSE</b></kbd> key.
* The `alwaysrun` CVAR can now be toggled by pressing the <kbd><b>CAPSLOCK</b></kbd> key while on the title screen.
* The fading of player messages onto and off of the screen is now smoother in some instances when the `fade` CVAR is `on`.
* A fade transition will now be applied when changing the `r_brightmaps` CVAR, or any CVARs that change a color, when the `fade` CVAR is `on`.
* Corpses and dropped items will no longer perpetually slide back and forth on the floor in some rare instances.
* The maximum number of blood splats that can be spawned when a corpse slides on the floor and the `r_corpses_slide` CVAR is `on`, has now been reduced.
* The number of monsters killed due to infighting is now displayed by the `playerstats` CCMD.
* The colors of doors in the automap that are unlocked using a keycard or skull key can now be changed using the new `am_reddoorcolor`, `am_yellowdoorcolor` and `am_bluedoorcolor` CVARs. These CVARs are `160` by default (such that there is no apparent difference until one of them is changed), and `231` when vanilla mode is enabled.
* Minor improvements have been made to rendering the automap when the player has a computer area map power-up.
* The following changes have been made to the external automap when the `am_external` CVAR is `on`:
  * When the <kbd><b>F</b></kbd> key is pressed to turn follow mode off, the player may now pan around the external automap as intended.
  * The mouse wheel will no longer zoom in/out of the external automap, and instead only cycle through the player’s weapons.
* A bug has been fixed whereby the player’s weapon wouldn’t always be lit correctly when fired.
* A previously implemented feature that caused monsters to not be fullbright when firing and facing away from the player now works correctly.
* Spectres will now always be rendered correctly when freeze mode is on.
* More fixes have been applied to certain maps when the `r_fixmaperrors` CVAR is `on`.
* Minor changes have been made to the positioning of some elements in the widescreen HUD.
* Changing the `r_screensize` CVAR in the console will now also change the `vid_widescreen` CVAR as necessary.
* The following changes have been made when on the intermission screen:
  * The `playerstats` and `mapstats` CCMDs will now work.
  * Pressing the <kbd><b>ENTER</b></kbd> key while the menu or console are also open will no longer trigger the intermission screen.
* The `+weapon1` to `+weapon7` actions can now be bound to a mouse button using the `bind` CCMD.
* The effect applied when the player is in a damaging sector and the `r_shake_damage` CVAR is greater than `0%` is now no longer applied when god mode is enabled.
* A bug has been fixed whereby the “Damage received” stat viewed using the `playerstats` CCMD would continue to update when god mode was on.
* The `toggle` CCMD now supports CVARs that accept multiple values. The value of these CVARs will increment by 1, and if their maximum value is reached, will wrap to their minimum.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, June 21, 2021

### DOOM Retro v4.1.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby some CVARs wouldn’t be initialized correctly when *DOOM Retro* was run for the first time.
* Player messages will now fade onto the screen when the `fade` CVAR is `on`.
* Minor changes have been made to text that is output to the console.
* The background will now be displayed correctly when resizing the window during intermission.
* The framerate will now be properly capped at 35 frames per second, rather than just interpolation being disabled, when the `vid_capfps` CVAR is `35`.
* The framerate will now drop to 35 frames per second if the game is paused, or the menu or console is open.
* The following changes have been made to the external automap when the `am_external` CVAR is `on`:
  * The <kbd><b>F</b></kbd> key can now be pressed to toggle follow mode.
  * Fade transitions will no longer be applied to the main display when pressing some keys to control the external automap and when the `fade` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, June 18, 2021

### DOOM Retro v4.1.2

* *DOOM Retro* is now compiled using v16.10.2 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The height of the console during a game has been increased to allow two additional lines of text.
* A fade transition will now be applied when changing the `r_fov` CVAR in the console and when the `fade` CVAR is `on`.
* Minor changes have been made to text that is output to the console.
* Obituaries will now be grouped again in the console, but only when the `groupmessages` CVAR is `on`.
* The mapping errors that are fixed when the `r_fixmaperrors` CVAR is `on` are no longer applied to the maps in *DOOM (Shareware)*.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, June 13, 2021

### DOOM Retro v4.1.1

* *DOOM Retro* is now compiled using v16.10.1 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The smoke trailing behind rockets fired by the player and cyberdemons when the `r_rockettrails` CVAR is `on` is now displayed correctly when the `r_textures` CVAR is `off`.
* A fade transition will now be applied when changing the `r_detail` or `r_playersprites` CVARs in the console and when the `fade` CVAR is `on`.
* Minor changes have been made to text that is output to the console.
* The following changes have been made to the external automap when the `am_external` CVAR is `on`:
  * Player messages will now always be displayed correctly.
  * The external automap will now be hidden while the menu or help screen is open on the main display.
  * The <kbd><b>0</b></kbd> key can now be pressed to toggle maximum zoom.
  * If the `+zoomin` and `+zoomout` actions are rebound from the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys, the external automap may be zoomed in and out.
* Player messages will now fade off the screen while the console is open and the `fade` CVAR is `on`.
* Player messages now fade off the screen smoother in the alternate widescreen HUD when the `fade` CVAR is `on`.
* If a filename is specified using the `condump` CCMD, `.txt` will be added to the filename if it has no extension, and the resulting file will now be saved in the `console` subfolder.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, June 5, 2021

### DOOM Retro v4.1

* *DOOM Retro* is now compiled using v16.10 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The correct credits screen will now be displayed in all instances.
* Minor changes have been made to text that is output to the console.
* The lighting applied to all walls, floors and ceilings in a map is now dithered. This may be toggled using the new `r_ditheredlighting` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
* The `r_dither` CVAR has been deprecated. *BOOM*-compatible translucent wall textures can no longer be dithered.
* The smoke trailing behind rockets fired by the player and cyberdemons when the `r_rockettrails` CVAR is `on` has been redesigned.
* The smoke trailing behind rockets fired by the player will no longer spawn too close to their face, obscuring their view, when they hold down a control bound to the `+fire` action to fire multiple rockets.
* The following changes have been made to player messages:
  * The grouping of identical player messages can now be toggled using the new `groupmessages` CVAR, which is `on` by default and `off` when vanilla mode is enabled.
  * Player messages are no longer grouped if more than 4 seconds apart.
  * Player messages now fade off the screen smoother when the `fade` CVAR is `on`.
  * A bug has been fixed whereby player messages wouldn’t completely fade off of the screen if the console was open and the `fade` CVAR was `on`.
* Any momentum applied to the player will now be removed when enabling freeze mode.
* Minor improvements have been made to the support of [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest) and [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* The power-up sound effect will no longer be played if the player picks up a berserk power-up and they already have one.
* The player will no longer make a noise if they drop down from a ledge when either freeze mode or no clipping mode is enabled.
* A bug has been fixed whereby the alert sounds of cyberdemons or spider masterminds wouldn’t be played.
* The maximum number of blood splats that can be spawned when a corpse slides on the floor and the `r_corpses_slide` CVAR is `on`, is now based on its size.
* A bug has been fixed whereby blood splats could be the wrong color in some instances.
* Invulnerability and partial invisibility power-ups will now respawn when the `respawnitems` CCMD is used.
* Minor changes have been made to the shadows cast by monsters when the `r_shadows` CVAR is `on`.
* Blood splats are now slightly more translucent when the `r_bloodsplats_translucency` CVAR is `on`.
* The shaking effect applied when the player is in a damaging sector and the `r_shake_damage` CVAR is greater than `0%` is now still applied when god mode is enabled.
* If the WAD file entered manually in the WAD launcher isn’t found, and *DOOM Retro* then successfully guesses what was intended by finding the nearest match, the `wad` CVAR will now be updated.
* The `r_hud` CVAR is now `off` by default.
* The default of the `r_bloodsplats_max` CVAR has now been doubled to `131,072`, and is now also set to `0` when vanilla mode is enabled.
* A new `toggle` CCMD has been implemented that can be used to toggle the value of CVARs between `on` and `off`.
* The translucency of item and teleport fogs is now additive when the `r_translucency` CVAR is `on`.
* After loading a savegame, a crash will no longer occur if a blood splat is removed because the sector it is on becomes liquid.
* The effects of changing the `vid_borderlesswindow` CVAR in the console are now immediate.
* The transitions between some screens when the `fade` CVAR is `on` are now smoother.
* Minor improvements have been made to the support of `DEHACKED` lumps.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, April 30, 2021

### DOOM Retro v4.0.9

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor improvements have been made to some title screens.
* Minor changes have been made to text that is output to the console.
* A dead player’s negative health will now always be positioned correctly in the widescreen HUD.
* A crash will no longer occur during the finale at the end of the third episode of *DOOM*.
* The `r_blood` CVAR is now `nofuzz` by default.
* A bug has been fixed whereby the `r_hud` CVAR would be left `on` when changing the `r_screensize` CVAR in the console in some instances.
* A sound is now made when changing the `r_screensize` CVAR in the console.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, April 16, 2021

### DOOM Retro v4.0.8

* *DOOM Retro* is now compiled using v16.9.4 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Minor changes have been made to text that is output to the console.
* The `condump` CCMD will no longer continue to dump every line of text that is output to the console after it is entered.
* Minor improvements have been made to the `DMENUPIC` lump.
* Minor changes have been made to the help screen displayed using the <kbd><b>F1</b></kbd> key.
* A bug has been fixed whereby the console couldn’t be opened using the <kbd><b>~</b></kbd> key on certain keyboards.
* The player will no longer be spawned into a map such that they could be stuck in the ceiling.
* Player messages will now quickly fade off of the screen if the `fade` CVAR is `on` when the alternate widescreen HUD is displayed.
* The `vid_showfps` CVAR can now be `on` at startup if set in a `.cfg` file that is either loaded using the WAD launcher or specified on the command-line.
* Dead monsters may now be spawned again using the `spawn` CCMD.
* A bug has been fixed whereby things with the `MF_BOUNCES` flag wouldn’t explode.
* More fixes have been applied to certain maps when the `r_fixmaperrors` CVAR is `on`.
* `-shot` may be now be used as an alternative to `-shotdir` on the command-line.
* A bug has been fixed whereby corpses wouldn’t gib correctly in some rare instances if the `r_corpses_gib` CVAR was `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, April 2, 2021

### DOOM Retro v4.0.7

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* is now compiled using v16.9.3 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Even wider versions of the title, credits, intermission and finale screens will now be used on ultra-wide displays when the `vid_widescreen` CVAR is `on`.
* *DOOM Retro’s* window can now be resized properly if the `vid_fullscreen` CVAR is `off` and the `vid_widescreen` CVAR is `on`.
* Minor changes have been made to text that is output to the console.
* The <kbd><b>F5</b></kbd> key may now be used to toggle the graphic detail while the automap is open and the status bar is visible (that is, when the `r_screensize` CVAR is less than `8`).
* Player messages will now quickly fade off of the screen if the `fade` CVAR is `on`.
* The `messages` CVAR is now `on` by default.
* Any screen shake will now be canceled when warping to another map using the `IDCLEV` cheat or loading a savegame in some instances.
* The credits screen will now be displayed for the same amount of time as the title screen.
* The correct title screen is now displayed when playing *DOOM (Shareware)*, *Final DOOM: The Plutonia Experiment* or *Final DOOM: TNT - Evilution*.
* The *id Software* logo is no longer missing from certain title screens.
* The correct credits screen is now displayed for the registered version of *DOOM*.
* *Vanilla DOOM’s* notorious [blockmap bug](http://doom2.net/doom2/research/things.html) has now finally been fixed.
* Par times are now shown on the intermission screen when playing [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest).
* A bug has been fixed whereby some sound effect lumps in the [WAV format](https://en.wikipedia.org/wiki/WAV) wouldn’t play correctly.
* The pitch of sounds made by monster projectiles are no longer randomized if the `s_randompitch` CVAR is `on`.
* The player’s weapon sprite is no longer cut off when reloading the super shotgun after firing it, and if the `vid_widescreen` CVAR is `on` and the `r_screensize` CVAR is `8`.
* Minor improvements have been made to the support of [*Freedoom: Phase 2*](https://freedoom.github.io/).

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, March 14, 2021

### DOOM Retro v4.0.6

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* is now compiled using v16.9.1 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* *DOOM Retro* now recognizes the following alternative names for the IWADs it supports: `DOOMU.WAD`, `BFGDOOM.WAD`, `DOOMBFG.WAD`, `DOOMUNITY.WAD`, `DOOM2F.WAD`, `BFGDOOM2.WAD`, `DOOM2BFG.WAD`, `DOOM2UNITY.WAD`, `PLUTONIAUNITY.WAD` and `TNTUNITY.WAD`.
* A bug has been fixed whereby certain lumps that are only 320 pixels wide wouldn’t be positioned correctly in some instances if the `vid_widescreen` CVAR was `on`.
* Minor improvements have been made to the color chosen for the surrounding pillarboxes when certain lumps from a PWAD that are only 320 pixels wide are displayed and the `vid_widescreen` CVAR is `on`.
* A wider version of the `DMENUPIC` lump will now be displayed, and the menu will automatically open at startup again, when playing the *BFG Edition* of *DOOM II: Hell On Earth*.
* Minor changes have been made to text that is output to the console.
* An obituary will now be displayed in the console if the `con_obituaries` CVAR is `on` and:
  * A barrel explodes because of another barrel’s explosion.
  * A corpse is gibbed by a nearby barrel or rocket explosion.
* Minor improvements have been made to the support of [*Freedoom: Phase 1*](https://freedoom.github.io/), [*Freedoom: Phase 2*](https://freedoom.github.io/) and [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* Minor improvements have been made to the support of `DEHACKED` lumps.
* A bug has been fixed whereby some characters in the automap title wouldn’t be displayed correctly in some instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, March 5, 2021

### DOOM Retro v4.0.5

* *DOOM Retro* is now compiled using v16.9 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Minor changes have been made to text that is output to the console.
* The finale at the end of the third episode of *DOOM* is now displayed correctly.
* You may no longer toggle widescreen using the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys, or open the help screen using the <kbd><b>F1</b></kbd> key, when on an intermission or finale screen.
* Certain fade transitions are now applied to the finales of both *DOOM* and *DOOM II: Hell On Earth* when the `fade` CVAR is `on`.
* When displaying certain lumps from a PWAD that are only 320 pixels wide, and the `vid_widescreen` CVAR is `on`, the color of the surrounding pillarboxes will now be based on the most used color along the left and right edges of those lumps, rather than always be black.
* A bug has been fixed whereby the wrong credits screen would be displayed in some instances.
* The background of the help screen displayed using the <kbd><b>F1</b></kbd> key now won’t animate.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, February 26, 2021

### DOOM Retro v4.0.4

* *DOOM Retro* is now compiled using v16.8.6 of [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* SSAA (supersampling anti-aliasing) is now still applied if the `r_detail` CVAR is `low`, the `r_supersampling` CVAR is `on`, but the `r_lowpixelsize` CVAR is not its default of `2×2`.
* The `r_lowpixelsize` CVAR can no longer be `1×1`.
* Further improvements have been made in supporting older, non-widescreen displays.
* Minor changes have been made to text that is output to the console.
* Minor changes have been made to the help screen displayed using the <kbd><b>F1</b></kbd> key.
* Minor improvements have been made to the status bar when the `r_detail` CVAR is `high`.
* The spacing of the map title in the automap has been improved.
* The corpses of monsters will now still be spawned at the start of a map when the `nomonsters` CCMD has been entered in the console, or the `-nomonsters` parameter has been specified on the command-line.
* The effects of changing the `con_edgecolor` CVAR in the console are now immediate.
* The following changes have been made when a pain elemental spawns a lost soul:
  * If a lost soul is spawned outside of a map, an obituary will no longer appear in the console if the `con_obituaries` CVAR is `on`.
  * The lost souls are now included in the stats displayed by the `playerstats` CCMD, and on the intermission screen once the player has finished a map.
* The correct background is now displayed on the intermission screen when playing the 4th episode of *The Ultimate DOOM*.
* The menu will no longer automatically open at startup when playing the *BFG Edition* of *DOOM II: Hell On Earth*.
* A bug has been fixed whereby the wrong `TITLEPIC` lump would be displayed in some instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, February 19, 2021

### DOOM Retro v4.0.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* will no longer crash on older, non-widescreen displays.
* A fade transition will now be applied when using the `am_path` CVAR in the console to toggle the player’s path in the automap and the `fade` CVAR is `on`.
* Gamepads will now briefly vibrate during startup to indicate they are connected and support vibration, if any of the `gp_vibrate_barrels`, `gp_vibrate_damage` or `gp_vibrate_weapons` CVARs are greater than `0%`.
* Player messages and the widescreen HUD are now slightly less translucent when the `r_hud_translucency` CVAR is `on`.
* Minor changes have been made to some player messages.
* The following changes have been made to *DOOM II’s* cast sequence:
  * The player now appears again.
  * The spectre will now always be positioned correctly while dying.
  * The double shotgun sound can now be heard again.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, February 12, 2021

### DOOM Retro v4.0.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The following changes have been made to the external automap:
  * A bug has been fixed whereby the automap wouldn’t be drawn correctly after changing the `am_external` CVAR in the console.
  * Map titles are now displayed again in the external automap.
  * The external automap now displays correctly when the menu is open.
  * The zoom level is now reset when the external automap is opened.
  * The <kbd><b>0</b></kbd> key can no longer be pressed to toggle maximum zoom if the external automap is open.
* The automap title and all player messages are now full scale when the `r_screensize` CVAR is `8` and the `r_althud` CVAR is `off`.
* The automap will now be displayed correctly if it is open while adjusting the screen size in the options menu.
* Player messages are now slightly more translucent when the `r_hud_translucency` CVAR is `on`.
* Fade transitions will now be applied in the following instances when the `fade` CVAR is `on`:
  * When pressing the <kbd><b>0</b></kbd> key in the automap to toggle maximum zoom.
  * When using the `r_textures` CVAR in the console to toggle textures.
  * When the player picks up an invulnerability power-up.
  * When the player picks up a computer area map power-up and the automap is open.
* The `IDDT` cheat can no longer be entered when playing using the *Nightmare!* skill level.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, February 5, 2021

### DOOM Retro v4.0.1

* A bug has been fixed whereby sprites would be incorrectly drawn in front of masked midtextures in some instances.
* The external automap now displays correctly when the `am_external` CVAR is `on`.
* Minor changes have been made to text that is output to the console.
* Music will now play again when loading a savegame created using *DOOM Retro v3.6* or *v3.6.1*.
* If the player starts a new game from the menu while playing *E1M4B: Phobos Mission Control* or *E1M8B: Tech Gone Bad*, the correct map will now be loaded.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, January 31, 2021

### DOOM Retro v4.0

* *DOOM Retro* now uses [*SDL v2.0.14*](https://www.libsdl.org).
* *DOOM Retro* now uses the [*Windows Audio Session API (WASAPI)*](https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi) rather than the deprecated [*DirectSound* API](https://en.wikipedia.org/wiki/DirectSound). Sound effects are now louder and clearer than before.
* Minor improvements have been made to *DOOM Retro’s* splash screen.
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Non-QWERTY keyboards are now properly supported.
* *DOOM Retro* now has improved support for wide and ultra-wide displays:
  * When the `r_screensize` CVAR is `7` and the `vid_widescreen` CVAR is `on`, a new widescreen mode complete with status bar will be displayed that horizontally fills displays of any aspect ratio.
  * When the `r_screensize` CVAR is changed to it’s new maximum value of `8`, a widescreen mode with an optional HUD instead of a status bar, and a greater vertical field of view, will now be displayed.
  * You may still press the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys during a game to toggle these new widescreen modes.
  * The <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys may now also be pressed on the title screen to toggle widescreen mode.
  * Certain [official widescreen assets](https://bethesda.net/en/game/doom-widescreen-mods), such as those for the title screen and status bar, will now be displayed if the `vid_widescreen` CVAR is `on`.
  * The default of the `vid_windowsize` CVAR has been changed from `768×480` to `854×480`.
* The following changes have been made to support the IWADs from the latest rereleases of [*The Ultimate DOOM*](https://store.steampowered.com/app/2280/Ultimate_Doom/) and [*DOOM II: Hell On Earth*](https://store.steampowered.com/app/2300/DOOM_II/) on *Steam*:
  * The WAD launcher will now look for these new IWADs when *DOOM Retro* is run for the first time.
  * The par times for E1M8, E2M8 and E3M8 have changed.
* A crash will no longer occur when trying to display a patch with a height greater than 200 pixels. These patches will now be cropped instead.
* Centered messages are now better centered vertically.
* The player’s gender identity can now be specified using the new `playergender` CVAR. It can be `male`, `female` or `other`, and is `male` by default. If the `playername` CVAR is changed from its default of `“you”` as well, the pronouns used in several player messages and obituaries will be affected.
* The following changes have been made to the menu:
  * A subtle scanline effect is now applied to the menu’s background.
  * When opening the menu during a game, a brief deceleration effect is now applied to the player’s rotating view in the menu’s background.
  * A bug has been fixed whereby the sky in the menu’s background wouldn’t be stretched correctly in some instances when the `mouselook` CVAR was `on`.
  * Navigation of the main menu will now behave correctly when there are no savegames for the currently loaded IWAD or PWAD.
  * A bug has been fixed whereby using the left mouse button to navigate the menu could cause the player to continuously fire when starting a new game.
  * The <kbd><b>F5</b></kbd> key may now be pressed to toggle the graphic detail while the menu is open.
  * Screenshots may now be taken while entering a savegame description in the save game menu.
* If the player has their fists equipped, a berserk power-up, and god mode is enabled, the screen will no longer flash red when they are attacked.
* Monsters may now walk under other flying monsters when the `infiniteheight` CVAR is `off`.
* The following changes have been made to the console:
  * The scrollbar now extends to the top of the screen.
  * The scrollbar’s grip is now translucent.
  * A slight shadow is applied to the text along the top of the console.
  * Minor changes have been made to text that is output to the console.
  * Long lines of text will now always wrap to the next line rather than be truncated.
  * Timestamps during the hour after midnight will now be displayed correctly.
* The following changes have been made to the automap:
  * A bug has been fixed whereby marks in the automap would reappear in wrong positions when panning far enough to the left or right.
  * Panning in the automap is now restricted to the dimensions of the current map when the `am_rotate` CVAR is `on`.
  * The player arrow is now slightly more translucent when the player has a partial invisibility power-up.
  * Thing triangles will no longer be affected by frame interpolation when the `IDDT` cheat has been entered, the console is open, and the `vid_capfps` CVAR is not `35`.
  * Fade transitions will now be applied when performing various actions in the automap when the `fade` CVAR is `on`.
  * A smaller crosshair is now displayed when the `am_followmode` CVAR is `off`.
  * The player’s path is now thinner when the `am_path` CVAR is `on`.
* The correct `CREDIT` lump is now displayed when finishing any of the first four episodes of *The Ultimate DOOM*, and *SIGIL* has been automatically loaded.
* The intermission screens displayed once the player has finished a map will now always transition correctly when the `fade` CVAR is `on`.
* The `wipe` CVAR has been renamed to `melt`.
* Fade transitions will now be applied when a melt transition normally would, but the `melt` CVAR is `off` and the `fade` CVAR is `on`.
* The default of the `s_musicvolume` CVAR has been increased from `67%` to `100%`.
* Further improvements have been made to the support of both `DEHACKED` and `UMAPINFO` lumps.
* *DeHackEd* support has been extended further to allow for an additional 200 sound effects (numbered 500 to 699, and named `DSFRE000` to `DSFRE199`).
* Music that has been changed because of a [`MUSINFO`](https://doomwiki.org/wiki/MUSINFO) lump is now remembered in savegames.
* The player’s health and ammo will now only flash in the widescreen HUD (or change color in the alternate widescreen HUD) when less than 10.
* More blood is spawned when the player is injured.
* Minor improvements have been made to the rendering of blood splats in some instances.
* The movement of lifts is now smoother in some instances.
* Improvements have been made in determining if the player or a monster is standing in liquid or not.
* Monsters will no longer unnecessarily drop from high ledges.
* The current map’s music will no longer restart when loading a savegame for the same map.
* Music will no longer continue to play if *DOOM Retro* crashes.
* Player messages are now slightly translucent again when the `r_hud_translucency` CVAR is `on` and the `vid_widescreen` CVAR is `off`, but not while vanilla mode is enabled.
* SSAA (supersampling anti-aliasing) is now applied to the help screen’s background when the <kbd><b>F1</b></kbd> key is pressed.
* Minor improvements have been made to the status bar when the `r_detail` CVAR is `high`.
* The bezel around the player’s view when the `r_screensize` CVAR is less than `7` is now only displayed if either all or none of the relevant graphics have been replaced in a PWAD.
* The number of times the player uses their fists and chainsaw are now displayed by the `playerstats` CCMD. The player’s fists and chainsaw may now also then be displayed as their `Favorite weapon`.
* More fixes have been applied to certain maps when the `r_fixmaperrors` CVAR is `on`.
* The crosshair will now be larger when the `crosshair` CVAR is `cross` or `dot`, and the `r_detail` CVAR is `low`.
* Improvements have been made to the fuzz effect of the player’s weapon when they have a partial invisibility power-up.
* A spectre now appears again in *DOOM II’s* cast sequence.
* A bug has been fixed whereby the player would move slightly slower when using a gamepad rather than the keyboard.
* The `+console` action may now be bound to a control on a gamepad.
* Improvements have been made when the player uses the `kill` CCMD to commit suicide.
* The blood of barons of hell and hellknights is slightly lighter.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, August 16, 2020

### DOOM Retro v3.6.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The following changes have been made to the automap:
  * A bug has been fixed whereby the <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys wouldn’t zoom the automap in and out.
  * The background menu effect will no longer be applied to the external automap when not in a game.
* Minor changes have been made to text that is output to the console.
* Navigating the options menu has now been fixed.
* Elevators will now make a sound again when they move.
* The player’s face in the status bar and widescreen HUD will no longer change, and armor will no longer be lost, if the player is attacked while god mode is enabled.
* `ENDGAMEC` is now supported in `MAPINFO` lumps.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, August 9, 2020

### DOOM Retro v3.6

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby a crash could occur when saving a game in some instances. Unfortunately, because of this important fix, the format of savegames has needed to change, breaking compatibility with previous versions of *DOOM Retro*.
* The gradual lighting applied under open doors and crushing ceilings is now correctly restored in savegames when the `r_graduallighting` CVAR is `on`.
* A subtle fade transition is now applied in certain situations, such as when toggling the menu or automap. This effect may be disabled using the new `fade` CVAR, which is `on` by default, and `off` when vanilla mode is enabled.
* The screen will now flash white when taking a screenshot by pressing the <kbd><b>PRINTSCREEN</b></kbd> key.
* Minor changes have been made to the console’s scrollbar.
* The following changes have been made to the menu’s background:
  * A subtle dithered effect is now applied.
  * Animated textures and changes to lighting will now still update.
  * The animation of liquid sectors when the `r_liquid_swirl` CVAR is `on` has been slowed down.
* The following changes have been made to the text caret in the savegame menu:
  * It now matches the height of the accompanying text.
  * Its color now better matches that of the accompanying text in some instances.
* The <kbd><b>BACKSPACE</b></kbd> key may now be used to cancel centered messages.
* Minor improvements have been made when mistyping a filename in the WAD launcher.
* The randomization of certain features (such as the amount of damage inflicted on and by the player, and the trajectories of the player’s gunshots) now more closely resembles what occurs in *Vanilla DOOM*.
* Minor changes have been made to text that is output to the console.
* The following changes have been made to the silhouettes of weapons in the alternate widescreen HUD:
  * Minor changes have been made to some of them.
  * They will now be displayed again when playing [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) or [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960).
* The map title in the external automap will now be positioned correctly when the `am_external` CVAR is `on`.
* If the player picks up a rocket launcher or BFG-9000 for the first time, they will now automatically equip them as intended.
* The following changes have been made when freeze mode is enabled:
  * Liquid sectors will no longer animate when the `r_liquid_swirl` CVAR is `on`.
  * Friction is no longer applied to the player’s movement when they are on a *BOOM*-compatible icy or muddy sector.
* Minor improvements have been made to the spread and color of blood splats.
* There is now smoother movement in the automap.
* *DOOM Retro* now includes partial support for `UMAPINFO` lumps.
* The following changes have been made to the support of `DEHACKED` lumps:
  * Altering the names of SFX and music lumps will no longer affect the `play` CCMD.
  * `Melee threshold`, `Max target range` and `Min missile chance` values have been added to `Thing` blocks.
  * `Dropped item` values in `Thing` blocks are now 1-based rather than 0-based.
* The following changes have been made to the `playerstats` CCMD:
  * A `Shots successful/fired` stat is now displayed for every weapon.
  * The `Weapon accuracy` stat has been removed, and is instead displayed for every weapon.
  * There is a new `Favorite weapon` stat.
* The precision of the angle the player is facing when using the `IDMYPOS` cheat has been improved.
* Certain cheats are now still active when the player respawns after death.
* The effect applied when the player is attacked and the `r_shake_damage` CVAR is greater than `0%` is now still applied when god mode is enabled.
* Fixes to maps that involve changing a sector’s tag will now work as intended when the `r_fixmaperrors` CVAR is `on`.
* A bug has been fixed whereby certain translucent things wouldn’t be translucent while the player had an invulnerability power-up.
* The music will no longer be reset when using the `map` CCMD to restart the current map.
* The player’s face will no longer be displayed in the widescreen HUD while paused.
* The sound that moving platforms make will no longer be occasionally silenced when they change direction.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, June 7, 2020

### DOOM Retro v3.5.10

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The `condump` CCMD now works again.
* The colors of blood splats now vary slightly.
* More blood splats are now spawned under corpses.
* To avoid accidentally firing them, the player will no longer automatically switch to the rocket launcher or BFG-9000 if they run out of all other ammo.
* The player’s face will now appear correctly in both the status bar and widescreen HUD when god mode is enabled and the `STFGOD0` lump has been replaced in a PWAD.
* If any weapon pickup sprites have been changed in a PWAD, their silhouettes will no longer appear in the alternate widescreen HUD.
* The minimum value of the `vid_capfps` CVAR has been changed from `1` to `10`.
* A bug has been fixed whereby the music volume wouldn’t be restored when closing the console in some instances.
* The randomization of certain features when a thing is spawned at the start of a map (such as whether a corpse is mirrored or not, or the placement of blood splats around a corpse) is now consistent if the map is restarted.
* The `mapstats` CCMD now displays alternate titles for those few maps that have one.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, May 23, 2020

### DOOM Retro v3.5.9

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Fixing a bug present in *Vanilla DOOM*, scrolling wall textures (such as the `SP_FACE1` textures in MAP05 of `plutonia.wad`) will now always scroll at the correct speed.
* A bug has been fixed whereby certain power-ups wouldn’t bob when the `r_floatbob` CVAR was `on`.
* The player’s ability to [straferun](https://doomwiki.org/wiki/Straferunning) has now been restored.
* The `A_RandomJump` codepointer now works again if specified in a `DEHACKED` lump.
* Files created by the `condump` CCMD when no parameter is specified will now be saved in a new `console` folder.
* The mouse or a gamepad can now be used to open the menu from the title screen again.
* Linedefs that have the *BOOM*-compatible line special of 190 (“SR Change Texture And Effect”) now work.
* The interpolation of floors and ceilings that move instantly has now been fixed when the `vid_capfps` CVAR is not `35`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, May 16, 2020

### DOOM Retro v3.5.8

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* When both the automap and menu are open, the grid will now still rotate in the background if the `am_rotatemode` CVAR is `off`.
* The menu background will no longer be affected if the `r_detail` CVAR is `low` and the `r_lowpixelsize` CVAR is not `2×2`.
* The player’s path in the automap is no longer reset when vanilla mode is enabled and the `am_path` CVAR is `on`.
* The timer shown when the `timer` CCMD is used is no longer displayed while vanilla mode is enabled.
* Only one sound is now heard when confirming the selection of the *Nightmare!* skill level.
* A bug has been fixed whereby the “entering” intermission screen would be displayed when exiting MAP30 in some instances.
* When to update the savegame description while saving a game has now been improved.
* Improvements have been made to the left and right edges of the console when it’s open over the automap.
* Minor improvements have been made to the widescreen HUD when god mode is enabled.
* When the player tries to open a *BOOM*-compatible door that requires more than one (or any) keycard or skull key that they don’t have, all of those keys will now flash in the widescreen HUD.
* The gamepad can now be used again to change monsters during *DOOM II’s* cast sequence.
* The `2`, `3` and `4` digits used for marks in the automap are now consistent with those displayed in the console.
* All function keys may now be used while the console is open.
* A bug has been fixed whereby the widescreen HUD would momentarily disappear when pressing the <kbd><b>F8</b></kbd> key to toggle player messages.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, May 8, 2020

### DOOM Retro v3.5.7

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Things spawned using the `spawn` CCMD are now given an ID that is displayed by the `thinglist` CCMD.
* The mouse and gamepad can now be used to open the menu from the title screen again.
* A bug has been fixed whereby an incomplete `MAPINFO` lump in a PWAD could cause a crash.
* The names of the monsters displayed during *DOOM II’s* cast sequence can now be specified in `DEHACKED` lumps.
* The player’s weapon can no longer be changed while freeze mode is on.
* The horizontal offset of the player’s weapon sprite will no longer be fixed when the `r_fixspriteoffsets` CVAR is `off`.
* The effects of both the `r_graduallighting` and `weaponbounce` CVARs are now better remembered in savegames.
* Screenshots may now be taken again while playing a game and the `+screenshot` action has been bound to something other than the <kbd><b>PRINTSCREEN</b></kbd> key.
* When a PWAD is loaded with [*Freedoom: Phase 1*](https://freedoom.github.io/) or [*Freedoom: Phase 2*](https://freedoom.github.io/), if a `STBAR` lump is present in that PWAD, it will now be used.
* Fixing a bug present in *Vanilla DOOM*, monsters will now always be alerted if attacked during the second frame of their idle animation.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, May 2, 2020

### DOOM Retro v3.5.6

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The name of the PWAD is no longer displayed in the automap if the current map has no title.
* Several improvements have been made to the support of [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest), [*Freedoom: Phase 1*](https://freedoom.github.io/) and [*Freedoom: Phase 2*](https://freedoom.github.io/).
* A bug has been fixed whereby the wrong music could be played in some rare instances.
* The randomness of the music chosen when the `s_randommusic` CVAR is `on` has been improved.
* A new `license` CCMD has been implemented that displays the [*GNU General Public License*](https://github.com/bradharding/doomretro/wiki/License) in the default browser.
* A message is now displayed in the console whenever an item or monster respawns.
* A new `r_supersampling` CVAR has been implemented that applies supersampling to the player’s view when the `r_detail` CVAR is `low` and the `r_lowpixelsize` CVAR is `2×2`. This CVAR is `on` by default and `off` when vanilla mode is on.
* Corpses can now trigger teleporter line specials.
* When using the `kill` CCMD, the items dropped by monsters will no longer trigger teleporter line specials.
* A bug has been fixed whereby some hanging decorations would be positioned incorrectly in some instances.
* Those monsters that don’t have red blood will no longer leave gibs if crushed.
* Blood will no longer be spawned while a monster is being crushed and the `r_blood` CVAR is `none`.
* A bug has been fixed whereby certain power-ups wouldn’t stop moving once off the edge of a *BOOM*-compatible scrolling sector and the `r_floatbob` CVAR was `on`.
* Improvements have been made to how gradual lighting is applied to doors and crushing ceilings when the `r_graduallighting` CVAR is `on`.
* The `STEP2` texture now appears correctly in *MAP01: Entryway* again.
* Minor improvements have been made to the support of `DEHACKED` lumps.
* The sky will now be rendered correctly when the `r_screensize` CVAR is less than `7`.
* Screenshots may now only be taken when not playing a game if the `+screenshot` action is still bound to the <kbd><b>PRINTSCREEN</b></kbd> key.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, April 18, 2020

### DOOM Retro v3.5.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The music volume is now lower while either the menu or console is open.
* The following changes have been made to the menu’s background:
  * The random noise is now slightly slower.
  * Liquid sectors now still animate in the background when the menu is open.
* The following changes have been made to the widescreen HUD:
  * Health, armor and ammo now flash gold rather than white when they change.
  * Keycards and skull keys that the player picks up are now slightly higher.
* Brightmaps are no longer applied to the `SLADRIP1`, `SLADRIP2` and `SLADRIP3` textures when the `r_brightmaps` CVAR is `on`.
* The timer displayed by the `timer` CCMD will now be positioned lower if the `vid_showfps` CVAR is `on`.
* More fixes have been applied to certain maps in the official *DOOM* and *DOOM II* WADs when the `r_fixmaperrors` CVAR is `on`.
* The `mapstats` CCMD now displays the release date for maps in the official *DOOM* and *DOOM II* WADs.
* The translucency of item and teleport fogs is no longer additive when the `r_translucency` CVAR is `on`.
* Due to a change in the latest patch of the [*Bethesda.net Launcher*](https://bethesda.net/en/game/bethesda-launcher) version of [*DOOM II*](https://bethesda.net/en/store/product/DO2GNGPCBG01), MAP04 to MAP08 in *No Rest For The Living* now use the `SKY3` texture as their skies.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, April 3, 2020

### DOOM Retro v3.5.4

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Improvements have been made to the support of [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest).
* A bug has been fixed whereby the wrong map name would be displayed on the intermission screen in some instances.
* A brightmap is now applied to the `SLADRIP2` texture when the `r_brightmaps` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, March 23, 2020

### DOOM Retro v3.5.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* A bug has been fixed whereby screenshots couldn’t be taken while the console was open.
* The following changes have been made to timestamps in the console:
  * The `con_timestamps` CVAR has been deprecated. Timestamps will now always appear in the console next to player messages.
  * Timestamps in the console are now a translucent yellow.
* The following changes have been made to obituaries in the console:
  * Obituaries involving an exploding barrel now indicate who caused the explosion.
  * Names set using the `name` CCMD will now be used when an arch-vile resurrects a monster.
* The number of times the player commits suicide is now displayed by the `playerstats` CCMD.
* All monsters spawned using the `spawn` CCMD can now cross monster-blocking lines.
* More blood splats now appear around decorative corpses.
* A crash will no longer occur when trying to spawn a berserk power-up using the `spawn` CCMD in *DOOM (Shareware)*.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, March 13, 2020

### DOOM Retro v3.5.2

* *DOOM Retro* now uses [*SDL v2.0.12*](https://www.libsdl.org).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro’s* keyboard and mouse controls are now more responsive.
* Further improvements have been made to the console’s autocomplete feature.
* Minor changes have been made to text that is output to the console.
* Item fogs spawned while using the `spawn` or `respawnitems` CCMDs are now always positioned correctly.
* The following changes have been made to the `r_blood` CVAR:
  * The CVAR can now be set to `nofuzz`, causing all blood spilled by spectres (as well as the player while they have a partial invisibility power-up) to be red instead of fuzzy.
  * Puffs are now spawned rather than nothing when the CVAR is `none`.
  * All blood spilled will now be red when the CVAR is `red`, and green when the CVAR is `green`.
* Flying monsters spawned using the `spawn` CCMD now spawn higher off the ground.
* The player’s path around the current map will now always be recorded even while the `am_path` CVAR is `off`.
* The following changes have been made to the `mapstats` CCMD:
  * The total number of things in the current map is now correct.
  * How much the current map is inside/outside is now displayed.
* The number of times the player has died in the current map is now correct in the `playerstats` CCMD.
* The following changes have been made to the numbers displayed in the widescreen HUD:
  * Health, armor and ammo now flash slightly brighter when they change, and do so even when the `r_hud_translucency` CVAR is `off`.
  * Ammo now flashes when the player switches to a weapon with different ammo.
  * Health and ammo now flash on and off when low and the `r_hud_translucency` CVAR is `off`.
* 3D bridges that use the `STEP2` wall texture are now rendered correctly.
* The automap will now rotate correctly in the background if the menu is open and the `am_rotatemode` CVAR is `off`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, February 21, 2020

### DOOM Retro v3.5.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to the character set used in the console.
* Minor changes have been made to text that is output to the console.
* Monsters can now be spawned using the `spawn` CCMD when the `nomonsters` CCMD has been entered in the console, or the `-nomonsters` parameter has been specified on the command-line.
* The following changes have been made to the widescreen HUD:
  * The HUD has been brought in slightly from the edges of the screen.
  * The player’s armor is now on the left side of the screen next to their health, and the ammo for their currently equipped weapon on the right.
* Item and teleport fogs are now spawned when using the `spawn` CCMD.
* Thing triangles in the automap representing *MBF*-compatible helper dogs are now the correct size again when using the `IDDT` cheat.
* Minor improvements have been made to the menu’s background.
* The player’s view is now reset again when exiting the menu.
* A bug has been fixed whereby the use of `LIQUID` or `NOLIQUID` in a `MAPINFO` lump would have no effect.
* The scrollbar in the console has been widened and now includes a grip.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, February 8, 2020

### DOOM Retro v3.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Improvements have been made to the support of IWADs that contain censored Wolfenstein SS sprites.
* IWADs specified using the `-file` command-line parameter will now be treated as PWADs.
* Minor changes have been made to the character set used in the console.
* Minor changes have been made to text that is output to the console.
* The keys on the numeric keypad now work correctly in the console.
* *DOOM Retro* will now play sound effect lumps that are in [WAV format](https://en.wikipedia.org/wiki/WAV).
* The corpses of some of the smaller monsters, as well as other marines, will now be gibbed when close enough to barrel and rocket explosions. This feature can be toggled on or off using the new `r_corpses_gib` CVAR, which is `on` by default and `off` when vanilla mode is on.
* A bug has been fixed whereby certain floor textures would glitch in some rare instances if the `r_liquid_current` CVAR was `on`.
* The following changes have been made to *DOOM Retro’s* *MBF*-compatible helper dogs:
  * The `DOGSA1` to `DOGSN0` lumps have now been replaced with [sprites by Nash Muhandes](https://forum.zdoom.org/viewtopic.php?f=59&t=58035), released under the [*Creative Commons (BY 3.0)*](https://creativecommons.org/licenses/by/3.0/) license.
  * The `DSDGACT` and `DSDGATK` lumps have now been replaced with edited versions of [“Mr Dog_01.wav” by apolloaiello](https://freesound.org/people/apolloaiello/sounds/276267/), released under the [*Creative Commons (CC0 1.0 Universal)*](https://creativecommons.org/publicdomain/zero/1.0/) license.
  * The `DSDGDTH`, `DSDGPAIN` and `DSDGSIT` lumps have now been replaced with edited versions of [“DogYelp.wav” by TobiasKosmos](http://freesound.org/people/TobiasKosmos/sounds/163280/), released under the [*Creative Commons (BY 3.0)*](https://creativecommons.org/licenses/by/3.0/) license.
* The `map` CCMD now also accepts a map’s title as its parameter. For example, entering `map nuclearplant` in the console will warp the player to *E1M2: Nuclear Plant*.
* The effects of changing the `r_blood` CVAR in the console will now always be immediate.
* The `teleport` CCMD now accepts an optional third parameter, specifying the height the player will be from the floor once they have teleported.
* The color of the console’s bottom edge may now be changed using the new `con_edgecolor` CVAR. It is `180` by default.
* The `s_musicvolume` and `s_sfxvolume` CVARs are now both set to the same lowest value when vanilla mode is on.
* A crash will no longer occur when the player uses the secret exit in *SIGIL’s E5M6: Unspeakable Persecution*.
* Further improvements have been made to the support of `MAPINFO` lumps.
* A sound is now made when toggling between fullscreen and a window.
* Lines with special 46 (“GR Door Open Stay”) now work correctly.
* Monsters can no longer be spawned using the `spawn` CCMD when the `nomonsters` CCMD is in effect.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, January 4, 2020

### DOOM Retro v3.4

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* will now only attempt to use [*Adaptive VSync*](https://www.geforce.com/hardware/technology/adaptive-vsync) if the `vid_vsync` CVAR is set to `adaptive`.
* The following changes have been made to the `mapstats` CCMD:
  * The episode or expansion, as well as the map number, of the current map will now be displayed.
  * Whether or not the current map is secret will now be displayed.
  * Whether or not the current IWAD is the *BFG Edition* will now be displayed.
  * The par time of the current map will now be displayed.
  * Improvements have been made to how *BOOM* and *MBF*-compatible maps are detected.
  * Whether or not a music lump has been modified in a PWAD will now be displayed.
* Minor changes have been made to the character set used in the console.
* Minor changes have been made to text that is output to the console.
* A bug has been fixed whereby widescreen mode wouldn’t be displayed correctly after pressing <kbd><b>ALT</b></kbd> + <kbd><b>ENTER</b></kbd> to toggle between fullscreen and a window, and if the `vid_borderlesswindow` CVAR was `off`.
* The default of the `vid_borderlesswindow` CVAR is now `on`.
* The default of the `vid_scalefilter` CVAR has been changed from `“nearest”` to `“nearest_linear”`.
* A new `r_graduallighting` CVAR has been implemented that toggles the gradual lighting under doors and crushing sectors. It is `on` by default and `off` when vanilla mode is on.
* The player’s view will no longer shift slightly when exiting a map by using a switch.
* Par times are no longer displayed on the intermission screen for maps that don’t have them.
* A crash will no longer occur when using the `kill` or `resurrect` CCMDs with a monster’s name previously set by the `name` CCMD as the parameter.
* `SIGIL_SHREDS.wad` will no longer be automatically loaded if music has been disabled by specifying `-nomusic` or `-nosound` on the command-line.
* The background will now always be displayed correctly when confirming use of the `resetall` CCMD.
* The background will now continue to rotate when confirming if the player wants to end a game or quit.
* The title of MAP05 in *Final DOOM: TNT - Evilution* has been corrected.
* The amount of friction applied to the player’s corpse has been increased, matching the corpses of monsters, when in a liquid sector.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, December 21, 2019

### DOOM Retro v3.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* If a mistyped filename is entered in the WAD launcher (for example, `doot.wad` instead of `doom.wad`), *DOOM Retro* will now try to find the closest match.
* If hardware acceleration is unavailable, *DOOM Retro* will now change the `vid_scaleapi` CVAR to `software` and scale each frame accordingly, rather than simply crashing during startup.
* *DOOM Retro* is now fully [*MBF*](https://doomwiki.org/wiki/MBF)-compatible, as support has now been added to `BOUNCES` and `TOUCHY` flags in *DeHackEd* lumps.
* Further improvements have been made to the console’s autocomplete feature.
* Minor changes have been made to the character set used in the console.
* Minor changes have been made to text that is output to the console.
* A level 2 warning will now be displayed in the console if there’s a locked door with no keycard or skull key provided in the map to open it.
* The visual glitch displayed when input is selected in the console while it is opening or closing is now fixed.
* Once the `condump` CCMD is used to dump the current contents of the console to a file, every line of text that is output to the console thereafter will continue to be dumped to that same file automatically.
* The `r_blood` CVAR can now be `green`, causing all blood spilled to be green.
* The `episode` CVAR will now be updated if the player finishes episode 4 and [*SIGIL*](https://www.romerogames.ie/si6il) is loaded.
* The `thinglist` CCMD now indicates if a thing has been dropped by a monster.
* A crash will no longer occur when a monster tries to make a noise and the `-nosfx` or `-nosound` parameters are specified on the command-line.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, November 27, 2019

### DOOM Retro v3.2.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* will now attempt to use [*Adaptive VSync*](https://www.geforce.com/hardware/technology/adaptive-vsync) if the `vid_vsync` CVAR is `on` and the `vid_scaleapi` CVAR is `“opengl”`.
* A bug has been fixed whereby some monsters wouldn’t attempt to fire at the player as often as they should if another monster was in the way.
* Further improvements have been made to the console’s autocomplete feature.
* Minor changes have been made to text that is output to the console.
* Minor improvements have been made to the menu’s background.
* A bug has been fixed whereby blood splats around crushed corpses could appear black in some rare instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, November 25, 2019

### DOOM Retro v3.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to the character set used in the console.
* Minor changes have been made to text that is output to the console.
* Long warnings in the console now wrap over two lines.
* A new `warninglevel` CVAR has been implemented to control the type of warnings displayed in the console. It can be `0` (minimal warnings), `1` (no warnings about the current map) or `2` (all warnings), and is `1` by default.
* The following changes have been made to warnings in the console when the `warninglevel` CVAR is `2`:
  * Warnings will now be displayed for every linedef in the current map that has either an unknown tag, a tag but no special, or a special but no tag.
  * Warnings describing any fixes made to the current map are now displayed when the `r_fixmaperrors` CVAR is `on`.
* Further improvements have been made to the console’s autocomplete feature.
* The `resurrect` CCMD has been enhanced to allow not only the resurrection of the player, but also all monsters or a type of monster.
* To accommodate for when the player is in liquid and needs to shoot a switch, now only their view will be lowered, and not their gunshot, if the `r_liquid_lowerview` CVAR is `on` and `mouselook` CVAR is `off`.
* The `-nodeh` command-line parameter now works as intended.
* Minor improvements have been made to the menu’s background.
* A crash will no longer occur when trying to autoload a savegame that was previously deleted using the <kbd><b>DEL</b></kbd> key in the save or load game menu.
* The vertical direction the player is looking is now centered when they teleport with the `mouselook` CVAR `on`.
* Further improvements have been made to the support for Noiser’s [*DOOM 4 VANILLA*](https://www.doomworld.com/forum/topic/108725).
* Timestamps between `12:00:00` and `12:59:59` in the console are now displayed correctly.
* The text on the help screen displayed using the <kbd><b>F1</b></kbd> key now has drop shadows.
* A new `vid_borderlesswindow` CVAR has been implemented that toggles the use of a borderless window rather than true fullscreen when the `vid_fullscreen` CVAR is `on`. It is `off` by default.
* A bug has been fixed whereby the player wouldn’t be able to telefrag a monster in some instances.
* A countdown will now be displayed in the top right of the screen if a timer is set using the `timer` CCMD.
* The number of maps started now appears alongside the number of maps completed in the `playerstats` CCMD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, November 2, 2019

### DOOM Retro v3.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM II: Hell On Earth’s* IWAD no longer needs to be the *BFG Edition* for [`nerve.wad`](https://doomwiki.org/wiki/No_Rest_for_the_Living) to be automatically loaded if found.
* Minor changes have been made to the character set used in the console.
* Minor changes have been made to text that is output to the console.
* The effects of the `autotilt` CVAR are now disabled while the freeze or no clipping modes are on.
* The default value of `am_gridcolor` has been changed from `6` to `111`, making the automap’s grid a slightly lighter gray.
* If a berserk power-up was previously taken away from the player using the `take` CCMD, they may no longer equip their fists if they have a chainsaw.
* The pistol may now be taken away from and given back to the player using the `take` and `give` CCMDs.
* The thing triangles in the automap representing dogs will now be the correct size when using the `IDDT` cheat.
* The player will no longer reload their super shotgun after firing their last two shells.
* A bug has been fixed whereby not all monsters near the player would infight once the player died and the `infighting` CVAR was `on`.
* Further improvements have been made to the support for Noiser’s [*DOOM 4 VANILLA*](https://www.doomworld.com/forum/topic/108725).
* The z-coordinate displayed by the `IDMYPOS` cheat now accommodates for when the player is in liquid and the `r_liquid_lowerview` CVAR is `on`.
* A bug has been fixed whereby a flying monster could fall out of the air if over a moving, liquid sector in some instances.
* The horizontal and vertical sensitivity of a gamepad’s thumbsticks can now both be adjusted individually. The `gp_sensitivity` CVAR has been replaced by the new `gp_sensitivity_horizontal` and `gp_sensitivity_vertical` CVARs. Each CVAR is a value between `0` and `128`, and `64` by default.
* If a friendly dog is returning to the player, they will now drop from any height regardless of how far they are away.
* Monsters and items can no longer be spawned outside of the map if the player is too close to a wall when using the `spawn` CCMD.
* A bug has been fixed whereby bobbing power-ups wouldn’t move on *BOOM*-compatible scrolling sectors in some instances if the `r_floatbob` CVAR was `on`.
* The correct sound is now played when a *BOOM*-compatible generalized door opens or closes at normal speed.
* Linedefs with specials but no sectors tagged are now handled better.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, October 11, 2019

### DOOM Retro v3.0.5

* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to the character set used in the console.
* Minor changes have been made to text that is output to the console.
* Picking up more than one power-up of the same type now works correctly.
* The `-file` parameter may now be used without an `-iwad` parameter on the command-line.
* A bug has been fixed whereby using either the `nomonsters` CCMD in the console, or `-nomonsters` on the command-line, would not stop monsters from being spawned in maps.
* The speed the player turns is now affected by the `turbo` CVAR and `-turbo` command-line parameter.
* The correct sky textures will now be displayed when playing [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) or [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960) and the `mouselook` CVAR is `off`.
* The `friendly` parameter will now work correctly with the `name` CCMD.
* A bug has been fixed whereby using the `monster` or `monsters` parameters with the `kill` CCMD would cause a crash.
* Minor improvements have been made to the support of `DEHACKED` lumps.
* The bullet puffs when shooting lost souls are now vertically centered.
* Obituaries that involve a friendly monster will now indicate if there’s only one of them in the map.
* Further improvements have been made to the support for Noiser’s [*DOOM 4 VANILLA*](https://www.doomworld.com/forum/topic/108725).

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, October 1, 2019

### DOOM Retro v3.0.4

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to the character set used in the console.
* Minor changes have been made to text that is output to the console.
* *DOOM Retro* will now automatically load `SIGIL_v1_21.wad` (or `SIGIL_v1_2.wad`) in preference to `SIGIL.wad`.
* Timestamps in the console are now displayed in 12 rather than 24-hour format, and without a leading zero.
* If the player saves a game, now that game will from then on be automatically saved at the start of each map. This feature can be disabled by the new `autosave` CVAR, which is `on` by default and `off` when vanilla mode is on.
* The following changes have been made to the `name` CCMD:
  * Spaces are now allowed in the name, whether surrounded by double quotes or not.
  * Whether the monster being named already has been named will now be indicated.
  * The player can now also be named (which is effectively the same as changing the `playername` CVAR).
* A bug has been fixed whereby enabling the `am_path` CVAR would cause *DOOM Retro* to crash.
* The path of the player that appears in the automap when the `am_path` CVAR is `on` is now thicker.
* Minor improvements have been made to the support of `DEHACKED` lumps.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 21, 2019

### DOOM Retro v3.0.3

* The behavior of the `-nodeh` command-line parameter has changed. It will still disable the automatic loading of `.deh` files, `.bex` files and `DEHACKED` lumps, but will now load `.deh` and `.bex` files if selected in the WAD launcher or specified using the `-deh` command-line parameter.
* A bug has been fixed whereby dogs spawned in front of the player using the `spawn` CCMD in the console would always be friendly, even when the `friendly` parameter wasn’t used. This bug also affected PWADs that would replace the dog using a `DEHACKED` patch. All monsters of that type spawned at the start of a map would be friendly and therefore attack and be attacked by other monsters. (An example of this are the nightmare demons in [*Eviternity*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/eviternity).)
* The following compatibility fixes have been implemented for Noiser’s [*DOOM 4 VANILLA*](https://www.doomworld.com/forum/topic/108725):
  * `D4V.WAD` will now always be the last PWAD to be loaded if selected together with another PWAD in the WAD launcher.
  * All monsters are now correctly named when using the `spawn` CCMD and in obituaries in the console.
  * The summoner’s projectiles no longer have smoke trails.
  * The super chainsaw and mega doll are now rendered correctly.
* Minor changes have been made to text that is output to the console.
* The word `monster` may now be used instead of a specific monster type to name the nearest monster to the player when using the `name` CCMD.
* Only monsters who are alive can be named using the `name` CCMD.
* Jumping will now always work as intended.
* A bug has been fixed whereby looking up and down with a gamepad when the `gp_invertyaxis` CVAR was `off` would still be inverted, and when `on` would not be and also cause the player’s view to jitter.
* The double shotgun sound can now be heard again in *DOOM II’s* cast sequence.
* The crosshair displayed using the `crosshair` CVAR no longer needs the `mouselook` CVAR to be `on`.
* The correct music will now be played on the intermission screen of episodes 1 to 4 when John Romero’s megawad [*SIGIL*](https://www.romerogames.ie/si6il) has been autoloaded.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 14, 2019

### DOOM Retro v3.0.2

* Changes have been made to improve the overall stability of *DOOM Retro*.
* A bug has been fixed whereby nearby monsters wouldn’t hear when the player fired the BFG-9000.
* Infighting among monsters will now work correctly.
* Shots fired by the player’s hitscan weapons will no longer be lowered if the player is standing in liquid and the `r_liquid_lowerview` CVAR is `off`. Similarly, shots fired by the hitscan weapons of some monsters will no longer be lowered if the monster is standing in liquid and the `r_liquid_clipsprites` CVAR is `off`.
* The following compatibility fixes have been implemented for Noiser’s [*DOOM 4 VANILLA*](https://www.doomworld.com/forum/topic/108725):
  * Cacodemons and hell knights will now always bleed red blood, regardless of the value of the `r_blood` CVAR.
  * Gore nests are no longer translucent or randomly mirrored when destroyed, regardless of the value of the `r_corpses_mirrored` CVAR.
  * A gore nest may now be spawned in front of the player by entering `spawn gore nest` in the console.
  * Rocket trails are disabled, regardless of the value of the `r_rockettrails` CVAR.
* Minor changes have been made to text that is output to the console.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, September 9, 2019

### DOOM Retro v3.0.1

* Changes have been made to improve the overall stability of *DOOM Retro*.
* A bug has been fixed whereby the BFG-9000 wouldn’t always target monsters correctly when fired, and in some instances would kill the player themselves.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 7, 2019

### DOOM Retro v3.0

* *DOOM Retro* now uses [*SDL v2.0.10*](https://www.libsdl.org) and [*SDL_image v2.0.5*](https://www.libsdl.org/SDL_image).
* When *DOOM Retro* is opened for the first time and the WAD launcher automatically navigates to a *DOOM* or *DOOM II* installation it has found, the corresponding IWAD will now also be selected.
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The format of savegames has needed to be changed, breaking compatibility with previous versions of *DOOM Retro*. (Provisions have also been put in place to avoid breaking compatibility when adding new features in the future.)
* If the 32-bit binary of *DOOM Retro* is used on a 64-bit version of *Windows*, a warning is now displayed in the console at startup recommending the 64-bit binary instead.
* The logo on the splash screen is now animated.
* The branding in the console has been redesigned.
* The following changes have been made to the support for John Romero’s megawad [*SIGIL*](https://www.romerogames.ie/si6il):
  * `SIGIL.wad` will now only be automatically loaded alongside *The Ultimate DOOM* and not the registered version of *DOOM*.
  * If [*Buckethead’s*](https://en.wikipedia.org/wiki/Buckethead) `SIGIL_SHREDS.wad` is found, it will now also be automatically loaded.
  * Savegame descriptions will now be updated with the current map’s name when saving over an existing savegame.
  * The `IDMUS` cheat can now be used to play *SIGIL’s* music.
  * The `episode` CVAR will now be updated if the `IDCLEV` cheat is used to warp to a map in *SIGIL*.
  * Par times are now displayed on the intermission screen for each map.
  * The `next`, `last` and `random` parameters of the `map` CCMD now accommodate for *SIGIL’s* maps.
* Minor changes have been made to text that is output to the console.
* Minor improvements have been made to how blood hits the floor.
* Fading to black upon quitting is now smoother, and will now also occur when a PWAD has its own `COLORMAP` lump.
* *DOOM Retro’s* support for [*MBF*](https://doomwiki.org/wiki/MBF)-compatible maps has been improved by supporting monsters with the `MF_FRIEND` flag. This also allows the following:
  * The `spawn` CCMD may now be used to spawn a friendly monster in front of the player. For example, to spawn a friendly imp, enter `spawn friendly imp` in the console. These monsters will follow the player around and occasionally attack other monsters.
  * A [*helper dog*](https://doomwiki.org/wiki/Helper_dog) may now be spawned in the current map by entering `spawn friendly dog` in the console (whereas just entering `spawn dog` will spawn a dog that will attack the player).
  * All friendly monsters in the current map may be killed by entering `kill friends` in the console.
  * Obituaries, as well as the `thinglist` CCMD, now indicate if a monster is friendly.
* The player can now give any monster a unique name using the new `name` CCMD. For example, entering `name cacodemon Hissy` will give the name Hissy to the nearest cacodemon that the player can see. The name given will then be used in any obituary that involves that monster. This also allows the following:
  * The `thinglist` CCMD will now show a monster’s name.
  * The `kill` CCMD may now be used to kill a monster by specifying its name.
* Further improvements have been made to the console’s autocomplete feature.
* A bug has been fixed whereby music often wouldn’t play at all in episode 4 of *The Ultimate DOOM* if the `s_randommusic` CVAR was `on`.
* The following changes have been made to the `mapstats` CCMD:
  * The PWAD of the current map will now always be correct.
  * Improvements have been made to how *BOOM* compatibility is detected.
  * Whether the current map is *MBF*-compatible or not is now displayed.
  * The author of some of the maps in *DOOM II: No Rest For The Living* has been corrected.
* The `playerstats` CCMD now shows how many times the player has saved the game in the current map.
* The `+screenshot` action can now be bound to a mouse button.
* A camera’s shutter sound is now played when taking a screenshot using the <kbd><b>PRINTSCREEN</b></kbd> key.
* A bug has been fixed whereby starting a new game after playing either *E1M4B: Phobos Mission Control* or *E1M8B: Tech Gone Bad* would cause the wrong map to be loaded.
* Partially restoring behavior present in *Vanilla DOOM*, the sound effects of the bosses (that is, the barons of hell in E1M8, the cyberdemon in E2M8 and the spider mastermind in E3M8) are no longer clipped by distance from the player.
* Improvements have been made in determining if animated flats depict liquid.
* The edges of liquid sectors are now rendered better in some instances.
* The status bar will no longer be partially displayed in the background when ending a game from the options menu.
* The friction applied to corpses in liquid is now greater rather than less than the friction applied when not in liquid.
* A bug has been fixed whereby the colors used in the automap would be wrong in some rare instances.
* The external automap displayed when the `am_external` CVAR is `on` will now:
  * Also fade to black when quitting *DOOM Retro*.
  * No longer be negatively affected by the `vid_vsync` CVAR.
  * Be displayed correctly when the help screen is opened on the main screen by pressing the <kbd><b>F1</b></kbd> key.
* Entering the `IDBEHOLD` cheat without any parameter will now:
  * Timeout after 2 seconds like all the other cheats.
  * Show underscores under the message if entered when the alternate widescreen HUD is displayed.
* A bug has been fixed whereby if the player fires their weapon the moment they are exiting a map, a crash could occur when they then try to spawn in the next one.
* The player will no longer move forward slightly once spawning in a new map in some rare instances.
* The lighting of sectors that have sector special 17 (“Light Flickers (Randomly)”) will now always flicker as intended.
* Gradual lighting is now applied to doors and crushing ceilings over damaging sectors.
* A compatibility fix has been implemented for [*Eviternity*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/eviternity) that changes the blood of nightmare demons to green.
* If the `+use` action is used against a wall that has a line special requiring it to be shot at instead, or has a scrolling or translucent texture, the player will now make an “oof” sound.
* The effects of using the `fastmonsters` CCMD are now immediate.
* The effects of changing the `con_backcolor` CVAR are now immediate.
* The direction the menu’s background spins is now the same as the direction the player last turned.
* The `freeze`, `notarget`, `pistolstart`, `regenhealth` and `respawnitems` CCMDs will now all be turned off when enabling vanilla mode.
* If the filename of a PWAD is used in the caption of *DOOM Retro’s* window, the correct case will be used.
* The crosshair displayed when the `crosshair` CVAR is `cross` or `dot` will now always be displayed correctly.
* The use of the color blue in the alternate widescreen HUD has been improved.
* The designs of the 4 and 5 digits used in the alternate widescreen HUD have been tweaked slightly.
* Recoiling of the player’s weapon when the `weaponrecoil` CVAR is `on` no longer requires the `mouselook` CVAR to be `on`, or a control to be bound to the `+mouselook` action.
* A bug has been fixed whereby a gamepad could randomly start vibrating during a game even though the player was using the mouse and/or keyboard instead.
* A bug has also been fixed whereby a gamepad could continue to vibrate once it should have stopped doing so in some rare instances.
* If an error is found in a `MAPINFO` lump, a warning will now be displayed in the console rather than *DOOM Retro* exiting with an error.
* The `IDFA` and `IDKFA` cheats will no longer work if the player already has all the items those cheats provide.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, June 1, 2019

### DOOM Retro v2.9.3

* The following changes have been made to the support for John Romero’s megawad [*SIGIL*](https://www.romerogames.ie/si6il):
  * The `SKY5_ZD` lump is now used instead of the `SKY5` lump to render the sky.
  * The `+jump` action is no longer disabled if `SIGIL.wad` is loaded automatically.
  * A bug has been fixed whereby the `SIGILINT` lump was being displayed in the intermission’s background for episodes 1 to 4.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, May 31, 2019

### DOOM Retro v2.9.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The following changes have been made to the support for John Romero’s megawad [*SIGIL*](https://www.romerogames.ie/si6il):
  * If `SIGIL.wad` is loaded automatically, it’s `TITLEPIC`, `CREDIT` and `HELP1` lumps won’t be displayed, and it’s `D_INTRO` music lump won’t be played.
  * The `mapstats` CCMD will now correctly identify the title and composer of the music of the current map (either [James Paddock](https://doomwiki.org/wiki/James_Paddock_(Jimmy)), or [Buckethead](https://en.wikipedia.org/wiki/Buckethead) if `SIGIL_SHREDS.wad` is loaded).
  * *E5M8: Halls of Perdition* will no longer end abruptly when killing a certain number of monsters.
  * The player will now be warped to the correct map once finishing *E5M9: Realm of Iblis*.
* Minor changes have been made to text that is output to the console.
* The player will no longer be injured when standing close to but above a damaging sector in some instances.
* The crosshair displayed when enabling the `crosshair` CVAR is now hidden sooner when the player runs out of ammo.
* A bug has been fixed whereby if the `-warp` command-line parameter was used, the player couldn’t access either the menu or the console.
* The `mapstats` CCMD will now show the lump name of the current map’s music.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, May 26, 2019

### DOOM Retro v2.9.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The support for John Romero’s megawad [*SIGIL*](https://www.romerogames.ie/si6il) has been greatly improved.
* Minor changes have been made to text that is output to the console.
* A crash will no longer occur if the player’s health is less than -999% when they die and the widescreen HUD is displayed.
* A dead player will no longer turn to face an attacker if that attacker actually killed a [voodoo doll](https://doomwiki.org/wiki/Voodoo_doll) rather than the player themselves.
* Things with the `MF2_FLOATBOB` flag set will no longer float and bob if they are also corpses.
* A bug has been fixed whereby the player wouldn’t die as intended when exiting certain maps in [*Eviternity*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/eviternity), causing them to erroneously retain their weapons and ammo when warping to the next map.
* If the WAD selected in the WAD launcher causes *DOOM Retro* to exit with an error, the `wad` CVAR will not be updated.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, May 13, 2019

### DOOM Retro v2.9

* *DOOM Retro* is now compiled using [*Microsoft Visual Studio Community 2019*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Support has been added for John Romero’s highly anticipated megawad [*SIGIL*](https://www.romerogames.ie/si6il). When loading *The Ultimate DOOM’s* IWAD from the WAD launcher, if `sigil.wad` is in the same folder, it will automatically be loaded and a fifth entry, “Sigil of Baphomet”, will then be available in the episode menu.
* The WAD launcher will now only automatically load [`nerve.wad`](https://doomwiki.org/wiki/No_Rest_for_the_Living) when the *BFG Edition* of *DOOM II: Hell On Earth’s* IWAD is selected.
* Further improvements have been made to the console’s autocomplete feature.
* An obituary is no longer displayed in the console if either the source or target is a [voodoo doll](https://doomwiki.org/wiki/Voodoo_doll).
* Thing triangles are now displayed in the automap for voodoo dolls if the `IDDT` cheat is used.
* A bug has been fixed whereby a gamepad’s left thumbstick couldn’t be used to navigate the menu, nor pan around the automap when the `am_followmode` CVAR was `off`. (A gamepad’s right thumbstick can now also be used in these instances.)
* The player’s favorite skill level is now displayed by the `playerstats` CCMD.
* The accuracy of the player’s weapons has been improved when the `mouselook` CVAR is `on` and the `autoaim` CVAR is `off`.
* Exploding barrels will no longer slide due to their own blast damage.
* To improve accuracy, a monster’s actual bounding box is now checked when it is attacked.
* A couple of rendering problems related to the use of line special 242 (“Create Fake Ceiling and Floor”) in *BOOM*-compatible maps have been fixed.
* The scrollbar in the console is now slightly wider.
* Scrolling up and down in the console using the <kbd><b>PGUP</b></kbd> and <kbd><b>PGDN</b></kbd> keys will now become faster the longer they are held down.
* If the player dies in either *E1M4B: Phobos Mission Control* and *E1M8B: Tech Gone Bad* they will now respawn in the correct map.
* Further improvements have been made to the support of `DEHACKED` lumps:
  * An error will no longer be displayed in the console if the `NOTDMATCH` flag is used.
  * The par times for the maps in Episode 4 of *The Ultimate DOOM*, as well as MAP33 in *DOOM II: Hell On Earth (BFG Edition)*, can now be specified in the `[PARS]` section.
  * A bug has been fixed whereby par times specified in the `[PARS]` section would not be read correctly in some rare instances.
  * A warning will now be displayed in the console if a `.deh` or `.bex` file was found but a `DEHACKED` lump was used instead.
* The following changes have been made to the `resetall` CCMD:
  * If a control is bound to the `+mouselook` action, the current map’s sky will now be unstretched immediately.
  * All aliases created using the `alias` CCMD will now be deleted.
* The following changes have been made to the `take` CCMD:
  * A bug has been fixed whereby if the player’s health was less than 100% it would be increased to 100% when using the `all` parameter. Now any health over 100% will be taken from the player.
  * The red palette effect will now be applied to the screen if any health is taken from the player.
  * The gold palette effect will no longer be applied to the screen if any item is taken from the player.
* The following changes have been made to the `crosshair` CVAR:
  * Instead of `on` or `off`, the CVAR can now be `none` (the new default), `cross` (to display an actual cross) or `dot` (to display a dot).
  * The crosshair now becomes brighter when the player fires their weapon.
  * The crosshair is now displayed when the `autoaim` CVAR is `on`.
  * The crosshair will no longer appear when the player has their fists or chainsaw equipped.
  * The color of the crosshair may now be changed using the new `crosshaircolor` CVAR. It is `4` (white) by default.
  * The crosshair is no longer translucent if the `r_hud_translucency` CVAR is `off`.
* *DOOM Retro* will no longer crash at startup when loading [`sunder.wad`](https://www.doomworld.com/forum/topic/46002).
* The current map’s title is now displayed on the correct screen when both the `am_external` and `r_althud` CVARs are `on`.
* The widescreen HUD will no longer briefly appear in the background when toggling messages on or off in the options menu.
* Low graphic detail will no longer be affected by the value of the `r_screensize` CVAR when not playing a game.
* The edges of liquid sectors are now rendered better in some instances.
* The color of player messages in the alternate widescreen HUD is now correct when the `r_textures` CVAR is `off` and a `COLORMAP` lump that replaces colormap 32 is present in the current PWAD.
* A bug has been fixed whereby the menu could appear with a corrupted palette during startup in some rare instances.
* The red palette effect when the player is injured will now fade out quickly when the console is opened.
* The `play` CCMD can now be used to restart music.
* The player’s health, armor and ammo will no longer flash when low in the widescreen HUD if freeze mode is on.
* The `Monsters killed` stats shown by the `playerstats` CCMD now only include kills actually made by the player, and not those due to infighting among monsters.
* A bug has been fixed whereby pressing the <kbd><b>F11</b></kbd> key on the intermission screen could affect the screen’s palette in some instances.
* The crosshair in the automap when follow mode is off is no longer translucent if the `r_hud_translucency` CVAR is also `off`.
* The status bar is now drawn in the background as necessary when a centered message is displayed.
* `midiproc.exe` will no longer remain open when quitting *DOOM Retro* and no MUS or MIDI music lumps have been played.
* Elements in the alternate widescreen HUD that are meant to be blue now appear blue rather than purple when playing [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) or [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960).
* A bug has been fixed whereby the splash damage from rockets would be doubled in some instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, February 17, 2019

### DOOM Retro v2.8.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The current map’s title is now displayed again in the automap when the `vid_widescreen` CVAR is `off`.
* Further improvements have been made to the console’s autocomplete feature.
* A bug has been fixed whereby loading separate `.deh` or `.bex` files would cause *DOOM Retro* to hang.
* `STBAR` patches wider than 320 pixels will now be displayed correctly when the `vid_widescreen` CVAR is `off`.
* Restoring behavior present in *Vanilla DOOM*, exploding barrels will now trigger line specials.
* Weapons that have been altered using a `DEHACKED` lump to use the `A_FireOldBFG` code pointer will now fire in the right direction based on the values of the `autoaim` and `mouselook` CVARs.
* The correct amount of ammo is now taken from the player when entering `take backpack` in the console.
* A crosshair can now be displayed by enabling the new `crosshair` CVAR. It is `on` by default, but also requires the `mouselook` CVAR to be `on` and the `autoaim` CVAR to be `off`.
* If the `iwadfolder` CVAR is reset using either the `reset` or `resetall` CCMDs, the WAD launcher will try to find a common *DOOM* or *DOOM II* installation again the next time it is opened.
* The value of the `facebackcolor` CVAR is no longer applied to the background of the player’s face in the widescreen HUD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, February 2, 2019

### DOOM Retro v2.8

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Noise is now applied to the menu’s background. Also, if the menu is opened while playing a game, the status bar or widescreen HUD as well as all sprites will be hidden, and the player’s view will slowly rotate either left or right.
* Minor changes have been made to text that is output to the console.
* By enabling the new `autotilt` CVAR, the player’s view will automatically tilt while they go up or down flights of stairs, inspired by a feature present in [*Quake*](https://doomwiki.org/wiki/Quake). This CVAR is both `off` by default and when vanilla mode is enabled, and will also have no effect if the `mouselook` CVAR is `on`.
* The `r_hud` CVAR will now be reset correctly when using the `reset` or `resetall` CCMDs while no game is being played.
* Keycards and skull keys in the widescreen HUD are now spaced slightly further apart.
* The player’s health, ammo and armor in the widescreen HUD will now flash when they decrease as well as increase.
* *DOOM Retro* will no longer hang when the player crosses a line with a *BOOM*-compatible special action of 154 (“WR Change Texture and Effect”) or 240 (“WR Change Texture and Effect to Nearest”).
* Pressing the <kbd><b>F1</b></kbd> key to open the help screen will no longer cause a crash in some very rare instances.
* Minor improvements have been made to the cast sequence at the end of *DOOM II*.
* The value of the `facebackcolor` CVAR is now also applied to the background of the player’s face in the widescreen HUD.
* Further improvements have been made to the support of `DEHACKED` lumps:
  * Things will no longer cast a shadow if they are spawned on the ceiling.
  * A thing’s name can now be changed as intended.
  * The finale text screen will be completely skipped if there is no text to display.
* Further improvements have been made to the support of `MAPINFO` and `RMAPINFO` lumps:
  * Strings containing escaped double quotes can now be used.
  * The title and composer of the music playing in the current map can now be displayed by the `mapstats` CCMD by using the new `musictitle` and `musiccomposer` entries.
* The support for `MUSINFO` lumps has been fixed.
* Music will now quickly fade out when quitting *DOOM Retro*.
* MP3 music lumps now play as intended.
* A crash will no longer occur when trying to display large patches on the intermission screen.
* A bug has been fixed whereby the `unbind` CCMD would only accept an action as a parameter and not a control.
* *DOOM Retro* now more intelligently determines if an animated flat depicts liquid. Only the following animated flats are now considered to be liquid:
  * All animated flats present in the original IWADs (with the exception of `RROCK05` to `RROCK08` and `SLIME09` to `SLIME12`),
  * All animated flats specified in an `ANIMATED` lump in a PWAD that include keywords such as “WATER”, “BLOOD”, “LAVA”, etc. (or certain abbreviations of those) in their names,
  * Certain animated flats specified in an `ANIMATED` lump in a PWAD that are part of the [*Community Chest 4*](https://doomwiki.org/wiki/Community_Chest_4) and [*OTEX*](https://doom.ukiro.com/about-otex/) texture packs.
  * A few animated flats from [a curated list of PWADs](https://github.com/bradharding/doomretro/wiki/Recommended-WADs).
  * All animated flats specified using a `LIQUID` entry in a `MAPINFO` lump in a PWAD.
* The blockmap of every map will now be rebuilt when loaded if `-blockmap` is specified on the command-line.
* The following changes have been made to the `playerstats` CCMD:
  * The number of times the player has saved a game is now displayed.
  * The `Map explored` stat is now completely accurate.
  * The number of cyberdemons and spider masterminds that the player has killed are no longer shown in *DOOM (Shareware)*.
* When saving over an existing savegame, that savegame will now be backed up in the savegame folder.
* A new `take` CCMD has been implemented that can be used to take ammo, armor, health, keys, weapons, or all or certain items from the player. It accepts the same parameters as the `give` CCMD.
* If the player uses the mouse wheel to select the shotgun or super shotgun in *DOOM II*, the first shotgun to be selected when pressing the <kbd><b>3</b></kbd> key will now be set correctly.
* No sound will be made if the player has their fists selected, has the berserk power-up, has no ammunition for any of their weapons, and tries to change weapons using the mouse wheel.
* The player’s health in the widescreen HUD will now flash if it regenerates due to use of the `regenhealth` CCMD.
* Fixing a [bug present in *Vanilla DOOM*](https://doomwiki.org/wiki/Player_face_grins_after_restoring_save_file), the player’s face in the status bar will no longer grin when the first item is picked up after loading a savegame.
* The player’s face in the status bar will now behave correctly when the player injures themselves by exploding a barrel.
* The player will no longer automatically switch to their fists upon picking up a berserk power-up if they have already picked one up elsewhere in the current map.
* There is no longer the possibility of a crash when the player dies and the `mouselook` CVAR is `on` or a control is bound to the `+mouselook` action.
* The `STTMINUS` patch is now positioned better in the widescreen HUD if it has been changed in a PWAD.
* The fixes intended for E1M4 and E1M8 in `doom.wad` are no longer inadvertently applied to E1M4B and E1M8B when the `r_fixmaperrors` CVAR is `on`.
* Minor improvements have been made to the support of [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest).
* Voodoo dolls are now specified in the output of the `thinglist` CCMD.
* The initial sound that a monster makes when it sees the player for the first time will no longer be interrupted by any further sounds that monster makes.
* A bug has been fixed whereby text copied from outside *DOOM Retro* to the *Windows* clipboard and then pasted into the console using <kbd><b>CTRL</b></kbd> + <kbd><b>V</b></kbd> would be corrupt.
* The player’s corpse will now still trigger line specials that exit the map when walked over.
* The vertical distance something is away from blast damage, as well as when telefragging, is no longer taken into account when the `infiniteheight` CVAR is `on`.
* The player’s rocket launcher will now be displayed correctly when fired in vanilla mode.
* Both the `r_messagepos` and `r_messagescale` CVARs have been deprecated. The position and scale of player messages now depend on the value of the `vid_widescreen` CVAR.
* Barrels will now animate correctly if their sprites have been replaced in a PWAD.
* The player’s view will now always be at the correct height when they are spawned at the start of a map.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, November 18, 2018

### DOOM Retro v2.7.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* The following changes have been made to the support of gamepads:
  * The buttons on many *DirectInput* and *XInput* gamepads are now mapped correctly.
  * Gamepads will no longer vibrate if the player punches the air.
  * The `guide` button found on some gamepads can now be bound to an action using the `bind` CCMD.
* A bug has been fixed whereby the flash of the player’s weapon would sometimes be positioned incorrectly if the player was firing when dropping down from a higher sector and the `weaponbounce` CVAR was `on`.
* Further improvements have been made to the support of `DEHACKED` lumps.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, November 4, 2018

### DOOM Retro v2.7.4

* *DOOM Retro* now uses [*SDL v2.0.9*](https://www.libsdl.org), [*SDL_mixer v2.0.4*](https://www.libsdl.org/SDL_mixer) and [*SDL_image v2.0.4*](https://www.libsdl.org/SDL_image).
* When *DOOM Retro* is opened for the first time, the WAD launcher will now try to find a common *DOOM* or *DOOM II* installation.
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* The fuzz effect of spectres, as well as the player’s weapon when they have a partial invisibility power-up, are now paused when freeze mode is on.
* The fuzz effect of spectres is now rendered correctly when the player has an invulnerability power-up.
* The sky will now be rendered correctly when the `r_skycolor` CVAR is a value other than `none` and the player has an invulnerability power-up.
* Further improvements have been made to the support of `DEHACKED` and `MAPINFO` lumps.
* The player’s weapon will now bounce slightly when they drop down from a greater height. This can be disabled using the new `weaponbounce` CVAR, which is `on` by default and `off` when vanilla mode is on.
* Monsters will no longer infight if no target mode is on and the player dies.
* If the `+alwaysrun` action is bound to the <kbd><b>CAPSLOCK</b></kbd> key, then that key will now be toggled on or off as necessary when *DOOM Retro’s* window gains or loses focus, and not just when it is closed.
* Any screen shake or palette effect will now be canceled when pressing <kbd><b>F7</b></kbd> to end a game, or <kbd><b>F9</b></kbd> to quicksave a game.
* The <kbd><b>F12</b></kbd> key can now be bound to an action using the `bind` CCMD.
* The following changes have been made to vanilla mode:
  * Certain controls that weren’t present in *Vanilla DOOM* will now be unbound.
  * The right mouse button will be bound to the `+strafe` action.
  * The automap’s grid is now turned off since it was off by default and its state was never saved in *Vanilla DOOM*.
* Minor improvements have been made to *DOOM Retro’s* renderer.
* The player’s weapon will now rise more smoothly at the start of a map.
* If the original music of *DOOM* or *DOOM II* is being played in a map, then the music’s composer, [Bobby Prince](https://doomwiki.org/wiki/Bobby_Prince), is now displayed by the `mapstats` CCMD.
* The console will now automatically close when a cheat is entered.
* When the `tossdrop` CVAR is `on`, if a monster is killed and then drops an item, some of the corpse’s momentum is now also applied to that item.
* A bug has been fixed whereby the `am_allmapfdwallcolor` CVAR was used instead of the `am_allmapwallcolor` CVAR to draw solid walls in the automap when the player had a computer area map power-up.
* Brightmaps have now been applied to the `SW2GARG`, `SW2LION` and `SW2SATYR` textures when the `r_brightmaps` CVAR is `on`.
* The correct obituary will now be displayed in the console when the player dies on molten rock.
* If both the `r_althud` and `vid_widescreen` CVARs are `on`, and the automap is open, both player messages and the map’s title will now be displayed using *DOOM Retro’s* alternate character set.
* A bug has been fixed whereby gridlines in the top and bottom right corners of the automap weren’t being displayed in some instances when the `am_grid` CVAR was `on`.
* The secret maps will no longer be included when entering `map last`, `map next` or `map random` in the console.
* If a CCMD that requires one or more parameters is entered in the console without those parameters, a description of that CCMD will now be displayed.
* Savegames will now be saved in the correct folder when playing [*Freedoom: Phase 1*](https://freedoom.github.io/) or [*Freedoom: Phase 2*](https://freedoom.github.io/).
* The behavior of the `-savedir` command-line parameter has changed. Savegames will now be placed directly in the folder specified, rather than in a subfolder based on the name of the WAD loaded.
* `-save` may be now be used as an alternative to `-savedir` on the command-line.
* A bug has been fixed whereby the player’s path in the automap wasn’t being shown correctly if both the `am_path` CVAR and no clipping mode were on.
* If the player has more than one power-up, the countdown bar in the alternate widescreen HUD will now always show the power-up that will run out first.
* If an SFX lump in a PWAD is in an unrecognized format, the original lump in the IWAD will be played instead.
* The `+use` action can no longer be used if the `autouse` CVAR is `on`.
* Items dropped by monsters when they are killed will now be rendered correctly if dropped on a moving platform and the `vid_capfps` CVAR is a value other than `35`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 8, 2018

### DOOM Retro v2.7.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* A bug has been fixed whereby gamepads may not initialize correctly when connected more than once during gameplay.
* A counter is now displayed at the end of player messages when they are the same as those preceding them, resembling how those same messages are displayed in the console.
* Further improvements have been made to the support of `MAPINFO` lumps.
* The armor bar in the alternate widescreen HUD is now green or blue to indicate the type of armor the player has.
* The type of armor the player has can now be changed using the new `armortype` CVAR. It can be `none`, `green` or `blue`.
* A bug has been fixed whereby the player wouldn’t move correctly while running if the `+run` action was bound to a mouse button.
* The player will now bob again when moving on a sector that has a rising or lowering floor or ceiling, and the `movebob` CVAR is not `0%`.
* The player’s height will now be lowered as intended when on a liquid sector that has a rising or lowering floor or ceiling, and the `r_liquid_lowerview` CVAR is `on`.
* A gamepad’s right thumbstick can no longer be used to move forward when the `mouselook` CVAR is `off` and the `gp_thumbsticks` CVAR is `2`.
* Looking up and down using a gamepad’s right thumbstick is now smooth when the `mouselook` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, August 24, 2018

### DOOM Retro v2.7.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Slightly more blood splats are now spawned when blood hits the floor.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* A bug has been fixed whereby a crash could occur when loading a savegame in some instances.
* The number in the leftmost column of the output to the `thinglist` CCMD is now the actual ID of each thing that is spawned in the current map.
* Further improvements have been made to the support of `DEHACKED` lumps.
* Pressing <kbd><b>ALT</b></kbd> + <kbd><b>F4</b></kbd> will now instantly quit *DOOM Retro* as originally intended.
* A bug has been fixed whereby the `+zoomin` and `+zoomout` actions couldn’t be rebound from their default <kbd><b>+</b></kbd> and <kbd><b>&ndash;</b></kbd> keys using the `bind` CCMD.
* Mouse acceleration can now be disabled using the new `m_acceleration` CVAR. It is `on` by default and `off` when vanilla mode is enabled.
* Movement of a gamepad’s thumbsticks can now be either analog or digital using the new `gp_analog` CVAR. It is `on` by default and `off` when vanilla mode is enabled.
* The number of thumbsticks used on a gamepad can now be set using the new `gp_thumbsticks` CVAR. If set to `2` (the default), the left thumbstick is used to strafe left/right and move forward/back, and the right thumbstick is used to turn left/right (and look up/down if the `mouselook` CVAR is `on`). If set to `1` (which it is when vanilla mode is enabled), one thumbstick is used to turn left/right and move forward/back.
* A bug has been fixed whereby monsters could be spawned at an incorrect height in some rare instances.
* Some translucency effects have been improved.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, August 4, 2018

### DOOM Retro v2.7.1

* The player will now move correctly when using a gamepad.
* The `r_diskicon` CVAR will now be turned `on` when vanilla mode is enabled.
* Minor changes have been made to text that is output to the console.
* The `restartmap` CCMD will now restart the correct map when playing *E1M4B: Phobos Mission Control* or *E1M8B: Tech Gone Bad*.
* The help screen’s background when pressing the <kbd><b>F1</b></kbd> key is now displayed better when using a custom colormap from a PWAD.
* If the super shotgun was selected by the player more recently than the shotgun, it will now be selected when pressing the <kbd><b>3</b></kbd> key, and vice versa.
* Improvements have been made to the gradual lighting effect under doors and crushing ceilings.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, August 3, 2018

### DOOM Retro v2.7

* With John Romero’s express permission, [*E1M4B: Phobos Mission Control*](https://www.doomworld.com/idgames/levels/doom/Ports/d-f/e1m4b) and [*E1M8B: Tech Gone Bad*](https://www.doomworld.com/idgames/levels/doom/Ports/d-f/e1m8b) are now included with *DOOM Retro*. If either the *DOOM Registered* or *The Ultimate DOOM* IWADs are loaded, these maps may be played by entering `map E1M4B` or `map E1M8B` in the console.
* *DOOM Retro’s* splash screen has been redesigned.
* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The format of savegames has changed, breaking compatibility with previous versions of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* The player can now jump once the new `+jump` action has been bound to a control using the `bind` CCMD.
* Improvements have been made to *BOOM-*compatible translucent wall textures when the `r_dither` CVAR is `on`.
* Further improvements have been made to the support of `DEHACKED` and `MAPINFO` lumps.
* The console’s background has been given a slightly green tint and can now also be changed using the new `con_backcolor` CVAR. It is `12` by default.
* The player is now also given all power-ups when entering `give all` in the console.
* A bug has been fixed whereby entering `give cellpack` in the console would give the player a box of bullets instead.
* The [*DOOM II* monster exclusion bug](https://doomwiki.org/wiki/Doom_II_monster_exclusion_bug) present in *Vanilla DOOM* has been fixed.
* The `vid_widescreen` CVAR can now be changed in the console when not playing a game.
* The `vid_capfps` CVAR can now be set to a value less than `35`.
* Intermission and finale texts are now also output to the console.
* The bottom of lost souls are now clipped when touching a liquid sector.
* A bug has been fixed whereby the blood of monsters could be the wrong color in some rare instances.
* The intensity of color on the screen can now be changed using the new `r_color` CVAR. It can be a value between `0%` (completely grayscale) and `100%` (the default). It is `100%` when vanilla mode is enabled.
* The default of the `am_gridcolor` CVAR has been changed from `7` to `6`.
* The default of the `m_sensitivity` CVAR has been changed from `32` to `16`.
* The default of the `r_gamma` CVAR has been changed from `0.75` to `0.90`.
* Improvements have been made to the effect when the player is near an exploding barrel and the `r_shake_barrels` CVAR is `on`.
* The screen’s pillarboxes are now updated immediately when changing the `vid_pillarboxes` CVAR in the console.
* A bug has been fixed whereby projectiles wouldn’t pass through some map decorations when the `infiniteheight` CVAR was `off`.
* Restoring behavior present in *Vanilla DOOM*, lost souls will no longer pass through non-solid objects while attacking if the `infiniteheight` CVAR is `on`.
* The `mapstats` CCMD now shows the total number of barrels in the current map.
* Keycards and skull keys are now shown in the right hand corner of the widescreen HUD when the player has no armor.
* The automap will now be shown in exactly the same colors as *Vanilla DOOM* when vanilla mode is enabled.
* A bug has been fixed whereby entering an action as a parameter for the `unbind` CCMD wouldn’t unbind the controls that action was bound to.
* If a gamepad with only one thumbstick is connected, movement will then be digital rather than analog and that one thumbstick will both turn the player left/right and move the player forward/back.
* There is no longer any gap between the end of the player’s path and their arrow in the automap when the `am_path` CVAR is `on`.
* <kbd><b>SPACE</b></kbd> can now be pressed to respawn the player, as well as advance the intermission and finale screens, even if the key isn’t bound to the `+use` action.
* The direction the player is looking is no longer recentered vertically when they go through a teleport and the `mouselook` CVAR is `on`.
* An obituary is now displayed when the player is crushed to death by a moving ceiling and the `con_obituaries` CVAR is `on`.
* Whether sound effects are played in mono or stereo can now be changed using the new `s_stereo` CVAR. It is `on` by default and when vanilla mode is enabled.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, March 31, 2018

### DOOM Retro v2.6.9

* The targets of monsters will now be restored correctly when loading a savegame.
* The player’s view will no longer go past the floor or ceiling in some rare instances.
* A bug has been fixed whereby the player would fire their weapon when the game was unpaused using the <kbd><b>PAUSE</b></kbd> key.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, March 29, 2018

### DOOM Retro v2.6.8

* *DOOM Retro* now uses [*SDL v2.0.8*](http://libsdl.org) and [*SDL_image v2.0.3*](http://libsdl.org/SDL_image).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Improvements have been made to how both sprites and the shadows they cast are rendered.
* Minor changes have been made to text that is output to the console.
* Player messages can no longer be present in screenshots taken using the <kbd><b>PRINTSCREEN</b></kbd> key.
* Spectres and the shadows they cast are now displayed correctly when the `r_textures` CVAR is `off`.
* Further improvements have been made to the support of `DEHACKED` and `MAPINFO` lumps.
* The player’s face is no longer updated in either the status bar or the default widescreen HUD when freeze mode is on.
* The screen is now rendered correctly while the player has an invulnerability power-up and the `r_textures` CVAR is `off`.
* A bug has been fixed whereby some map-specific fixes enabled using the `r_fixmaperrors` CVAR weren’t being applied.
* Hanging corpses no longer bob up and down if above liquid.
* Corpses can no longer trigger line specials when sliding over them.
* Fixing a bug present in *Vanilla DOOM*, monsters will no longer momentarily freeze when trying to open certain locked doors.
* The correct map names will now be displayed when playing [*Freedoom*](https://freedoom.github.io/).
* An error will no longer occur when trying to load [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* The screen will now fade to black upon quitting from either [*Freedoom*](https://freedoom.github.io/) or [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* Liquid sectors are now rendered slightly higher when the `r_liquid_bob` CVAR is `on` to improve the bottom edge of surrounding tileable wall textures.
* Projectiles will now pass through map decorations when the `infiniteheight` CVAR is `off`.
* Corpses in liquid no longer bob up and down in time with each other.
* The positioning of the monsters in *DOOM II’s* cast sequence has been improved when the `r_fixspriteoffsets` CVAR is `on`.
* The underscores in the message displayed when using the `IDBEHOLD` cheat now align correctly.
* A bug has been fixed whereby savegames could become corrupted in some rare instances.
* A bug present in *Vanilla DOOM* has been fixed whereby certain switches wouldn’t turn on when used by the player.
* Repeatable switches that are adjacent to a moving sector will no longer make a second sound when they turn off.
* The alternate widescreen HUD and player messages are now black rather than white when the player has the invulnerability power-up or the `r_textures` CVAR is `off`.
* The `vid_screenresolution` and `vid_windowsize` CVARs are now validated better at startup and when changed in the console.
* Player messages will now always be positioned correctly when the `r_messagepos` CVAR is changed from its default of `(3,2)`.
* The map title in the automap is now always positioned correctly when the `r_messagescale` CVAR is `small`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, February 23, 2018

### DOOM Retro v2.6.7

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The format of savegames has changed, breaking compatibility with previous versions of *DOOM Retro*.
* A bug has been fixed whereby skull keys picked up by the player wouldn’t be displayed in the status bar in some instances.
* Entering `give keys` in the console will now give the player all keycards and skull keys, rather than just those present in the current map.
* Entering `give keycards` or `give skullkeys` in the console will now give the player all keycards or all skull keys.
* Minor changes have been made to text that is output to the console.
* Walls, ceilings and floors with missing textures will now be rendered in white rather than not at all.
* A bug has been fixed whereby some skies weren’t being rendered correctly when the `mouselook` CVAR was `off`.
* Blood splats are now lit correctly when the `r_textures` CVAR is `off`.
* The brightmap of the `COMP2` texture has been improved.
* As [originally intended in *Vanilla DOOM*](https://doomwiki.org/wiki/Sound_cutoffs#DSFLAMST), the `DSFLAMST` sound effect will now be played when an arch-vile attacks the player or another monster.
* The AI of monsters has been improved when on or next to a lift.
* How far away a monster is vertically from the player during its melee attack is no longer taken into account if the `infiniteheight` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, February 9, 2018

### DOOM Retro v2.6.6

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby the wall textures visible beyond a moving sector would shift slightly.
* Improvements have been made to how blood splats are spawned.
* Entering text in the console has now been improved for non-US keyboard layouts.
* Minor changes have been made to text that is output to the console.
* Early versions of *DOOM Shareware* will now run without quitting with an error.
* When the player’s health or ammo is low, now only the values themselves will flash in the default widescreen HUD.
* Multiple keycards and skull keys are now positioned better in the default widescreen HUD.
* A bug has been fixed whereby keycards and skull keys weren’t appearing at all in the alternate widescreen HUD.
* Keycards and skull keys in both the default and alternate widescreen HUDs now appear in the order they were picked up by the player.
* Minor improvements have been made to the alternate widescreen HUD.
* A bug has been fixed whereby a monster’s position would be interpolated when teleporting if the `vid_capfps` CVAR was a value other than `35`.
* Giving the player and monsters infinite height can now be toggled on or off using the new `infiniteheight` CVAR. This CVAR is `off` by default and `on` when vanilla mode is enabled.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 26, 2018

### DOOM Retro v2.6.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby the wrong description would be displayed when entering a CVAR without a value in the console.
* The rendering of two-sided textures has been improved.
* The widescreen HUD has been completely redesigned and now includes the player’s face instead of a medikit.
* The `r_althud` CVAR is now `off` by default.
* When the `r_textures` CVAR is `off`, shadows cast by monsters are now still translucent when the `r_shadows_translucency` CVAR is `on`, and *BOOM*-compatible translucent wall textures are now still translucent when the `r_translucency` CVAR is `on`.
* A crash will no longer occur when picking up an invulnerability power-up on the same map after loading a savegame.
* The `playername` CVAR is now used instead of “OUR HERO” when displaying the player in *DOOM II’s* cast sequence.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 19, 2018

### DOOM Retro v2.6.4

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby an error would be displayed when trying to load `doom1.wad`.
* A new `restartmap` CCMD has been implemented that restarts the current map.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* A bug present in *Vanilla DOOM* has been fixed whereby [the player would sometimes bounce off walls](https://doomwiki.org/wiki/Elastic_collisions_with_walls).
* The `unbind` CCMD will now also accept an action as its parameter, unbinding all keyboard, mouse and gamepad controls bound to that action.
* The `resetall` CCMD will now also reset all bound controls to their default actions.
* A bug has been fixed whereby the bottom edge of spectres and their blood weren’t drawn correctly in some instances.
* The effects of changing the `r_blood` CVAR between `all` and `red` are now immediate.
* Movement of the player arrow in the automap is now smoother.
* The vertical axis of a *DirectInput* gamepad’s right thumbstick when looking up and down will now be inverted when the `gp_invertyaxis` CVAR is `on`.
* The shadows cast by the monsters in *DOOM II’s* cast sequence are now more consistent with how they appear during a game.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 5, 2018

### DOOM Retro v2.6.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The following changes have been made to the boss in *MAP30: Icon Of Sin* at the end of *DOOM II: Hell On Earth*:
  * Its projectiles will now move downwards.
  * The explosions when it is destroyed are now translucent if the `r_translucency` CVAR is `on`.
* The screen will now quickly fade to black when quitting *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* The keycards and skull keys displayed in the alternate widescreen HUD now use the most dominant colors of those picked up by the player.
* A bug has been fixed whereby the screen would continue to shake after the player had died in some rare instances if the `r_shake_damage` CVAR was `on`.
* Vertical autoaiming as the player fires their weapon while using mouselook can now be toggled on and off using the new `autoaim` CVAR. This CVAR is `on` by default.
* The player is now given a berserk power-up when entering the `IDFA` or `IDKFA` cheats so they can then still equip their fists.
* The following changes have been made when freeze mode is on:
  * The bob of the player and the player’s weapon are disabled.
  * The `give` CCMD will now work correctly.
  * Any power-ups the player has will no longer time out.
  * The player’s health will no longer regenerate if the `regenhealth` CCMD has been used.
* The screen will now be updated immediately when entering the `IDBEHOLDL` or `IDBEHOLDV` cheats in the console.
* Areas outside of the map (accessible when either freeze mode or no clipping mode are on) are now white rather than black when the player has an invulnerability power-up.
* The `-config` command-line parameter will no longer be ignored when saving the configuration file.
* The `-shotdir` command-line parameter can now be used to specify the folder that screenshots will be saved in when the <kbd><b>PRINTSCREEN</b></kbd> key is pressed.
* Objects will no longer be lit incorrectly in some rare instances.
* The shadows cast by monsters will now be displayed correctly in areas with a custom colormap.
* The shadows cast by spectres will now be displayed correctly when the `r_shadows_translucency` CVAR is `off`.
* Using the `nomonsters` CCMD will now instantly remove all monsters in the current map.
* The brightmaps for several wall textures are now fixed.
* A bug present in *Vanilla DOOM* has been fixed whereby [Mancubi projectiles would sometimes pass through walls](https://doomwiki.org/wiki/Mancubus_fireball_clipping).

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, December 16, 2017

### DOOM Retro v2.6.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The effects of changing the `gp_swapthumbsticks` CVAR are now immediate.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* A bug has been fixed whereby changing the `vid_screenresolution` CVAR to a value other than `desktop` wouldn’t change the screen resolution.
* Pressing <kbd><b>ALT</b></kbd> + <kbd><b>ENTER</b></kbd> to toggle between fullscreen and a window will now work when the `vid_screenresolution` CVAR is a value other than `desktop`.
* Both player messages and the map title in the automap are no longer truncated in the middle of the screen in some instances.
* Sprites that are replaced in PWADs will now be offset correctly.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, December 7, 2017

### DOOM Retro v2.6.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby sound effects weren’t playing correctly for some users.
* The rendering of floor and ceiling textures has been improved.
* A bug has been fixed whereby there was no effect when the `r_corpses_nudge` CVAR was `on`.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* Weapon recoil is now reset when the player is either teleported or resurrected.
* The `freeze` CCMD can now only be used when in a game.
* The player’s view now tilts upward in time with it also lowering to the floor when the player dies and either the `mouselook` CVAR is `on` or a control is bound to the `+mouselook` action.
* The infighting among monsters once the player dies can now be toggled on or off using the new `infighting` CVAR. This CVAR is `on` by default and `off` when vanilla mode is enabled.
* An incorrect obituary is no longer displayed in the console when the player uses the `kill` CCMD to kill themselves.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, November 25, 2017

### DOOM Retro v2.6

* *DOOM Retro* now uses [*SDL v2.0.7*](http://libsdl.org), [*SDL_mixer v2.0.2*](http://libsdl.org/SDL_mixer) and [*SDL_image v2.0.2*](http://libsdl.org/SDL_image).
* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The format of savegames has changed, breaking compatibility with previous versions of *DOOM Retro*.
* A bug has been fixed whereby the help screen accessed using the <kbd><b>F1</b></kbd> key had a solid blue background.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* The player’s field of view can now be changed using the new `r_fov` CVAR. This CVAR can be a value between `45` and `135`, and is `90` by default and when vanilla mode is enabled.
* Using the `vanilla` CCMD in an alias will now work correctly.
* Strings of commands, separated by semi-colons, can now be entered directly in the console.
* Most actions can now be entered directly in the console.
* The `bind` CCMD can now be used to bind a string of commands to a control. For example, to press the <kbd><b>V</b></kbd> key to enable vanilla mode without lowering the graphic detail, enter `bind 'v' "vanilla; r_detail high"` in the console.
* A custom message can now be displayed using the new `print` CCMD.
* A new `if` CCMD has been implemented that allows a string of commands to be executed only if a CVAR equals a certain value.
* The player’s view is now updated immediately when on a moving platform.
* Items dropped by monsters when they are killed are now spawned above rather than on the floor before being tossed upwards.
* Tossing items dropped by monsters when they are killed can now be toggled on or off using the new `tossdrop` CVAR. This CVAR is `on` by default and `off` when vanilla mode is enabled.
* The position of player messages can now be changed using the new `r_messagepos` CVAR. This CVAR is `(3,2)` by default and `(0,0)` when vanilla mode is enabled.
* The `m_acceleration` and `m_threshold` CVARs have been removed.
* The `vid_windowposition` CVAR has been shortened to just `vid_windowpos`.
* If they can be found, the *Final DOOM* IWADs (`plutonia.wad` and `tnt.wad`) will now automatically be loaded for certain PWADs that require them.
* Brightmaps are now applied to more wall textures in *Final DOOM: TNT - Evilution* when the `r_brightmaps` CVAR is `on`.
* The player can now automatically use doors and switches if they are near enough by enabling the new `autouse` CVAR. This CVAR is `off` by default.
* A crash will no longer occur when trying to switch between fullscreen and a window by pressing <kbd><b>ALT</b></kbd> + <kbd><b>ENTER</b></kbd> while on the title screen.
* Blood splats will no longer be spawned around corpse decorations if their sprites have been changed in a PWAD.
* Long lines are no longer truncated in files output by the `condump` CCMD.
* A bug has been fixed whereby a corrupted player message would be displayed when trying to open a locked door in some instances.
* The vertical position of the player’s weapon is now reset immediately when the `mouselook` CVAR is turned `off`.
* Improvements have been made to the effect when the player is damaged and the `r_shake_damage` CVAR is `on`.
* A time limit for each map can now be set in minutes using the new `timer` CCMD, functioning like the command-line parameter of the same name.
* Pain elementals killed using the `kill` CCMD are now counted correctly in the stats displayed by the `playerstats` CCMD.
* The media keys on some keyboards now work correctly if pressed while *DOOM Retro* is running.
* The shadows cast by monsters when the `r_shadows` CVAR is `on` no longer bleed into the bottom edge of the player’s view, or into a higher sector in the foreground.
* The `m_doubleclick_use` CVAR is now turned `on` and the `vid_showfps` CVAR `off` when vanilla mode is enabled.
* An obituary is now displayed in the console when the player is killed by an exploding barrel or a damaging sector, and the `con_obituaries` CVAR is `on`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, October 11, 2017

### DOOM Retro v2.5.7

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The icon of `doomretro.exe` has been redesigned.
* Elevators now move smoothly when the `vid_capfps` CVAR is a value other than `35`.
* The dimensions displayed by the `mapstats` CCMD are now correct for extremely large maps.
* The vertical sensitivity of the gamepad’s right thumbstick has been reduced slightly when the `mouselook` CVAR is `on`.
* The spectre’s shadow will now pause if the console is opened during *DOOM II’s* cast sequence.
* Improvements have been made to the console’s autocomplete feature.
* *XInput*-compatible gamepads will now vibrate when the player is near an exploding barrel. The amount of vibration can be specified using the new `gp_vibrate_barrels` CVAR, which can be a value between `0%` and `200%` and is `100%` by default.
* A bug has been fixed whereby a gamepad could continue to vibrate if the player had the chainsaw selected and then used the gamepad to start a new game or load an existing one from the menu.
* The `vid_scalefilter` CVAR will now affect the external automap correctly when the `am_external` CVAR is `on`.
* An evil grin will now be displayed in the status bar when the player uses the `IDFA` or `IDKFA` cheats.
* Minor changes have been made to text that is output to the console.
* Obituaries in the console now indicate if the player has a berserk power-up when killing a monster with their fists.
* The following changes have been made to all CVARs that specify a color:
  * They may now be set in the console to a hexadecimal string of the format `#rrggbb`. The CVAR will then be set to the index of the closest color in *DOOM’s* 256-color palette.
  * They may now also be set to one of the following color names: `black`, `blue`, `brick`, `brown`, `cream`, `darkbrown`, `darkgray`, `darkgreen`, `darkred`, `gold`, `gray`, `green`, `lightblue`, `olive`, `orange`, `purple`, `red`, `tan`, `white` or `yellow`.
* Any lumps in a PWAD between `HI_START` and `HI_END` markers will now be ignored.
* A bug has been fixed whereby when unbinding certain controls using the `unbind` CCMD, that control would then be bound again the next time *DOOM Retro* was run.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 30, 2017

### DOOM Retro v2.5.6

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The effects of changing the `r_corpses_color` CVAR are now immediate.
* A bug has been fixed whereby the player would restart the current map immediately upon death in some rare instances.
* <kbd><b>TAB</b></kbd> and <kbd><b>SHIFT</b></kbd> + <kbd><b>TAB</b></kbd> may now also be used in the console to autocomplete the parameters of most CCMDs and CVARs.
* When entering an alias previously created using the `alias` CCMD, the alias itself will now be added to the console’s input history rather than the contents of the alias.
* Minor changes have been made to some of the text in the console.
* The console’s background will now be updated if opened on the credits screen.
* Changes have been made to the status bar’s background when the `r_detail` CVAR is `high`.
* The maximum value that the `s_channels` CVAR can be set to is now `64`.
* IWADs and PWADs can now be specified on the command-line without a `.wad` extension.
* The slight current enabled using the `r_liquid_current` CVAR will no longer be applied to liquid sectors that also have a *BOOM*-compatible scrolling effect.
* When using the `idclip` cheat, `idclip` will now be displayed in the console rather than `idspispopd`.
* A crash will no longer occur when trying to display the spectre in *DOOM II’s* cast sequence.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, September 8, 2017

### DOOM Retro v2.5.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Some stray dark pixels have been removed from the top of the super shotgun’s muzzle when firing.
* The shadows of crushed monsters are now positioned correctly. This changes the format of savegames, breaking compatibility with previous versions of *DOOM Retro*.
* The correct names are now shown for MAP31, MAP32 and MAP33 of *DOOM II: Hell On Earth (BFG Edition)*.
* A bug has been fixed whereby a ceiling could move through a floor, or vice versa, in some rare instances.
* The intermission screen will no longer wrongly indicate that the player is about to enter the next map when having exited the final map.
* Cacodemons and lost souls will no longer drift upwards after being shot at.
* Monsters will no longer be pushed under the floor if shot at while being crushed by a lowering ceiling.
* Further improvements have been made to lowering the player’s view in liquid sectors when the `r_liquid_lowerview` CVAR is `on`.
* A bug has been fixed whereby the slight current enabled using the `r_liquid_current` CVAR wasn’t being applied to some liquid sectors.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, August 26, 2017

### DOOM Retro v2.5.4

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The behavior of lost souls has been restored to better resemble *Vanilla DOOM*.
* Wiping when transitioning between screens can now be toggled on or off using the new `wipe` CVAR.
* Weapon recoil is now disabled when vanilla mode is enabled.
* A bug has been fixed whereby monsters could use stairs in some instances when they shouldn’t have been able to.
* Chaingunners, arachnotrons and spider masterminds now light up more in time with their firing animations.
* A value of `off` can now be used for the `facebackcolor` CVAR and is the same as using the default value of `5`.
* A bug has been fixed whereby the sky would be slightly darker than normal when the player has the light amplification visor power-up.
* When pressing the <kbd><b>PRINTSCREEN</b></kbd> key while the `am_external` CVAR is `on`, a screenshot of both screens will now be taken rather than two screenshots of the same screen.
* The `vid_capfps`, `vid_scalefilter` and `vid_vsync` CVARs will now affect the external automap when the `am_external` CVAR is `on`.
* Obituaries in the console now correctly reflect when the player or a monster is telefragged.
* Pressing <kbd><b>CTRL</b></kbd> + <kbd><b>&uarr;</b></kbd>/<kbd><b>&darr;</b></kbd> can now be used as well as <kbd><b>PGUP</b></kbd>/<kbd><b>PGDN</b></kbd> to scroll the output in the console up and down.
* The `r_gamma` CVAR can now correctly be set to `2.0` in the console and at startup.
* When the `vid_capfps` CVAR is a value other than `35`, rockets and plasma rifle and BFG-9000 projectiles are now slightly further away from the player when fired.
* Further improvements have been made to lowering the player’s view in liquid sectors when the `r_liquid_lowerview` CVAR is `on`.
* Reducing the `health` CVAR will now work correctly when playing the *I’m too young to die* skill level.
* If no IWAD is specified, *DOOM Retro* will now also check for an installation of *DOOM 3: BFG Edition* purchased through [*GOG.com*](https://www.gog.com/game/doom_3_bfg_edition).

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, July 31, 2017

### DOOM Retro v2.5.3

* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The power-up bar is now displayed in the alternate widescreen HUD when the player has the berserk power-up and their fist selected, and when the `IDBEHOLDx` cheat is used.
* MOD and XM music lumps will now play correctly.
* Improvements have been made to lowering the player’s view in liquid sectors when the `r_liquid_lowerview` CVAR is `on`.
* Minor changes have been made to text that is output to the console.
* A bug has been fixed whereby moving sectors could shudder when the `vid_capfps` CVAR was set to value other than `35` in some rare instances.
* The bottom of masked midtextures submerged in liquid will now render correctly when the `r_liquid_bob` CVAR is `on` and the `vid_capfps` CVAR is set to value other than `35`.
* A bug has been fixed whereby some stray pixels weren’t being drawn in some rare instances.
* Pain elementals can no longer spawn lost souls behind their target when directly in front of them.
* The Hall of Mirrors indicator enabled using the `r_homindicator` CVAR will now work when in no clipping mode and freeze mode.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, July 16, 2017

### DOOM Retro v2.5.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Savegames for *DOOM II: No Rest For The Living* are now saved in the correct folder, rather than in the same folder as *DOOM II: Hell On Earth* savegames.
* A bug has been fixed whereby the frames per second displayed by enabling the `vid_showfps` CVAR wouldn’t display correctly in some instances.
* The fuzz effect of spectres no longer bleeds past the bottom of the player’s view, or into a higher sector when partially obscured by it.
* The `spawn` CCMD can now be used to spawn monsters in a map when the `nomonsters` CCMD or `-nomonsters` command-line parameter have been used.
* The `+mouselook` action may now be bound to a control. Mouselook will then only work while that control is held down. Releasing that control will then cause the player’s view to spring back.
* Blood will no longer be spawned when crushing the player while they are invulnerable.
* A bug has been fixed whereby the player’s rocket launcher would shift to the left when fired.
* Frames from *DOOM’s* rocket launcher are no longer shown when firing the missile launcher in [*Freedoom*](https://freedoom.github.io/).

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, July 5, 2017

### DOOM Retro v2.5.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* If no IWAD is found when loading a PWAD using the WAD launcher (either in the same folder as the PWAD or the folder specified by the `iwadfolder` CVAR), several common installation folders will now be checked.
* The introductory message is no longer displayed when opening *DOOM Retro* for the first time.
* Minor changes have been made to text that is output to the console.
* A bug has been fixed whereby a crash could occur when exiting a map in some instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, June 30, 2017

### DOOM Retro v2.5

* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor improvements have been made to *DOOM Retro’s* renderer.
* The gray elements in the alternate widescreen HUD now appear correctly in PWADs with custom `PLAYPAL` lumps.
* Minor changes have been made to some elements in both the status bar and alternate widescreen HUD.
* A countdown bar is now displayed in the alternate widescreen HUD underneath the ammo bar when the player has a power-up that runs out.
* Mouselook can now be enabled using the new `mouselook` CVAR. It is `off` by default. (Please note that due to the addition of this feature, savegames created with previous versions of *DOOM Retro* are not compatible with this version.)
* An `m_invertyaxis` CVAR has also been implemented that toggles inverting the mouse’s vertical axis when using mouselook. It is `off` by default.
* Looking up and down using the gamepad’s right thumbstick has also been implemented, and is enabled when the new `mouselook` CVAR is `on` as well. A new `gp_invertyaxis` CVAR can be used to invert the vertical axis of the gamepad’s right thumbstick. It is `off` by default.
* The following changes have been made to some of the stats displayed by the `playerstats` CCMD:
  * The `Shots Fired`, `Shots Hit` and `Weapon Accuracy` stats are now calculated correctly.
  * The `Ammo`, `Armor` and `Health` stats no longer increase when using the `give` CCMD or certain cheats.
  * The `Health` stat now increases when the player picks up a health bonus.
  * The number of barrels in the current map as part of the `Barrels exploded` stat is now calculated correctly.
* The following changes have been made to some of the stats displayed by the `mapstats` CCMD:
  * The `Damaging Sectors` stat now takes *BOOM*-compatible damaging sectors into account.
  * The `Pickups` stat is now calculated correctly.
* Several compatibility fixes have been implemented for:
  * [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens),
  * [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1),
  * [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960),
  * [*TNT: Revilution*](https://www.doomworld.com/files/file/18810-tnt-revilution/).
* Three new CVARs have been implemented to individually toggle the translucency of certain elements: `r_bloodsplats_translucency`, `r_hud_translucency` and `r_shadows_translucency`. They are all `on` by default. (The `r_translucency` CVAR remains to toggle the translucency of sprites and *BOOM*-compatible wall textures.)
* The console’s background is now always translucent.
* Further improvements have been made to the support of `DEHACKED` and `MAPINFO` lumps.
* The ability to have the player’s weapon recoil when fired can now be enabled using the new `weaponrecoil` CVAR. (Mouselook also needs to be enabled using the `mouselook` CVAR.)
* The [wallrunning](https://doomwiki.org/wiki/Wallrunning) exploit present in *Vanilla DOOM* is now fixed.
* The number of sound effects that can be played at the same time is now specified using the new `s_channels` CVAR. It can be between `8` and `256`, and is `32` by default.
* The skull in the menu is no longer positioned incorrectly when certain PWADs with custom menu lumps are loaded.
* Minor changes have been made to text that is output to the console.
* The bottom of spectres are now clipped correctly when partially obscured by a higher sector.
* The player will now bleed fuzzy rather than red blood if injured while they have a partial invisibility power-up.
* The gold palette effect will no longer stay on the screen if the player picks up an item and dies at the same time.
* Red walls in the automap are now drawn thinner when the player zooms out far enough.
* If the `r_skycolor` CVAR is set to a color rather than `none`, that color will now be used to render the sky when the `r_textures` CVAR is `off`.
* When exiting a map that has no monsters to kill and/or no items to pick up, the intermission screen will now show `100%` kills and/or items instead of `0%`.
* Kills will now be shown correctly on the intermission screen when playing either [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/) or [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest).
* The `maplist` CCMD will now work correctly when playing [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* The name of the map will now be displayed on the intermission screen before starting MAP31 in *DOOM II* and MAP09 in *DOOM II: No Rest For The Living*.
* Monster corpses are now gibbed rather than just becoming a pool of blood splats when crushed by a lowering platform or door.
* Monster corpses spawned at the start of a map as decorations can now be crushed by a lowering platform or door.
* The `health` CVAR can now be reduced when freeze mode is on.
* A bug has been fixed whereby blood splats could be black in some rare instances.
* Blood splats now render correctly when against the left edge of the screen.
* The default of the `r_bloodsplats_max` CVAR has been increased to `65,536`.
* All blood splats are now restored when turning off vanilla mode.
* Barrels are no longer fullbright for the first two frames of their animation when exploding.
* A gradual lighting effect has been applied to sectors under crushing ceilings.
* The length of each frame in milliseconds is now displayed along with the number of frames per second when the `vid_showfps` CVAR is `on`.
* The `episode`, `expansion`, `savegame` and `skilllevel` CVARs are now integers rather than strings, and are no longer read-only.
* If the `skilllevel` CVAR is changed in the console, the skill level will be changed for the next map.
* A warning is now displayed when changing the `nomonsters` or `pistolstart` CCMDs, or the `r_fixmaperrors` or `skilllevel` CVARs, indicating that the change won’t be effective until the next map is loaded.
* The effects of using the `respawnitems` CCMD are now immediate, respawning all items picked up so far in the current map.
* The `s_timiditycfgpath` CVAR has been removed.
* The effects of changing the `r_fixspriteoffsets` CVAR are now immediate.
* The grid in the automap is now slightly darker.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, April 7, 2017

### DOOM Retro v2.4.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Sprites taller than 255 pixels are now supported.
* The map title in the automap is now positioned correctly when the `r_messagescale` CVAR is `small`.
* Frames from *DOOM’s* rocket launcher are no longer shown when firing the missile launcher in [*Freedoom*](https://freedoom.github.io/).
* Entering the `IDMYPOS` cheat will no longer cause a crash.
* Pressing the <kbd><b>DEL</b></kbd> key when in the save or load game menus will now delete the currently selected savegame.
* Minor changes have been made to text that is output to the console.
* The inverted gray color palette is now applied to the sky when the player has the invulnerability power-up, as originally intended.
* A bug has been fixed whereby blood splats would no longer be spawned after loading a savegame in some instances. Please note that because of this, savegames created with previous versions of *DOOM Retro* are not compatible with this version.
* Another 29 map-specific fixes, enabled using the `r_fixmaperrors` CVAR, have been applied to maps in the `doom.wad`, `doom2.wad` and `plutonia.wad` IWADs.
* The text caret shown when entering a savegame description in the save game menu is now always a vertical line using the dominant color of the character set. (Previously, the `STCFN121` lump was used. In the *DOOM* and *DOOM II* IWADs this lump is a vertical pipe character, but in some PWADs it is replaced with a “Y” character.)
* The sound of the chainsaw will no longer cut off sounds made by the player.
* A bug has been fixed whereby translucent sprites would become less bright when the player had the light amplification visor power-up.
* A bug present in *Vanilla DOOM* has been fixed whereby homing rockets fired by revenants would randomly become non-homing, and vice versa, when loading a savegame or when pausing then unpausing a game.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, March 27, 2017

### DOOM Retro v2.4.4

* A bug has been fixed whereby a crash would often occur when the player died and the `vid_widescreen` CVAR was `off`.
* The value of the `r_messagescale` CVAR is now displayed correctly in `doomretro.cfg`.
* Over 200 additional map-specific fixes, enabled using the `r_fixmaperrors` CVAR, have been applied to maps in `doom.wad`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, March 26, 2017

### DOOM Retro v2.4.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The <kbd><b>ENTER</b></kbd> key may now be used as an alternative to the <kbd><b>Y</b></kbd> key when responding to messages requiring a yes/no answer.
* When the player dies, the amount of health displayed in the widescreen HUD will no longer always be zero. It will instead often be a negative number to indicate how much damage was inflicted upon the player in order to kill them.
* A bug has been fixed whereby the `ammo` and `armor` CVARs could be set to negative values and cause a crash.
* Animated decorations will no longer shift back and forth in the final release of [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) or if the [*Minor Sprite Fixing Project v1.8*](https://www.doomworld.com/idgames/graphics/sprfix18) is loaded.
* The message displayed when the player tries to open a locked door when they don’t have the required key now always distinguishes between keycards and skull keys.
* An `r_messagescale` CVAR has been implemented to allow the scale of messages to be changed between `big` and `small`. It is `big` by default.
* The messages displayed in the alternate widescreen HUD now use the same font that is used in the console.
* The player’s path in the automap is now disabled when vanilla mode is enabled.
* An additional 300 map-specific fixes, enabled using the `r_fixmaperrors` CVAR, have been applied to maps in both `doom.wad` and `doom2.wad`.
* The following bugs from *Vanilla DOOM* have been fixed:
  * Missiles no longer explode when impacting with the sky in some instances.
  * Lost souls will no longer slowly drift backwards when attacked while charging.
  * Lost souls will no longer forget their target immediately after attacking them.
* When using the `kill` CCMD to kill all monsters, all missiles fired by those monsters will now explode at the same time. Also, a `missiles` parameter may now be used to only explode the missiles.
* `ccmdlist` can now be used as an alternative to the `cmdlist` CCMD, and `explode` as an alternative to the `kill` CCMD.
* The `IDCLEV` cheat may now be used in the console while no map is loaded.
* The maximum value the `r_screensize` CVAR can be set to is now `7` rather than `8`. Setting the CVAR to `8` previously had no effect.
* The player can no longer trigger secrets when in freeze mode.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, March 8, 2017

### DOOM Retro v2.4.2

* *DOOM Retro* is now compiled using [*Microsoft Visual Studio Community 2017*](https://www.visualstudio.com/vs/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The *Fortress Of Mystery* is now shown on the intermission screen for the remainder of the episode when the player finds the secret exit to E2M9 in `doom.wad`.
* The music volume is now set correctly at startup.
* Greater precision is now possible when changing the SFX or music volume through the options menu.
* A bug has been fixed whereby some player stats saved in `doomretro.cfg` could become corrupted in some instances.
* A PWAD whose header is incorrectly marked as an IWAD can now be opened using the WAD launcher provided a valid IWAD is selected with it.
* Minor changes have been made to text that is output to the console.
* A bug has been fixed whereby it was possible for some controls to be bound twice to the same action in `doomretro.cfg`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, February 28, 2017

### DOOM Retro v2.4.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The following changes have been made to accommodate for the release of [*Back To Saturn X E1: Get Out Of My Stations v1.0*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1):
  * Teleports are no longer treated as liquid.
  * If only `btsx_e1a.wad` is opened using the WAD launcher, then `btsx_e1b.wad` is automatically opened as well, and vice-versa.
* The header of WADs specified on the command-line using the `-file` parameter will no longer be checked.
* A bug has been fixed whereby the super shotgun would appear entirely translucent when the player fired it.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, February 27, 2017

### DOOM Retro v2.4

* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*, particularly when rendering blood splats and shadows.
* Improvements have been made to *DOOM Retro’s* renderer.
* Optimizations have been made to the size of savegames and therefore those created using previous versions of *DOOM Retro* are not compatible with this version.
* `doomretro.exe` is now compressed using [*UPX (Ultimate Packer for eXecutables) v3.93*](https://upx.github.io/).
* Maps built using a new node/blockmap builder called [*ZokumBSP*](http://doom2.net/zokum/zokumbsp/) are now supported.
* The filename of a WAD may now be entered manually in the WAD launcher without its “.WAD” extension.
* A “vanilla mode” may now be toggled on or off using the new `vanilla` CCMD. Enabling this mode will disable several features to make *DOOM Retro* look as close to *Vanilla DOOM* as possible. Any further changes to CVARs in the console won’t be saved while this mode is enabled. When this mode is then disabled, or the player quits *DOOM Retro* altogether, all CVARs will be restored to their values prior to enabling this mode.
* A bug has been fixed whereby sectors would be incorrectly identified as liquid when loading a savegame in some rare instances.
* The palette effects from the berserk and radiation shielding suit power-ups are now retained after changing a CVAR in the console starting with `vid_` that resets the video subsystem.
* The radiation shielding suit power-up is now closer to the ground.
* The “pile of skulls and candles” decoration now spawns with blood splats at its base when the `r_corpses_moreblood` CVAR is `on`.
* Changing the `r_translucency` CVAR will now immediately affect blood splats.
* Minor changes have been made to text that is output to the console.
* A compatibility fix has been implemented that disables the translucency of the player’s weapons in [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens).
* Further improvements have been made to the support of `DEHACKED` and `MAPINFO` lumps.
* The `alwaysrun` CVAR will no longer be reset to `off` at startup.
* A bug has been fixed whereby single quotes couldn’t be used in the *control* parameter when entering the `bind` CCMD in the console.
* All textures can now be toggled off using the new `r_textures` CVAR.
* The following improvements have been made to “freeze mode”, enabled using the `freeze` CCMD:
  * All moving floors and ceilings, as well as scrolling textures, are now disabled.
  * Doors and switches can no longer be used by the player.
  * “No clipping mode” is also enabled.
  * Liquid sectors that are off of the screen when this mode is enabled are now drawn correctly.
* The `r_dither` CVAR is now `off` by default.
* The `vid_scalefilter` CVAR is now `"nearest"` by default.
* A bug has been fixed whereby some MP3 music lumps would either play incorrectly or not at all.
* The following improvements have been made to the `mapstats` CCMD:
  * The format of the currently playing music lump (`MIDI (converted from MUS)`, `MIDI`, `Ogg Vorbis`, `MP3`, `WAV`, `FLAC` or `MOD`) is now displayed.
  * The overall height of the map is now also displayed in the map’s dimensions.
  * The dimensions of the current map are now displayed in feet/miles or meters/kilometers as specified by the `units` CVAR.
  * Whether the map is “limit removing” is now indicated.
* Objects no longer bob up and down when underwater.
* If a PWAD contains a sound lump called `DSSECRET`, it will now be played along with a message when the player finds a secret area.
* A `regenhealth` CCMD has been implemented that toggles the ability of the player’s health to regenerate at a rate of 1% per second when it drops below 100%.
* A bug has been fixed whereby projectiles fired to or from a monster or the player standing in liquid were still being lowered when the `r_liquid_clipsprites` or `r_liquid_lowerview` CVARs were `off`.
* Grammatical errors have been fixed in the obituaries displayed in the console if the `playername` CVAR was changed from its default.
* The `vid_capfps` CVAR will now be saved correctly in `doomretro.cfg`.
* Entering cheats will no longer interfere with moving the player in [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* The bob of the player’s weapon will no longer become stuck when the `centerweapon` CVAR is `off` in some instances.
* The bob of the chainsaw is now as smooth as the other weapons when the player moves.
* `BIGDOOR7`, `FIRBLU1` and `SKY1` textures are now displayed correctly.
* The `vid_motionblur` CVAR now accepts a value between `0%` and `100%`, rather than just `on` or `off`. It is `0%` by default.
* An `r_skycolor` CVAR has been implemented that allows the player to override the current map’s sky texture and use a solid color for the sky instead. It is `none` by default, and also accepts a value between `0` and `255`.
* A bug present in *Vanilla DOOM* has been fixed whereby if the player stood between two damaging sectors at different heights, they wouldn’t be damaged.
* The `wad` CVAR is now reset whenever `reset iwadfolder` or `resetall` are used in the console.
* There is no longer any delay with the player’s face updating in the status bar upon loading a savegame or progressing to the next map.
* *Vanilla DOOM’s* [“status bar face hysterisis”](https://doomwiki.org/wiki/Status_bar_face_hysteresis) bug is now fixed.
* Blood splats will now be immediately removed from the current map if either the `r_blood` CVAR is changed to `none`, or the `r_bloodsplats_max` CVAR is changed to `0`.
* A `vid_pillarboxes` CVAR has been implemented that toggles using the pillarboxes on either side of the screen for palette effects. It is `off` by default.
* A bug has been fixed whereby the individual monster stats in the `playerstats` CCMD would be recalculated incorrectly when an arch-vile resurrected another monster.
* Fixes have been applied to three locked doors in E2M2 and E2M6 of `doom.wad` so that monsters can’t open them from the other side.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 13, 2017

### DOOM Retro v2.3.9

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby multiple files couldn’t be loaded using *DOOM Retro’s* WAD launcher.
* Improvements have been made to translucent wall textures when the `r_dither` CVAR is `on`.
* Savegames no longer become corrupted in some instances. Consequently, savegames created using previous versions of *DOOM Retro* are not compatible with this version.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 6, 2017

### DOOM Retro v2.3.8

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*, particularly in the handling of large numbers of blood splats.
* *DOOM Retro’s* splash screen has been redesigned.
* Minor changes have been made to text that is output to the console.
* The CVARs in `doomretro.cfg` will no longer be reset if *DOOM Retro* quits with an error during startup in some rare instances.
* A bug has been fixed whereby in some rare instances objects would get suspended in midair when the sector they were on lowered.
* The last WAD to be opened now prepopulates the `File name` field in *DOOM Retro’s* WAD launcher.
* The `r_shakescreen` CVAR has been renamed to `r_shake_damage` and is now `50%` by default.
* The screen will now shake when the player is near enough to an exploding barrel. This feature may be disabled using the new `r_shake_barrels` CVAR.
* The following changes have been made to the `vid_showfps` CVAR:
  * When both the `vid_showfps` and `vid_vsync` CVARs are `on`, the frames per second displayed will be red if less than the display’s refresh rate.
  * If the frames per second drops too low, a warning will now be displayed in the console.
  * If the CVAR is disabled in the console, the minimum and maximum FPS since the CVAR was enabled will now be displayed.
* An `exec` CCMD has been implemented that allows a series of commands stored in a file to be executed at once as if they had been typed in the console individually.
* Textures that have patches with negative offsets (such as `TEKWALL1` and `STEP2`) now appear correctly.
* A bug has been fixed whereby monsters would continue to fire at the player’s corpse after killing them in some instances.
* When the `r_dither` CVAR is `on`, multiple translucent wall textures are now visible through each other.
* Further improvements have been made to the support of `DEHACKED` lumps.
* Aliases can now be created using the new `alias` CCMD. These aliases can be entered into the console to execute a string of commands, and are saved in `doomretro.cfg`.
* Small amounts of damage to the player are now more evident.
* The total amount of ammo, armor and health picked up in the current map that is displayed by the `playerstats` CCMD is now correctly reset when the map changes.
* The `map` CCMD now has a `random` parameter that will warp the player to a random map.
* All sky textures with a height other than 128 pixels will now be ignored if specified in a `MAPINFO` lump.
* A `freeze` CCMD has been implemented that toggles freezing of gameplay while still allowing the player to move around.
* A bug has been fixed whereby the <kbd><b>,</b></kbd> key couldn’t be bound nor unbound in the console.
* The bound controls displayed by the `bindlist` CCMD are now enumerated correctly.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, December 2, 2016

### DOOM Retro v2.3.7

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* A message is now displayed when using the `IDCLEVxy` cheat in the console.
* The average frames per second will no longer be shown when the `vid_showfps` CVAR is `on` and the game is paused or the menu is open.
* The lumps in `doomretro.wad` that are used for the alternate HUD can now be replaced by lumps in a PWAD.
* When the `r_shadows` CVAR is `on` and the `vid_capfps` CVAR is a value other than `35`, the shadows of monsters are now interpolated along with the movement of the monsters themselves.
* The *BFG Edition* of `doom2.wad` will now be identified as an IWAD in the console, even though it has been incorrectly marked as a PWAD.
* The number of barrels exploded is now displayed when using the `playerstats` CCMD.
* The number of monsters, pickups and decorations, as well as the number of liquid and damaging sectors, are now displayed when using the `mapstats` CCMD.
* The choice of colors used in the numerous translucent effects in *DOOM Retro* has been improved.
* *BOOM*-compatible translucent wall textures are now drawn using a dithering effect. This can be disabled using the new `r_dither` CVAR.
* Instead of being set to the currently selected item in the corresponding menu, the `episode`, `expansion`, `savegame` and `skilllevel` CVARs are now set to read-only strings of the episode, expansion, savegame and skill level for the current game.
* The player’s path in the automap, enabled using the `am_path` CVAR, will no longer be recorded while the player is in “no clipping mode”.
* Corpses will no longer perpetually shift back and forth over sector boundaries with small height differences.
* The effect of changing the `r_brightmaps` CVAR from `off` to `on` is now immediate, and doesn’t require *DOOM Retro* to be restarted.
* A bug has been fixed whereby the brightmap of the `SW1STON2` and `SW2STON2` switch textures wouldn’t be applied correctly in *DOOM* but would in *DOOM II*.
* The following changes have been made to the `spawn` CCMD:
  * A crash will no longer occur when trying to spawn certain decorations and pickups that don’t exist in *DOOM* but do in *DOOM II*.
  * Hanging decorations are now spawned on the ceiling.
  * Things will now be in the same state they would be if they were spawned when the map started.
* The “floating skull rock” decoration now casts a shadow when the `r_shadow` CVAR is `on`.
* A crash will no longer occur when using the `give` CCMD to try to give the player the plasma rifle, BFG-9000 or cells in *DOOM Shareware*, or the super shotgun in *DOOM*.
* A bug has been fixed whereby when the `s_randommusic` CVAR was `on`, random music would attempt to start playing at the start of a map but then stop, and the game would become almost completely unresponsive.
* When the `s_randommusic` CVAR is `on`, the random music chosen at the start of a map will now loop rather than different music starting to play after the first finishes.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, November 19, 2016

### DOOM Retro v2.3.6

* *DOOM Retro* is now compiled using [*Microsoft Visual Studio Community 2017 RC*](https://www.visualstudio.com/vs/visual-studio-2017-rc/).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Changes have been made to the format of savegames and so are not compatible with previous versions of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* A bug has been fixed whereby if the framerate was capped to a value less the display’s refresh rate by using the `vid_capfps` CVAR, it couldn’t be uncapped while the `vid_vsync` CVAR was `on` or without restarting *DOOM Retro*.
* The effect enabled using the `r_shakescreen` CVAR requires hardware acceleration. If the `vid_scaleapi` CVAR is `"software"`, the screen will no longer momentarily freeze when the player is attacked and the `r_shakescreen` is not `0%`.
* A warning will now be displayed in the console, and *DOOM Retro* will default to nearest-neighbor interpolation, if the `vid_scaleapi` CVAR is `"software"` and the `vid_scalefilter` CVAR is anything other than `"nearest"`.
* The wrong map title is no longer displayed for MAP31 and MAP32 in some PWADs when using the *BFG Edition* of `doom2.wad`.
* A bug has been fixed whereby the `MAPINFO` lump in a PWAD could be parsed incorrectly and cause the wrong music to be played in a map.
* Music will now be paused if either the menu or console is open and the window loses focus.
* A crash will no longer occur when trying to spawn a spider mastermind using the `spawn` CCMD in *DOOM Shareware*.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, November 15, 2016

### DOOM Retro v2.3.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The console is automatically opened at startup when `-devparm` is specified on the command-line.
* If the `vid_scaleapi` CVAR is `"opengl"` and the version of the available *OpenGL* API is less than v2.1, then it will be changed to `"direct3d"` instead.
* If `-cdrom` is specified on the command-line and the `r_diskicon` CVAR is `on`, the `STCDROM` lump will be used instead of the `STDISK` lump.
* The size of the grid in the automap can now be changed using the `am_gridsize` CVAR. It is `128x128` by default.
* The last menu item to be selected is now remembered when using the <kbd><b>F2</b></kbd>, <kbd><b>F3</b></kbd> or <kbd><b>F4</b></kbd> keys to display a menu.
* Further improvements have been made to make sure objects are lit correctly in all instances.
* The music volume is now properly set at startup.
* The console now opens and closes at a consistent speed, slowing down as it is almost completely opened.
* The title of the currently playing music track, as well as the number of secret sectors, are now displayed in the output of the `mapstats` CCMD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, November 10, 2016

### DOOM Retro v2.3.4

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The console now opens and closes faster.
* The game will no longer crash when opening the console on the title screen and using the mouse wheel to scroll upwards.
* Instead of toggling capping of the framerate at 35 FPS, the value of the `vid_capfps` CVAR is now the actual frames per second at which the framerate will be capped. It can be `off`, or between `35` and `1,000` FPS, and is `200` FPS by default. All interpolation is automatically disabled when this CVAR is set to `35` FPS. This CVAR has no effect if it is set to a value greater than the display’s refresh rate and the `vid_vsync` CVAR is `on`.
* Vertical sync with the display’s refresh rate now works correctly when the `vid_vsync` CVAR is `on` and the `vid_scaleapi` CVAR is `"opengl"`.
* The `vid_scaleapi` CVAR is now `"opengl"` by default.
* The `vid_scalefilter` CVAR is now `"nearest_linear"` by default.
* The `vid_vsync` CVAR is now `on` by default.
* The following improvements have been made to [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/) support:
  * `hacx.wad` will now be identified correctly when it is loaded using the `-iwad` command-line parameter.
  * The health bar in the alternate HUD is now displayed correctly when the player’s health is greater than 200.
  * Using the <kbd><b>A</b></kbd> key to strafe left now works.
  * Windows are no longer shattered when using `kill all` in the console.
* The music will now be stopped if *DOOM Retro* crashes.
* A bug has been fixed whereby the music volume and sound effects volume were set incorrectly at startup in some instances.
* The text carets in both the save game menu and the console will no longer be displayed, and the skull in the menu will no longer animate, while the window doesn’t have focus.
* Objects will no longer be lit incorrectly in some rare instances.
* A bug has been fixed whereby certain secrets wouldn’t be counted in some *BOOM*-compatible maps.
* Sectors with multiple effects in some *BOOM*-compatible maps will now behave correctly.
* The weapon keys <kbd><b>1</b></kbd> to <kbd><b>7</b></kbd> will no longer momentarily fail to work after entering an invalid parameter for the `IDMUSxy` cheat.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, October 29, 2016

### DOOM Retro v2.3.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The limited MIDI support in *Windows Vista* and above has now been overcome, allowing the music volume to be adjusted independently of the sound effects volume. To allow this to happen, an additional file called `midiproc.exe` is now included and needs to remain in the same folder as `doomretro.exe`.
* The `s_musicvolume` CVAR is now `66%` by default.
* The `r_diskicon` CVAR is now `off` by default.
* Minor changes have been made to text that is output to the console.
* The console will now fill the entire screen when opened using the <kbd><b>~</b></kbd> key on the title screen.
* The scrollbar in the console is now hidden if all the text in the console fits entirely on the screen.
* The extreme edges of both the menu and console backgrounds have been softened slightly.
* *DOOM Retro’s* title and version in the console are now white.
* A bug has been fixed whereby using the `map` CCMD when no game was being played would cause a crash.
* The player will now be thrust away with the correct amount of force when attacked by an arch-vile, or within the blast radius of a rocket or barrel explosion.
* A time limit for each map can now be set using the `-timer` command-line parameter.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, October 22, 2016

### DOOM Retro v2.3.2

* *DOOM Retro* now uses version 2.0.5 of the [*SDL (Simple DirectMedia Layer)*](http://www.libsdl.org/) library.
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Both *DOOM Retro’s* icon and splash screen have been redesigned.
* A bug has been fixed whereby the initialization of *DirectInput* gamepads was not being displayed in the console at startup.
* The directional pad on *DirectInput* gamepads now works correctly and won’t interfere with the other buttons.
* Invalid characters are no longer displayed in the console or the resulting text file of the `condump` CCMD.
* “™”, “©” and “®” characters can now be displayed in the console.
* Minor changes have been made to text that is output to the console.
* The palette will no longer inadvertently change when exiting the menu in some rare instances.
* The `con_obituaries` CVAR, which enables obituaries in the console, is now `on` by default.
* Obituaries are now displayed correctly for monsters killed by a barrel exploding.
* An obituary is no longer displayed when using `kill player` in the console.
* A bug has been fixed whereby trying to change the `gp_deadzone_left` CVAR would in some instances change the `gp_deadzone_right` CVAR instead.
* All sprites, including the player’s weapon, will now be lit correctly when in a sector whose light level has been transfered from another sector. (An example of this is directly beyond the first door in MAP01 of [*Sunlust*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/sunlust).)
* The following improvements have been made to [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/) support:
  * The arms background in the status bar is now positioned correctly.
  * There are no longer any green or blue blood splats.
  * Obituaries are no longer displayed.
* The individual monster kill stats displayed using the `playerstats` CCMD are no longer incremented in either [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/) or [*Chex Quest*](https://doomwiki.org/wiki/Chex_Quest).
* Support has been added for [*Chex Quest 2: Flemoids Take Chextropolis*](https://doomwiki.org/wiki/Chex_Quest#Chex_Quest_2).
* A bug has been fixed whereby when the player tried opening a locked door that required all six keys, the message displayed would indicate that only three keys were required.
* The external automap enabled using the `am_external` CVAR will now be recreated successfully when changing another CVAR that causes the graphics subsystem to be restarted.
* The `vid_motionblur` CVAR now has a value of either `on` or `off` rather than a percentage, and is `off` by default. The motion blur effect it enables now better matches the player’s turning speed regardless of the control method used.
* The additional motion blur effect applied when the player is injured and the `r_shakescreen` CVAR is `on` will now only be applied when the `vid_motionblur` CVAR is also `on`.
* The `gp_vibrate` CVAR that toggles the vibration of *XInput* gamepads has now been replaced by two CVARs: `gp_vibrate_damage` for the amount of vibration when the player is damaged, and `gp_vibrate_weapons` for the amount of vibration when the player fires their weapon. Both accept a value between `0%` and `200%` and are `100%` by default.
* Gamepad buttons can now be bound to the `+back`, `+forward`, `+left`, `+right`, `+strafe`, `+strafeleft` and `+straferight` actions.
* Bound controls are now saved in the correct order in `doomretro.cfg`.
* “(BFG Edition)” is no longer added to the end of the window’s caption when playing a PWAD with the *DOOM II: Hell On Earth (BFG Edition)* IWAD.
* A bug has been fixed whereby the window’s position wouldn’t be correctly restored at startup.
* A texture has been corrected in MAP18 of `doom2.wad`.
* Follow mode can no longer be disabled while the `am_external` CVAR is `on`.
* When using an automap function while the `am_external` CVAR is `on`, its message is now shown on the external automap rather than the main display.
* Pressing a gamepad button bound to the `+clearmark`, `+followmode`, `+grid`, `+mark`, `+maxzoom` or `+rotatemode` actions now works as intended in the automap.
* The `vid_scaleapi` and `vid_widescreen` CVARs will now be reset correctly when using either the `reset` or `resetall` CCMDs.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, October 6, 2016

### DOOM Retro v2.3.1

* The correct value of the `ammo` CVAR is now displayed when the player has their fists or chainsaw selected.
* A bug has been fixed whereby lost souls wouldn’t be fullbright at certain angles.
* A previously implemented feature that caused monsters not to be fullbright when firing and facing away from the player now works correctly.
* Minor changes have been made to text that is output to the console.
* A confirmation message is now displayed when using the `resetall` CCMD.
* If an invalid map marker is encountered in a PWAD’s `MAPINFO` lump, a warning will now be displayed and *DOOM Retro* will continue to parse the lump rather than exiting with an error.
* The game will no longer crash when trying to spawn a cyberdemon using the `spawn` CCMD in *DOOM Shareware*.
* An error is now displayed in the console when a monster can’t be spawned using the `spawn` CCMD.
* A bug has been fixed whereby the player’s death sound wasn’t being played when they died.
* Blood splats are no longer spawned around corpse decorations that are in a liquid sector.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, September 30, 2016

### DOOM Retro v2.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Changes have been made to the format of savegames and so are not compatible with previous versions of *DOOM Retro*.
* Motion blur when the player turns quickly can now be enabled by setting the new `vid_motionblur` CVAR to a value greater than its default of `0%`.
* The `am_xhaircolor` CVAR has been renamed to `am_crosshaircolor`.
* The `vid_scaledriver` CVAR has been renamed to `vid_scaleapi`.
* The default of the `vid_scaleapi` CVAR is no longer `""`. When *DOOM Retro* is run for the first time, the best available API is chosen, changing this CVAR to `"direct3d"`, `"opengl"` or `"software"`.
* A bug has been fixed whereby some CVARs weren’t being reset to their correct values, or at all, when using either the `reset` or `resetall` CCMDs.
* Entering `reset all` in the console will now work the same as entering the `resetall` CCMD.
* Toggling “always run” using the <kbd><b>CAPSLOCK</b></kbd> key while in the console will no longer inadvertently affect player messages from appearing.
* Many minor changes have been made to text that is output to the console.
* A new `-nomapinfo` command-line parameter has been implemented that will stop any `MAPINFO` lumps from being parsed in PWADs at startup.
* If there is a `MAPINFO` lump present in `nerve.wad` that contains invalid map markers, the PWAD will no longer exit with an error, and a warning will be displayed in the console instead.
* The <kbd><b>SHIFT</b></kbd> key will now be ignored when pressing <kbd><b>Y</b></kbd> or <kbd><b>N</b></kbd> in response to a centered message.
* A bug has been fixed whereby no value would be displayed when entering the `r_hud` CVAR in the console without a value.
* When entering a CVAR in the console without a value, the CVAR’s description, current value and default value will now be displayed.
* The shadows of cyberdemons have been raised slightly.
* The values of CVARs in `doomretro.cfg` now have thousands delimiters.
* Thousands delimiters may now be used when entering values of CVARs in the console.
* Monster spawners are now disabled when using `kill all` in the console.
* All automap controls (pressing the <kbd><b>G</b></kbd> key to toggle the grid for instance) may now be used when there’s an external automap, provided they don’t conflict with any other controls.
* A bug has been fixed whereby certain items wouldn’t teleport in some rare instances. (An example of this is one of the yellow skull keys in MAP23 of [*Going Down*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/gd).)
* Lost souls spawned by pain elementals now move towards their target in their attack frame.
* The `playerstats` CCMD now displays 8 additional stats:
  * the number of maps completed,
  * the distance traveled (where 1 foot equals 16 map units),
  * the amount of ammo picked up (divided into bullets, cells, rockets and shells),
  * the amount of armor picked up, and
  * the amount of health picked up.
* The units used to display the new “Distance traveled” stat in the `playerstats` CCMD can be changed from `feet`/`miles` to `metres`/`kilometres` by changing the new `units` CVAR from its default of `imperial` to `metric`.
* The effects of changing the `r_translucency` CVAR will now be immediate in the HUD.
* When the `r_translucency` CVAR is `off`, the console and the alternate HUD will now no longer be translucent.
* The alternate HUD is now enabled by default.
* A texture has been corrected in MAP13 of `doom2.wad`.
* The player’s path may now be displayed in the automap by enabling the new `am_path` CVAR. It is `off` by default.
* The color of the player’s path may be changed using the new `am_pathcolor` CVAR. It is `95` (a light gray) by default.
* The console is now automatically closed when the `spawn` CCMD is used.
* Spaces are now allowed in the `playername` CVAR.
* The `playername` CVAR is now changed back to its default of `"you"` if it is changed to an empty string.
* The values of the `r_detail` CVAR are now displayed correctly in the output of the `cvarlist` CCMD.
* The `+use` and `+fire` actions will now respawn a dead player when in the automap.
* A bug has been fixed that stopped some string CVARs from being able to be changed in the console.
* The digits in the status bar are no longer lowered by 1 pixel in [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960).
* The “Cheated” stat in the `playerstats` CCMD now increases when using some CCMDs and command-line parameters that would be considered cheating.
* The console is now automatically closed when the `ammo`, `armor` and `health` CVARs are changed.
* If the `health` CVAR is changed to a smaller value, the effects of the damage to the player will now be shown.
* If the `ammo`, `armor` and `health` CVARs are changed to a larger value, the screen will now flash.
* The player will now be resurrected if the `health` CVAR is changed in the console when they are dead.
* There is now a read-only `version` CVAR that may be used to determine which version of *DOOM Retro* created a `doomretro.cfg` file.
* The super shotgun will now be displayed correctly when fired in [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens).
* The default gamepad sensitivity (set using the `gp_senstivity` CVAR) has been increased from `48` to `64`.
* The `+forward2`, `+back2`, `+strafeleft2`, `+straferight2` and `+use2` actions have been removed. The controls that were bound to these actions are now bound to `+forward`, `+back`, `+strafeleft`, `+straferight` and `+use`, respectively.
* The right thumbstick on gamepads is now bound to the `+use` action and may be pressed as an alternative to the <kbd><b>A</b></kbd> button to open doors, use switches, etc.
* A bug has been fixed whereby certain player stats were being reset to `0` at startup.
* The effects of the `IDDT` cheat are now removed from the automap when the player changes levels.
* The shaking of the screen when the player is injured and the `r_shakescreen` CVAR is `on` has been improved slightly.
* A bug has been fixed whereby firing the chaingun would increase the “Shots Fired” stat by 1, but would increase the “Shots Hit” stat by 2 if the shot successfully hit a monster.
* If the player has the invulnerability power-up when using `kill player` in the console, the inverted screen effect will now be removed.
* The map title in the automap is now positioned better when using a taller character set from a PWAD (such as [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens)).
* The folder where savegames are saved and loaded can now be specified using the `-savedir` command-line parameter.
* The suicide bombers in [*Valiant*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/valiant) will now explode as intended.
* If a `TITLEPIC` lump exists in a PWAD, and there is no `CREDIT` lump to accompany it, then the `CREDIT` lump in the IWAD won’t be displayed during the title sequence.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, July 30, 2016

### DOOM Retro v2.2.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The shadows of mancubi have been raised slightly.
* The `resetall` CCMD will no longer cause a crash when used while not in a game.
* A warning will now be displayed in the console if gamepad initialization fails.
* A bug has been fixed whereby the `vid_scalefilter` CVAR would default to `"nearest_linear"` rather than `"nearest"` if invalid.
* If the initialization of music fails at startup, a warning will be displayed in the console and startup will continue with music disabled, rather than *DOOM Retro* quitting with an error.
* The `playername` CVAR can now be set to `""` in the console.
* The armor bar in the alternate HUD now aligns exactly with the health bar below it.
* The `-respawn` command-line parameter will now work correctly.
* Minor changes have been made to some of the output in the console.
* The correct `INTERPIC` lump will now be displayed if replaced in a PWAD and using *DOOM II: Hell On Earth (BFG Edition)*.
* When using `kill player` in the console, the player will no longer turn to face their last attacker.
* A `respawnitems` CCMD has been implemented that allows most items to be respawned 30 seconds after the player picks them up, replicating what happens in *DOOM* multiplayer.
* The contents of `doomretro.cfg` have been rearranged slightly and commented.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, July 16, 2016

### DOOM Retro v2.2.4

* Blood splats and shadows are now drawn at greater distances.
* Minor changes have been made to some elements of the console.
* A bug has been fixed whereby the screen’s colors may appear wrong in some rare instances.
* If in low detail mode, and the `r_lowpixelsize` has been changed from its default, the view border will no longer be affected at smaller screen sizes.
* A `reset` CCMD has been implemented which will reset a CVAR to its default value.
* A `resetall` CCMD has been implemented which will reset all CVARs to their default values.
* A `bindlist` CCMD has been implemented which will list all the bound controls. Previously, entering the `bind` CCMD without any parameters would do this.
* The individual monster kill stats displayed using the `playerstats` CCMD will no longer sometimes become corrupted when an arch-vile resurrects a monster.
* If *DOOM Retro* fails to launch for some reason, a more descriptive error will now be displayed.
* A bug has been fixed whereby changing the `vid_scalefilter` CVAR to `nearest_linear` in the console could fail in some instances.
* The floor texture of sector 103 in MAP04 of `plutonia.wad` has been fixed.
* A bug has been fixed whereby rocket launcher frames would be shown when firing the photon ’zooka in [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* The text caret’s animation now resets each time the console is open, and is hidden when the console closes.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, July 5, 2016

### DOOM Retro v2.2.3

* *DOOM Retro* is now completely portable. The configuration file, `doomretro.cfg`, is now saved in the same folder as the executable, savegames are saved in a `savegames\` folder and screenshots are saved in a `screenshots\` folder.
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to some of the text in the console.
* The width of any selected text is now accounted for when inputting text in the console.
* The number of available displays are now rechecked before creating an external automap if the `am_external` CVAR is enabled through the console.
* An `RMAPINFO` lump will now be used if present in preference to a `MAPINFO` lump to avoid conflicts with other *DOOM* source ports.
* The keyboard shortcuts <kbd><b>SHIFT</b></kbd> + <kbd><b>HOME</b></kbd> and <kbd><b>SHIFT</b></kbd> + <kbd><b>END</b></kbd> are now allowed in the console to select all text to the left and right of the caret.
* The `r_berserkintensity` CVAR now accepts a value between `0` and `8` inclusive instead of a percentage. It has a default of `2`.
* The `expansion` CVAR is no longer changed if `nerve.wad` is automatically loaded.
* The player’s view will no longer jump slightly when dead and their corpse is sliding down stairs.
* A `teleport` CCMD has been implemented that allows the player to be teleported to another location in the current map.
* Fuzzy shadows are now applied to any thing whose `SHADOW` bit has been set in a `DEHACKED` lump.
* The map number in the console and automap is now shown in the format `E2Mxy` in [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960).
* The `r_bloodsplats_total` CVAR is now calculated correctly once it reaches `r_bloodsplats_max`.
* A bug has been fixed whereby palette effects from power-ups would remain on the screen after ending a game from the options menu in some instances.
* The value of `r_lowpixelsize` will no longer affect the display of the title screen when the menu is open.
* The <kbd><b>F5</b></kbd> key can no longer be used to change the graphic detail when the automap is open.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, June 18, 2016

### DOOM Retro v2.2.2

* One IWAD or PWAD may now be specified on the command-line without the need for the `-iwad` or `-file` command-line parameters.
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The `am_followmode` CVAR no longer appears in `doomretro.cfg`.
* A bug has been fixed whereby the case of map titles in the console would be displayed incorrectly in some instances.
* The `movebob` CVAR now only specifies the amount the player’s view bobs up and down when they are moving. The amount the player’s weapon bobs up and down is now specified using a new CVAR called `weaponbob`. Both CVARs are `75%` by default.
* The `savegame` CVAR, that specifies the savegame currently selected in the menu, can now be accessed from the console.
* The `map` CCMD can now be used with the `first` and `last` parameters when a game hasn’t started.
* Several new monster name variations can now be used with the `spawn` CCMD.
* A `play` CCMD has been implemented that allows any music or sound lump to be played.
* A bug has been fixed whereby marks couldn’t be added to the automap when rotate mode was off.
* The filename displayed when taking a screenshot while `am_external` is `on` is now fixed.
* The “picked up” player messages are no longer displayed when using the `give` CCMD.
* The position of flashing keycards and skull keys in the alternate HUD when the player tries to open a locked door has been fixed.
* Taking screenshots can now be bound to a key other than <kbd><b>PRINTSCREEN</b></kbd> using the `bind` CCMD with the new `+screenshot` action.
* Parameters can no longer be entered at the end of CCMDs that don’t use them.
* The player’s view will no longer jump slightly when dropping down between two liquid sectors greater than 24 units apart.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, June 9, 2016

### DOOM Retro v2.2.1

* *DOOM Retro* is now back to supporting *Windows XP* again.
* A crash will no longer occur when pressing the <kbd><b>PRINTSCREEN</b></kbd> key to take a screenshot on a display with an aspect ratio less than 4:3 (such as 1280×1024).
* A missing texture has been added to linedef 445 in E3M9 in `doom.wad`.
* Messages are now paused while the console is open.
* A bug has been fixed whereby IWADs weren’t being identified correctly.
* The player’s view is now only lowered if they are actually touching a liquid sector.
* Bobbing liquid sectors will now animate correctly if adjacent to a masked midtexture.
* The `centerweapon` CVAR can now also be entered as `centreweapon`.
* The `centered` value for the `vid_windowpos` CVAR can now also be entered as `centred`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, June 4, 2016

### DOOM Retro v2.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro’s* settings and savegames are now placed in `C:\<username>\DOOM Retro\`.
* “.wad” is now removed from savegame folder paths.
* Entering `r_gamma off` in the console now works correctly.
* Player messages and the automap title are now less translucent.
* Monsters will no longer be alerted when the player punches thin air. They will still be alerted if the player punches a wall, however.
* A bug has been fixed whereby obituaries in the console would be incorrect in some instances.
* Support has been added for `MUSINFO` lumps.
* Support has also been added for music changer map objects.
* The translucency of health bonuses has been reduced from 33% to 25%.
* A slight change has been made to the background noise effect in the console.
* Screenshots are now saved as PNG files rather than *Windows* BMP files.
* The full path of the file is now displayed in the console when a screenshot is taken.
* The player’s view will no longer briefly change before the screen wipes when exiting a map.
* The view border is now displayed correctly for PWADs such as [*Valiant*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/valiant) and [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens).
* The positions of the shadows of some monsters have been improved.
* The flats `SLIME09` to `SLIME12` no longer animate as liquid in [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens).
* The `help` CCMD will now open the “Console” chapter of the [*DOOM Retro Wiki*](http://wiki.doomretro.com) in the default browser.
* There are now `ammo`, `armor` and `health` CVARs that allow changing the player’s ammo, armor and health to specific values.
* The texture offset of linedef 638 in MAP10 of `doom2.wad` has been corrected.
* The compiler used to build *DOOM Retro*, and its version, are now displayed in the console at startup.
* A bug has been fixed whereby pressing a mouse or gamepad button at startup in the brief moment before the splash screen appears would cause the screen to stay black.
* Several visual aspects of the console have been redesigned.
* The markers are now smaller in the health and armor bars in the alternate HUD.
* If the player has more than 100% health, a second marker will appear above the health bar in the alternate HUD.
* The armor bar in the alternate HUD is now slightly thinner, and divided into five rather than four sections.
* The larger digits used in the alternate HUD are now consistent with the smaller ones.
* The screen’s green haze when the player has the radiation suit power-up will now be visible if the player also has the berserk power-up, their fists selected, but the `r_berserkintensity` CVAR set to `0%`.
* The default gamepad sensitivity has been increased from 32 to 48.
* Corpses are no longer teleported if the `kill` CCMD is used.
* Player messages are now always yellow in the console.
* Color CVARs are now set to their defaults, rather than the closest valid value, if invalid at startup.
* C++ style comments are now allowed in `MAPINFO` lumps.
* The correct colors are now preserved in the automap, the console and the alternate HUD if a PWAD contains a custom `PLAYPAL` lump. (An example of such a PWAD is skillsaw’s recently released MegaWAD, [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens).)
* A bug has been fixed whereby parts of the super shotgun would be translucent in [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens).
* The `r_corpses_color` CVAR is now validated at startup.
* If `am_external` is on but there’s only one display found, there will no longer be a crash if the graphics system is restarted.
* The number of logical cores and amount of system RAM is now displayed in the console at startup.
* [*ZDOOM’s*](http://zdoom.org/) obituary strings are now ignored in `DEHACKED` patches so warnings aren’t displayed in the console at startup.
* A bug has been fixed whereby a frame would be skipped when rotating monsters in the *DOOM II* cast sequence.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, May 3, 2016

### DOOM Retro v2.1.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Scrolling up in the options menu now works correctly.
* The `playername` CVAR is reset to its default if it’s empty at startup.
* The fixes that *DOOM Retro* applies to maps in the official *DOOM* and *DOOM II* IWADs are now listed in the console when a map is loaded and `-devparm` is specified on the command-line.
* The default mouse sensitivity has been increased from 24 to 32.
* The health cross in the alternate HUD is now squarer.
* An armor count is now displayed in the alternate HUD.
* A bug has been fixed whereby keycards and skull keys weren’t positioned correctly in the status bar in some instances.
* The brightmap for the `SW2BRNGN` wall texture is now fixed.
* Blood splats are now always spawned for monsters that bleed when using the `kill` CCMD.
* Monsters will now be alerted when the player punches a wall or the air.
* The full map title of John Romero’s new map, [*E1M4B: Phobos Mission Control*](https://twitter.com/romero/status/725032002244759552) is now displayed in both the console and automap.
* The player’s view will now bob up and down if they die on a liquid sector.
* Improvements have been made to the accuracy of “Weapon accuracy” in the output of the `playerstats` CCMD.
* The corpses of monsters are no longer spawned if “No monsters” has been set.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, April 24, 2016

### DOOM Retro v2.1.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Changes have been made to the format of savegames that breaks compatibility with previous versions of *DOOM Retro*.
* Any momentum from the player is now stopped when exiting a map so updating the exit switch is smoother.
* The `r_shakescreen` CVAR is now a value between `0%` and `100%`, instead of just `on` or `off`. Its default is `100%`.
* A bug has been fixed whereby some screen resolutions weren’t displaying correctly in the console at startup.
* *DOOM Retro’s* settings and savegames are now placed in `C:\<username>\AppData\Local\DOOM Retro\`. Remember, to change settings, open the console using the <kbd><b>~</b></kbd> key when *DOOM Retro* is running.
* Monster counts in the `playerstats` CCMD are no longer increased if “No monsters” has been set.
* Hanging decorations will no longer drop to the ground when over a liquid sector that moves.
* The direction items are dropped when a monster is killed is now better randomized.
* A bug has been fixed whereby pressing the <kbd><b>ENTER</b></kbd> key to close the help screen would cause the screen’s aspect ratio to be set incorrectly.
* The effects of changing the `r_translucency` CVAR are now instantaneous.
* A stray black pixel has been removed from under “N” characters in the menu.
* The <kbd><b>WINDOWS</b></kbd> key can no longer be pressed when fullscreen, as intended.
* The *Windows* screensaver is now disabled while *DOOM Retro* is running.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, April 9, 2016

### DOOM Retro v2.1.1

* Pain elementals can now shoot lost souls through two-sided walls that have the `ML_BLOCKMONSTERS` flag, as is possible in *Vanilla DOOM*. (An example of this is at the end of MAP04 in [`requiem.wad`](https://www.doomworld.com/idgames/levels/doom2/megawads/requiem).)
* The screen will no longer briefly flash if the player has a berserk or radiation shielding suit power-up and then loads a savegame or starts a new game from the menu.
* The time taken to complete a map is now restored correctly when loading a savegame.
* A bug has been fixed whereby a map would become corrupted if the player triggered a generalized line with no tag (such as when the player takes the “plunge” in MAP08 of [`jenesis.wad`](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/jenesis)).
* A crash will no longer occur when using the `maplist` CCMD.
* Multiple `STBAR` lumps are now better handled. (For example, now the correct status bar will be displayed if `JPCP_HUDjpn.wad` is loaded along with [`JPCP_1st.wad`](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/jenesis).)
* Double-resolution yellow and gray digits will no longer be displayed in the status bar if a `STBAR` lump from a PWAD is used.
* The correct WAD is displayed in the output of the `mapstats` in *DOOM II: Hell On Earth* if `nerve.wad` is also present.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, April 2, 2016

### DOOM Retro v2.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Support has been added for the `SWITCHES` lump.
* Fast monsters can now be enabled by using the `fastmonsters` CCMD. Also, `-fastmonsters` as well as just `-fast` can be specified on the command-line.
* Warnings displayed when parsing *DeHackEd* files and lumps are now only displayed if `-devparm` is specified on the command-line.
* Automatically loading the last savegame when the player dies can now be disabled by using the `autoload` CVAR.
* The intensity of the red haze effect when the player has the berserk power-up and their fists selected can now be changed using the `r_berserkintensity` CVAR. The default is `33%`.
* Player sprites may now be translucent if they are replaced in a PWAD by using `Translucent = 1` for the relevant frame in *DeHackEd* files and lumps.
* Thing flags specific to *DOOM Retro* can now be changed using `Retro Bits` instead of `Bits2` in *DeHackEd* files and lumps.
* A bug has been fixed whereby the use of unknown music names in `MAPINFO` lumps would cause a crash.
* The `s_musicvolume` and `s_sfxvolume` CVARs are now corrected at startup if invalid.
* The `faceback` CVAR has been renamed to `facebackcolor`.
* The extraneous brown pixel in the super shotgun is no longer removed if the weapon is changed in a *DeHackEd* file or lump.
* Further improvements have been made to the appearance of the rocket launcher’s muzzle flash.
* DoomEd numbers are now allowed as the parameter for the `give`, `kill` and `spawn` CCMDs.
* The translucency of the super shotgun’s muzzle flash is now disabled when the `r_translucency` CVAR is off.
* A bug has been fixed whereby monsters would not respawn correctly when playing using the *Nightmare!* skill level.
* The shadow of the player’s corpse is now removed when resurrecting using either the `resurrect` CCMD or the `IDDQD` cheat.
* The console is now hidden when using the `IDDQD` cheat to resurrect the player.
* The screen now goes to black sooner when starting *DOOM Retro*.
* *DOOM II’s* cast sequence now works correctly when using [`smoothed.wad`](https://www.doomworld.com/forum/topic/85991).
* The console can no longer be opened during a screen wipe.
* The bottoms of things that bob in liquid now animate.
* A bug has been fixed whereby no monsters had lower pitches when the `s_randompitch` CVAR was enabled.
* The error message displayed when `doomretro.wad` is invalid has changed.
* `bfg` is now allowed as a parameter for the `spawn` and `give` CCMDs.
* A bug has been fixed whereby the player was unable to switch back to the selfie stick once obtained in [*InstaDoom*](https://www.doomworld.com/idgames/combos/instadm).
* The last savegame will no longer be automatically loaded when the player dies of the `pistolstart` CVAR is enabled.
* Individual monster kill counts used by the `playerstats` CCMD are now retained in savegames. Please note that this breaks savegame compatibility with previous versions of *DOOM Retro*.
* The speed of the player can now be changed using the `turbo` CVAR. Functioning like the command-line parameter of the same name, it can be a value between `10%` and `400%` and has a default of `100%`.
* A bug has been fixed (that is present in *Vanilla DOOM*) whereby the player would go in reverse when running if their speed was set to a value greater than `255%` using the `-turbo` command-line parameter.
* Monsters can now respawn when playing a skill level other than *Nightmare!* by using the `respawnmonsters` CCMD. The `-respawn` command-line parameter has also been reimplemented, and `-respawnmonsters` may also be used.
* The value set by `Max Health` in *DeHackEd* files and lumps is now only applied to health bonuses.
* 100 extra things (numbered 152 to 251) have been added for use in *DeHackEd* files and lumps.
* Things with the same (x,y,z) coordinates now bob in sync with each other if they are in liquid.
* The arachnorbs in [*Valiant*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/valiant) are now killed when using the `kill` CCMD.
* Map names changed using a `MAPINFO` lump are now shown in the output of the `maplist` CCMD.
* The <kbd><b>WINDOWS</b></kbd> key can no longer be used at any time when fullscreen. It can only be used when in a window, and the game is paused, or the menu or console is open.
* Wall textures that are both animated and translucent can now be rendered correctly without causing a crash.
* The <kbd><b>E</b></kbd> key may now be pressed as an alternative to <kbd><b>SPACE</b></kbd> to use doors, switches, etc. It is bound to the `+use2` action.
* When the `vid_showfps` CVAR is enabled, the frames per second is now displayed correctly while the screen shakes when the player is injured.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, February 18, 2016

### DOOM Retro v2.0.5

* Bugs have been fixed whereby using `map next` in the console would warp the player to the next episode rather than the next map, and `map ExMy` wouldn’t warp at all.
* 100 additional sprites, named `SP00` to `SP99` and numbered 145 to 244, have been added for use in *DeHackEd* lumps.
* The amount of negative health a monster must receive to be gibbed can now be changed using a `Gib health` parameter in *DeHackEd* lumps.
* An invalid character will no longer be displayed in the console when changing the music or SFX volume in the menu.
* A bug has been fixed whereby when adjusting the SFX volume in the menu, the music volume was being displayed in the console instead.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, February 10, 2016

### DOOM Retro v2.0.4

* Using an `A_FireOldBFG` code pointer in a *DeHackEd* lump will no longer cause the game to freeze.
* The following improvements have been made to [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/) support:
  * The correct status bar is now displayed.
  * The projectiles of the nuker are no longer translucent.
  * The smoke trails have been removed from the projectiles of the photon ’zooka.
* The “B” in John Romero’s [*E1M8B: Tech Gone Bad*](https://twitter.com/romero/status/688054778790834176) is now displayed when the map starts, and in the automap.
* Dead players can now trigger actions that allow them to exit a map.
* The total number of monsters, and the percentage killed, are now displayed for each type of monster in the output of the `playerstats` CCMD.
* The position of keys when using a custom status bar has been corrected.
* CCMDs and CVARs now appear in the correct order when pressing the <kbd><b>TAB</b></kbd> key in the console to autocomplete.
* The brightmap for the `SW2METAL` wall texture has been fixed.
* `SLIMExx` flats will no longer animate as liquid in `epic2.wad`.
* A small icon is now shown next to each warning in the console.
* The `STARTUP5` string is now displayed correctly in the console when playing [*Freedoom*](https://freedoom.github.io/).
* The `SDL2_mixer.dll` file supplied with *DOOM Retro* is now compiled with [*libmad 0.15.1b*](http://www.underbit.com/products/mad/), fixing the tempo of some MP3 lumps. Consequently, `smpeg2.dll` is no longer required.
* A bug has been fixed whereby using the `map` CCMD would cause the game to crash in some instances.
* The selected episode or expansion in the menu is set as necessary when using the `map` CCMD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, January 30, 2016

### DOOM Retro v2.0.3

* “Pistol start” gameplay is now supported. By using the `pistolstart` CCMD, (or specifying `-pistolstart` on the command-line), the player’s health, armor, weapons and ammo will be reset at the start of each map. Also, a `PISTOLSTART` definition may now be used in `MAPINFO` lumps.
* The muzzle flash of the player’s rocket launcher has been fixed.
* The `+menu` action can now be bound to a key, with `escape` being the default.
* The `+console` action can now be bound to a key, with `tilde` being the default.
* The amount of bobbing has been reduced for higher values of the `stillbob` CVAR.
* A bug has been fixed whereby successive movement keys would not register if a cheat existed that started with a movement key (as is the case in [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/)).
* The armor bar in the alternate HUD is now slightly lighter.
* A bug has been fixed whereby map names from `MAPINFO` lumps weren’t being displayed.
* The map title and author of John Romero’s recently released [*E1M8B: Tech Gone Bad*](https://twitter.com/romero/status/688054778790834176) is now displayed when the map starts, and in the automap.
* The `GOTREDSKULL` string may now also be spelled as `GOTREDSKUL` in *DeHackEd* lumps.
* Pressing <kbd><b>ALT</b></kbd> + <kbd><b>F4</b></kbd> to quit *DOOM Retro* now works again.
* Stylized quotes are now used in place of double quotes in the console.
* Text in the console is now slightly translucent.
* A random static effect has been applied to the console’s background.
* The effects of changing the `vid_windowpos` and `vid_windowsize` CVARs while in the console and in a window are now immediate.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, January 17, 2016

### DOOM Retro v2.0.2

* A rare bug has been fixed whereby the player’s view would continuously move or turn in one direction by itself.
* The `+run` action now works correctly when bound to a mouse button.
* The sound of a door closing is no longer played if the player walks over a line to trigger the door, and the door is already closed.
* It is now possible to warp to a map using `first`, `prev`/`previous`, `next` and `last` as the parameter for the `map` CCMD.
* A bug has been fixed whereby the muzzle flash of some weapons could be offset from the muzzle in some rare instances.
* The file `smpeg2.dll` is now included with *DOOM Retro* again.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, January 10, 2016

### DOOM Retro v2.0.1

* A bug has been fixed whereby the screen wouldn’t stop shaking after the player was killed in some instances.
* The `+run` action may now be bound to a mouse button.
* The player’s weapon will no longer be fullbright while the player is injured.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, January 9, 2016

### DOOM Retro v2.0

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* now uses the official releases of [*SDL v2.0.4*](http://libsdl.org) and [*SDL_mixer v2.0.1*](http://libsdl.org/SDL_mixer).
* Alun “Viggles” Bestor’s Cacoward 2015 winner `breach.wad` is now bundled with *DOOM Retro*!
* The mouse wheel controls to cycle through the player’s weapons has been inverted.
* Support has now been added for `TRANMAP` lumps.
* The warning stating a “music lump can’t be played” will no longer be displayed in the console when `-nomusic` or `-nosound` are specified on the command-line.
* The keys the player has already picked up are no longer reordered when using the `IDKFA` cheat.
* Blood splats are no longer spawned on the ground when corpses are nudged by the player or a monster moving over them.
* A `s_randommusic` CVAR has been implemented. When it is enabled, music will be chosen randomly rather than using the default music for each map.
* The following changes have been made to the `playerstats` CCMD:
  * The percentage of map revealed is now slightly more accurate.
  * The amount of shots fired, the shots hit, and the weapon accuracy are now displayed.
  * The number of player deaths for the current map, not just the total player deaths, is now displayed.
  * The number of monsters killed is now broken down by the type of monster.
  * The CCMD may now be entered in the console when not playing a game.
* A `stillbob` CVAR has been implemented. When it is changed from its default of `0%`, the player will bob when idle.
* A `faceback` CVAR has been implemented. It may be used to change the background color of the player’s face in the status bar. The default is `5` (a dark gray), and can be set to a [`PLAYPAL` color index](http://doomwiki.org/wiki/PLAYPAL) between `0` and `255` inclusive.
* Pressing <kbd><b>SHIFT</b></kbd> + <kbd><b>[</b></kbd>, <kbd><b>SHIFT</b></kbd> + <kbd><b>]</b></kbd> or <kbd><b>SHIFT</b></kbd> + <kbd><b>&#92;</b></kbd> in the console will now display the correct characters.
* The error displayed when `am_external` is `on` and an external automap can’t be created is now only displayed once at startup rather than each time the graphics subsystem is reset.
* The external automap is now blurred when the main display is.
* The `pm_walkbob` CVAR has been renamed to `movebob`.
* The `pm_alwaysrun` CVAR has been renamed to `alwaysrun`.
* The `pm_centerweapon` CVAR has been renamed to `centerweapon`.
* A bug has been fixed whereby the texture offsets for sectors that change from liquid to solid weren’t reset.
* An error is now displayed in the console if pressing the <kbd><b>PRINTSCREEN</b></kbd> key fails to take a screenshot.
* Pillarboxes are now cropped from screenshots.
* Rudimentary support has now been added for `MAPINFO` lumps. `MUSIC`, `NEXT`, `PAR`, `SECRETNEXT`, `SKY1` and `TITLEPATCH` keywords are supported. Additionally, the following keywords are supported specific to *DOOM Retro*:
  * `AUTHOR <author>`: Display the author’s name in the console when the map starts.
  * `LIQUID "<flat>"`: Specify a flat that will be treated as liquid in the map.
  * `NOLIQUID "<flat>"`: Specify an animated flat that won’t be treated as liquid in the map.
* *DOOM Retro* now has an alternate widescreen heads up display, inspired by the new *DOOM* released on May 13, 2016. It is disabled by default, and can be enabled using the `r_althud` CVAR. Widescreen mode, (displayed by pressing the <kbd><b>+</b></kbd> key to increase the screen size during a game, or through the options menu), needs to be enabled for it to be displayed.
* The player’s view no longer shifts at the start of a map when in windowed mode.
* The background is now redrawn whenever pressing the <kbd><b>ENTER</b></kbd> key in the console.
* Support is now included for MOD, XM, IT, S3M and FLAC music lumps.
* The pitch of barrel explosions is no longer randomized when the `s_randompitch` CVAR is on.
* Green marine corpses are now randomly colored. This feature may be disabled using the `r_corpses_color` CVAR.
* The message displayed when a gamepad is detected is now only displayed once in the console.
* A bug has been fixed that stopped a door from opening in MAP10 of `doom2.wad`.
* The vertical position of the large digits in the status bar has now been fixed when using an `STBAR` lump from a PWAD.
* The z-height of line attacks when in liquid sectors is no longer adjusted.
* Monsters now recognize when they are standing on *BOOM*-compatible lifts.
* Corpses are now nudged with more momentum when they are in liquid.
* Pain elementals will no longer appear to open and close their mouth for no reason. They will now still try to spawn Lost Souls that won’t fit in the map, but they will explode instantly.
* Whether lost souls spawned by pain elementals are above the ceiling or below the floor is now checked.
* The possibility of a key flashing when switching to the widescreen HUD has been fixed.
* Ceilings are now marked in the automap as no longer secret once they move.
* Flashing elements in the widescreen HUD are now paused correctly when the game is paused.
* Improvements have been made to the rendering of really long walls.
* A bug has been fixed whereby *BOOM* general crushers were not able to be triggered by walkover.
* Some animated flats in `btsx_e1.wad`, `eternall.wad`, `freedoom.wad` and `tvr!.wad` are no longer incorrectly displayed as liquid.
* The maximum number of mouse buttons supported has been increased from 8 to 16.
* The following CVARs have been implemented to allow the colors in the automap to be changed:
  * `am_allmapcdwallcolor`
  * `am_allmapfdwallcolor`
  * `am_allmapwallcolor`
  * `am_backcolor`
  * `am_cdwallcolor`
  * `am_fdwallcolor`
  * `am_gridcolor`
  * `am_markcolor`
  * `am_playercolor`
  * `am_teleportercolor`
  * `am_thingcolor`
  * `am_tswallcolor`
  * `am_wallcolor`
  * `am_xhaircolor`
* The space between words in the console has been condensed.
* A bug has been fixed whereby the scale filter wouldn’t change straight away if the `vid_scalefilter` CVAR was changed to an empty string (the default) in the console.
* The player’s weapon is no longer centered upon firing if its state’s `misc1` or `misc2` values are set in a *DeHackEd* patch. This fixes an issue with [*InstaDoom*](http://www.doomworld.com/idgames/combos/instadm).
* The `r_detail` CVAR can now be set correctly in the console.
* The maximum number of blood splats that can appear in a map can no longer be unlimited. The default of the `r_bloodsplats_max` CVAR is now `32768`.
* Some CVAR descriptions in the output of the `cvarlist` CCMD now span over 2 lines.
* The menu can no longer be opened while the console is closing.
* The gamma correction level is now calculated correctly.
* The parameter for the `map` CCMD when using [*Freedoom*](https://freedoom.github.io/) can now be of the format `CxMy`.
* The gradual light effect that is applied to opening and closing door sectors is no longer applied to sectors that have light specials.
* When entering the `IDKFA` cheat, if a keycard and a skull key of the same color are present in the map, only the keycard is given to the player.
* A bug has been fixed whereby updating the `gp_sensitivity` CVAR in the console was having no immediate effect.
* Changes have been made to how gamepad sensitivity is calculated.
* The misalignment of patches with negative horizontal offsets has been fixed. This issue was evident in some textures, such as `BIGDOOR7`, and some switches in `btsx_e1.wad`.
* If the same CVAR is changed more than once in succession, there will now only be one line of output in the console.
* The status bar’s background has been enhanced.
* Timestamps in the console are now updated to the newest message when player messages are combined.
* The small digits used in the status bar and for marks in the automap are now consistent with those used in the console.
* A bug has been fixed whereby if the `r_gamma` CVAR was set to `2.0`, the gamma correction level would still be set to the default of `0.75` at startup.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, October 21, 2015

### DOOM Retro v1.9

* *DOOM Retro* now allows the automap to be shown on a second display. This feature may be enabled using the `am_external` CVAR, and will display a fullscreen 16:10 automap in the first display it finds that is not the main display set by the `vid_display` CVAR. While this external automap is displayed, the <kbd><b>TAB</b></kbd> key is disabled, and the `IDDT` cheat can be entered at any time. Also, the automap’s usual controls are unavailable, but the grid and rotate mode may still be toggled in the console using the relevant CVARs.
* Optimizations have been made to further improve the overall performance of *DOOM Retro*.
* A new filter is now available to scale *DOOM Retro* onto the display. It is enabled by changing the value of the `vid_scalefilter` CVAR to `"nearest_linear"`, and is a combination of the existing two filters, `"nearest"` (which uses nearest-neighbor interpolation, the default) and `"linear"` (which uses linear filtering).
* The screen will no longer “bleed” along the edges when the `vid_scaledriver` CVAR is set to `""` or `"direct3d"` and the `vid_scalefilter` CVAR is set to `"linear"` on some systems.
* A bug has been fixed whereby screenshots couldn’t be taken by pressing the <kbd><b>PRINTSCREEN</b></kbd> key if characters that can’t be used in a filename were present in the current map’s title.
* A disk icon (the `STDISK` lump) is now displayed in the top right hand corner of the screen when saving and loading a game, and when loading a map. It may be disabled using the `r_diskicon` CVAR.
* A `nomonsters` CCMD has been implemented in the console which will toggle whether monsters are spawned or not in the next map. This CCMD will override when `-nomonsters` is specified on the command-line.
* The entire screen will now shake when the player is injured. This feature may be disabled using the `r_shakescreen` CVAR.
* Several improvements have been made to the support of *DeHackEd* patches and files.
* If a savegame fails to load, the menu will now close and the console will open displaying an error explaining why.
* A crash will no longer occur when trying to open a map that contains only one subsector.
* A bug has been fixed whereby some CVARs weren’t being validated at startup.
* Timestamped obituaries can now be displayed in the console each time something is killed, exploded or resurrected during a game. They are disabled by default, and may be enabled by using the `con_obituaries` CVAR.
* A bug has been fixed whereby a HOM could appear in maps that use texture 0 as a placeholder.
* Changing the `r_liquid_swirl` CVAR will now always update visible liquid sectors immediately.
* The darkest of the gradually transitioning lighting under doors is now slightly darker than all adjacent sectors to make the feature more prominent.
* Blood splats are no longer left behind if Lost Souls are close enough to the ground when the `kill` CCMD is used to kill them.
* `nerve.wad` will now be recognized correctly when specified using `-file` on the command-line.
* A bug has been fixed whereby entering a cheat in the console could stop player messages from appearing.
* The `iwadfolder` CVAR will now be set correctly when more than one PWAD is selected in the WAD launcher.
* Map titles that have been changed using a *DeHackEd* file or lump will be capitalized as necessary.
* “DOOM RETRO” is now displayed as “DOOM Retro” in all instances.
* The output displayed in the console at startup regarding `doomretro.cfg` now distinguishes between the file not being present because *DOOM Retro* is being run for the first time, or because the file is missing.
* Grammar has been fixed in the output in the console when only one lump is present in a PWAD.
* The game will no longer crash when `-nosfx` or `-nosound` are specified on the command-line.
* A bug has been fixed whereby floating and bobbing items would get stuck in the floor or ceiling in some instances.
* The error displayed when an invalid action is passed to the `bind` CCMD has been fixed.
* Which WAD the `COLORMAP` is being used from is now output to the console at startup.
* Spacing may now be used in the parameters passed to the `kill` and `spawn` commands in the console. For example, `spawn baron of hell` can be used as well as `spawn baronofhell`.
* The UK English spelling of “armor” (that is, “armour”) may now be used as the parameter for CCMDs that allow it.
* Entering the `kill` CCMD without any parameters will no longer kill the player, and instead will display a list of parameters. To kill the player, now use `kill player`.
* The `give` CCMD can now also be used to give the player anything that they can pick up during a game. For example, `give berserk` and `give soulsphere`.
* The console is now automatically closed when using the `give` CCMD.
* A bug has been fixed whereby corpses weren’t sliding even when the `r_corpses_slide` CVAR was `on`.
* The intensity of the red screen tint when the player has a berserk power-up and their fists up has been reduced slightly.
* The red screen tint when the player is injured, and the gold tint when the player picks something up will now show through while the player has a berserk power-up and their fists up.
* The `r_altlighting` CVAR has been removed.
* A slight current is now applied to liquid sectors, in a random direction determined at the start of each map. It may be disabled using the `r_liquid_current` CVAR.
* A bug has been fixed whereby the bottom wall texture between adjacent liquid sectors would show through in some instances.
* The player’s weapon sprite will no jump slightly when switching to and from the automap while moving.
* Although a majority of animated flats in *DOOM* are liquid, in some PWADs there are some that are not. There are now several instances in some popular PWADs where *DOOM Retro’s* liquid effects won’t be applied.
* The `r_lowpixelsize` CVAR will now be correctly parsed at startup.
* The `r_lowpixelsize` CVAR can now be set to values of `2×1` and `1×2`.
* The amount of blood splats spawned when corpses slide along the ground has been halved to 256.
* A bug has been fixed whereby generalized floors could become stuck after loading a savegame.
* The help screen has been updated to show that the <kbd><b>~</b></kbd> key opens the console.
* All instances of “DOOM” and “DOOM Retro” that appear in the console and the splash screen have been italicized.
* If an IWAD is specified on the command-line using `-iwad` but with no path, *DOOM Retro* will now check for an installation of *DOOM* purchased through [*GOG.com*](http://www.gog.com/game/the_ultimate_doom).
* If a PWAD file is loaded, savegames are now separated based on that file, rather than the IWAD file. (Because of this, and also because a change in how the folders savegames are saved into are named, savegames from previous versions of *DOOM Retro* will no longer appear in the save and load game menus.)
* The `savegamefolder` CVAR has been removed. Instead, the folder savegames are saved to and loaded from is displayed in the console at startup.
* A bug has been fixed whereby if the player died in a sector with special 11, they would appear in the next map with zero health and be unable to pick up items or alert enemies.
* The default mouse sensitivity has been increased from 16 to 24.
* Shadows are now opaque when the `r_translucency` CVAR is `off`.
* The pitch of the player’s sounds is no longer randomized when the `s_randompitch` CVAR is `on`.
* Generalized linedefs without tags are now applied locally, rather than not working at all.
* The positions that blood splats are spawned are now more accurate when blood falls to the ground.
* The `%` symbol will no longer disappear from the end of the `pm_walkbob` CVAR’s value.
* The CVAR names will no longer be shown when entering the `gp_deadzone_left` or `gp_deadzone_right` CVARs to display their values.
* *DOOM Retro* will now play MP3 and Ogg Vorbis music lumps. This requires the files `libogg-0.dll`, `libvorbis-0.dll`, `libvorbisfile-3.dll` and `smpeg2.dll` all to be in the same folder as `doomretro.exe`.
* A warning is now displayed in the console when a music lump can’t be played.
* Tilde characters are now removed from the files saved using the `condump` CCMD.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, September 5, 2015

### DOOM Retro v1.8.5

* More than one instance of `-file` may now appear on the command-line.
* The amount of “map revealed” in the output of the `playerstats` CCMD is now always calculated correctly.
* The number of times the player cheats, both in the current map and overall, as well as the overall amount of time spent playing *DOOM Retro*, are now displayed in the output of the `playerstats` CCMD.
* *BOOM’s* `MF_TRANSLUCENT` flag is now supported in *DeHackEd* lumps and files.
* When binding an action to a control using the `bind` CCMD, any other actions that are bound to that same control will now be unbound.
* A bug has been fixed whereby the mouse pointer wouldn’t be released when pressing <kbd><b>ALT</b></kbd> + <kbd><b>TAB</b></kbd> to switch to the desktop.
* The game will now pause slightly when the player uses a switch to exit a map, to stop the switch’s sound from stuttering.
* Support has been added for certain hacks to the `NODE` lump of a map. See [here](http://doomwiki.org/wiki/Linguortal) for more information.
* The chaingunner’s refire frame is now fullbright.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, August 29, 2015

### DOOM Retro v1.8.4

* Entering the `playername` CVAR without a value will now display the value it is currently set to.
* Gradual lighting is now applied to the sectors under all doors as they open and close, similar to the effect introduced in *BOOM*.
* The player is no longer gibbed when their corpse is under a door.
* Whether there are *BOOM* line specials present in maps with *DeepBSPv4* extended nodes is now displayed correctly in the output of the `mapstats` CCMD.
* The correct author is now displayed in the output of the `mapstats` CCMD.
* The effects of enabling the `s_randompitch` CVAR is now immediate.
* A bug has been fixed whereby the game could crash when trying to render the fuzz effect while paused in some rare instances.
* Whether the player’s view is lowered when in liquid sector is now toggled by changing the new `r_liquid_lowerview` CVAR. Now the existing `r_liquid_clipsprites` will only toggle whether the bottom of sprites are clipped in liquid, as expected.
* A bug has been fixed whereby messages wouldn’t appear in some instances.
* Tabs are now converted to spaces in the file created by the `condump` CCMD.
* Although quite often the same folder, the file created by the `condump` CCMD will now be put in the same folder as the executable, rather than the current working folder.
* Timestamps are now displayed next to each player message in the console. They may be disabled using the new `con_timestamps` CVAR.
* The `btsx_e1.wad` PWAD may now be used with `freedoom2.wad` as the IWAD without crashing.
* The game will no longer crash when trying to display patches with dimensions larger than 320×200.
* The game will no longer crash when trying to bind an action to an invalid control using the `bind` CCMD. And now a warning will be displayed in the console advising that it couldn’t be bound.
* The `+alwaysrun` action can now be bound to a gamepad button.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, August 20, 2015

### DOOM Retro v1.8.3

* A bug has been fixed whereby some platforms would move silently.
* The brightmap for walls with the `SW2BRN2` texture has been fixed.
* Lifts will no longer often become stuck when restoring a savegame.
* A bug has been fixed whereby the player wouldn’t die in some very rare instances when a “voodoo doll” was present in the map.
* Sectors that don’t start as liquid will now always sync with sectors that do when `r_liquid_bob` is `on`.
* Additional momentum is no longer applied to barrels in liquid.
* Music will now start after a map is loaded, and not before.
* The `playerstats` CCMD now shows the amount of damage the player has inflicted and received, and the number of times they have died, both in the current map, and in total. Accumulative totals of the number of items picked up, the monsters killed and the secrets found are also displayed.
* A bug has been fixed whereby the game would crash when trying to draw teleport lines in the automap in some instances.
* The game will no longer crash when trying to use the <kbd><b>F9</b></kbd> to quickload a game in some rare instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, August 12, 2015

### DOOM Retro v1.8.2

* Although quite often the same folder, *DOOM Retro* will now put savegames in the same folder as the executable, rather than the current working folder.
* A bug has been fixed whereby sprites would appear through closed doors in some instances.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, August 9, 2015

### DOOM Retro v1.8.1

* The width of the text is now checked before pasting in the console.
* *DOOM Retro* will now look in the same folder as the executable, rather than the current working folder, to find `doomretro.wad` and `doomretro.cfg`.
* Extra mouse buttons are now handled correctly.
* The alternate lighting of the player’s weapon may now be disabled using the new `r_altlighting` CVAR.
* The alternate lighting of the player’s weapon is now slightly lighter.
* A bug has been fixed whereby the player’s weapons would start to cycle when entering the `IDBEHOLD` cheat.
* Whether *Windows 10* is the *Home Edition* or not is now shown in the console at startup.
* The revision number of `sdl2.dll` is now shown in the console at startup.
* The default of the `vid_windowsize` CVAR is now `768x480`, giving the window a 16:10 aspect ratio.
* All liquid sectors are now slightly lower.
* *BOOM* elevators no longer shudder when they move.
* The liquid swirl animation is no longer applied to flats using the `SLIME09` texture.
* The liquid swirl animation is now applied to flats using the `SLIME05` texture.
* Brightmaps will no longer be rendered when the player has an invulnerability power-up, or in areas with a *BOOM* colormap.
* A crash will no longer occur when a *BOOM* pusher or puller thing is present in a map.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, August 2, 2015

### DOOM Retro v1.8

* An extensive number of optimizations have been made to improve the overall performance and stability of *DOOM Retro*.
* *DOOM Retro* is now compiled using [*Microsoft Visual Studio Community 2015*](http://www.visualstudio.com/vs-2015-product-editions). *Visual Studio’s* runtime library is now statically linked to the binary, meaning it doesn’t need to be installed.
* *DOOM Retro* now uses a pre-release version of [*SDL v2.0.4*](http://www.libsdl.org/).
* *DOOM Retro* now supports *BOOM*-compatible maps with the following features:
  * Deep water effects.
  * Scrolling walls, floors and ceilings.
  * Translucent walls.
  * Friction effects.
  * Custom colormaps.
  * Support for the `ANIMATED` lump.
  * Silent teleporters.
  * Elevators.
  * Generalized linedef types.
* The lighting of the player’s weapon is now slightly darker and more dynamic.
* *Direct3D* is now the default renderer, which does not have the flicker that occurs with the *OpenGL* renderer in fullscreen. As before, the renderer can be changed using the `vid_scaledriver` CVAR.
* The pillarboxes and letterboxes on the screen are now cleared each frame.
* Configuration files (with the extension `.cfg`) may now be loaded through the WAD launcher.
* If a CFG file exists in the same folder as a PWAD file with the same name, it will automatically be loaded.
* Support has been added for maps with *DeepBSP* extended nodes v4 and [*ZDOOM*](http://zdoom.org/) uncompressed normal nodes.
* Several rendering anomalies in maps have been resolved.
* Any flats that are missing in a map will now be rendered as sky, and a warning displayed in the console, rather than *DOOM Retro* exiting with an error.
* Further improvements have been made to the support for *DeHackEd* lumps.
* The translucency of the chaingun’s muzzle flash has been improved slightly.
* The “always run” feature may now be bound to a key other than <kbd><b>CAPSLOCK</b></kbd> in the console by using the `+alwaysrun` action with the `bind` CCMD.
* Movement of the player’s weapon is now interpolated to appear smoother.
* Rather than using the standard animation, which is only updated every 125 milliseconds, a much smoother swirl effect is now applied to every liquid sector. It is on by default, and can be turned off using the `r_liquid_swirl` CVAR.
* The speed of liquid sectors bobbing up and down has now been doubled.
* Things in liquid sectors no longer bob in time with each other.
* If the blockmap of a map is invalid or not present, it will now be recreated.
* The position of keycards and skull keys in the widescreen HUD when the player has no armor has been improved.
* The input in the console will now be restored after viewing the input history using the <kbd><b>&uarr;</b></kbd> key.
* The `r_playersprites` CVAR has now been implemented allowing the player’s weapon to be hidden.
* Several changes have been made to the descriptions of CCMDs and CVARs when using the `cmdlist` and `cvarlist` CCMDs in the console.
* A new `mapstats` CCMD has been implemented that will show the following information about the current map: map title, map author, node format, if the blockmap was recreated, total vertices, total sides, total lines, if *BOOM*-compatible line specials are present, total sectors, total things, map size and music title.
* The `r_maxbloodsplats` CVAR has been renamed to `r_bloodsplats_max`. Also, when it is set to `0`, it will now be shown as `0` rather than `off`.
* The `totalbloodsplats` CVAR has been renamed to `r_bloodsplats_total`.
* The `r_mirrorweapons` CVAR has been renamed to `r_mirroredweapons`.
* The `mapfixes` CVAR has been renamed to `r_fixmaperrors`.
* The `spritefixes` CVAR has been renamed to `r_fixspriteoffsets`.
* A bug has been fixed whereby weapons spawned at the start of a map weren’t being randomly mirrored if `r_mirroredweapons` was `on`.
* The format of `doomretro.cfg` has changed considerably, and is divided into two parts: CVARs and bindings.
* Tall textures are now supported.
* The wall texture between two liquid sectors (often an animated waterfall texture) will no longer rise and fall with those sectors.
* The randomness of such things as mirrored corpses and the spawning of blood splats has been improved.
* The positions of the numbers in the widescreen HUD are now improved when custom lumps are used.
* The translucency of the elements in the widescreen HUD has been increased slightly.
* The health, ammo and armor counts in the widescreen HUD will now flash briefly when the player picks up the corresponding items during a game.
* The game will now exit with an error if no subsectors are present in a map.
* Improvements have been made to the consistency by which blood splats are spawned when a monster is shot, and when a corpse slides along the floor.
* A feature is now available that randomizes the pitch of monster sounds. It is disabled by default, and can be enabled using the `s_randompitch` CVAR.
* If a PWAD is loaded, the window caption will now be changed to its filename while no map is loaded.
* A bug has now been fixed whereby the operation of the mouse wheel to select the previous/next weapon was reversed, and would no longer work at all if the user attempted to change it using the `bind` CCMD.
* The console now opens and closes slightly faster.
* The background of the console now has a slight diagonal pattern, and a drop shadow.
* The scrollbar track and dividers in the console are now translucent.
* If a PWAD is loaded that uses a custom character set, the color of the player messages in the console will now reflect the color of those characters.
* Widescreen mode will now be enabled or disabled correctly when setting the `vid_widescreen` CVAR.
* The contents of the window now updates dynamically as it is being resized.
* A bug has been fixed whereby the screen size couldn’t be adjusted in the options menu when not in a game.
* The mouse pointer is now released while the console is open.
* The window caption will no longer be reset to “DOOM RETRO” when the graphics subsystem is restarted by entering certain CVARs in the console.
* The blink rate of the text caret in the console is now the same speed as the *Windows* setting.
* The window is no longer reset to a 4:3 aspect ratio at startup.
* The position of the window is now restored correctly at startup, and when switching from fullscreen mode, if using multiple displays.
* The minimum size that the window can be resized to is now 320×240.
* The console is now closed when pressing the close button in the window’s title bar.
* If a masked midtexture is used on a one-sided line, the transparent parts will now be displayed as black rather than randomly-colored pixels. Code by Fabian Greffrath.
* Autocomplete and input history are now reset if a character is deleted in the console.
* The output in the console is now correct when the music and SFX volumes are changed in the menu.
* The graphics subsystem will now be reset when the `vid_display` CVAR is changed in the console, so displays can now be switched during a game.
* If the `vid_display` CVAR is found to be invalid at startup, it will no longer be restored to its default, in case the display it points to happens to be off. Instead, a warning will be displayed in the console, and display 1 will be used.
* An acronym for the screen resolution, and the correct aspect ratio, will now be displayed in the console at startup.
* Whether *Windows* is 32 or 64-bits will now be displayed in the console at startup.
* A small amount of ammo is now given to the player when using the `give backpack` CCMD, to be consistent with what the player is given when picking up a backpack during a game.
* A bug has been fixed whereby an additional character could be entered into a cheat sequence in some instances.
* The use of a *TiMidity* configuration file is now displayed in the console at startup.
* MAP05C and MAP16C in [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960) may now be loaded using the `map` CCMD.
* Monsters will no longer be alerted when the player makes a noise while “No Target” mode is on using the `notarget` CCMD.
* The `s_maxslicetime` CVAR has been removed.
* Since it produces the same result as using `linear`, the `anisotropic` value for the `vid_scalefilter` CVAR has been removed.
* A bug has been fixed whereby the `kill` CCMD wasn’t killing the player when they had armor.
* A bug has been fixed whereby pressing the media keys on some keyboards would cause the player to change weapons.
* The `showitems`, `showkills`, `showmapped` and `showsecrets` CCMDs, as well as the `maptime` CVAR, have all been combined into one `playerstats` CCMD.
* The `com_showfps` CVAR has been renamed to `vid_showfps`.
* The `com_showmemoryusage` CVAR has been removed.
* The `r_liquid_animatedheight` CVAR has been renamed to `r_liquid_bob`.
* A bug has been fixed whereby bobbing items could be pushed below liquid sectors in some instances.
* Text entered into the console can now be selected, with the following keyboard shortcuts being implemented:
  * <kbd><b>SHIFT</b></kbd> + <kbd><b>&larr;</b></kbd>: Select the character to the left of the text caret.
  * <kbd><b>SHIFT</b></kbd> + <kbd><b>&rarr;</b></kbd>: Select the character to the right of the text caret.
  * <kbd><b>CTRL</b></kbd> + <kbd><b>A</b></kbd>: Select all of the text.
  * <kbd><b>CTRL</b></kbd> + <kbd><b>C</b></kbd>: Copy the currently selected text to the clipboard.
  * <kbd><b>CTRL</b></kbd> + <kbd><b>V</b></kbd>: Paste text from the clipboard.
  * <kbd><b>CTRL</b></kbd> + <kbd><b>X</b></kbd>: Cut the currently selected text to the clipboard.
  * <kbd><b>CTRL</b></kbd> + <kbd><b>Z</b></kbd>: Undo the last change that was made to the text.
* A bug has been fixed whereby no evil grin would be displayed in the status bar when the player picked up a new weapon.
* `warp` can now be used as an alternative to the `map` CCMD.
* A feature has been implemented that causes corpses to be moved slightly if a monster walks over them. It is enabled by default, and may be disabled using the `r_corpses_nudge` CVAR.
* A bug has been fixed whereby monsters would sometimes fall off tall ledges.
* Friction in liquid is now only reduced for corpses.
* Active crushers will now be active again when loading a savegame.
* Improvements have been made to the player using a switch, lift or door if very close to another.
* The display of the value of the `r_lowpixelsize` CVAR in the console has been fixed.
* There is no longer any blue or green blood or blood splats in [*Freedoom*](https://freedoom.github.io/).
* A bug has been fixed whereby no maps were listed by the `maplist` CCMD in [*Freedoom*](https://freedoom.github.io/).
* Blues are now emphasized better in translucent sprites.
* The console now automatically closes when using the `map` CCMD or the `IDCLEVxy` cheat.
* The HOM indicator is now paused while the console is open.
* A bug has been fixed whereby warnings weren’t being displayed in the console.
* The <kbd><b>WINDOWS</b></kbd> key is now only disabled during a game, and not while the game is in a menu, paused, in the console, or on the title screen.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

### DOOM Retro v1.7.1

###### Thursday, May 14, 2015

* Optimizations have been made to further improve the overall performance of *DOOM Retro*.
* The stray brown pixel in the super shotgun’s sprite has now been removed from all the frames that it appears.
* Red blood splats are now slightly darker.
* A warning is now displayed in the console when a map contains an unknown object.
* Warnings are now displayed in the console when the player enters a sector, crosses a line, or shoots a switch with an unknown special.
* Sounds are now made again when liquid sectors move up or down.
* The file `doom2.wad` is no longer autoloaded if the `-file` command-line parameter without `-iwad`.
* A warning is now displayed in the console if the versions of `sdl2.dll` and `SDL2_mixer.dll` don’t match the versions that *DOOM Retro* was compiled with.
* A bug has been fixed whereby the game would crash when trying to render sprites that had been manipulated with *DeHackEd*.
* The “screen shot” string may now be changed with *DeHackEd*.
* Several further improvements have been made to *DeHackEd* support.
* The <kbd><b>HOME</b></kbd> and <kbd><b>END</b></kbd> keys will now scroll to the top and bottom of the console output, if the player has started scrolling up with the <kbd><b>PGUP</b></kbd> key.
* A bug has been fixed whereby the game would crash when using the `bind` command in the console to bind an action to some keyboard controls.
* `save` and `load` commands have now been implemented in the console.
* Savegames are now saved with a file extension of `.save` rather than `.dsg`.
* The new `savegamefolder` CVAR shows where savegame files are saved.
* The new `totalbloodsplats` CVAR shows the total number of blood splats currently in the map.
* The `r_bloodsplats` CVAR has been renamed to `r_maxbloodsplats`.
* If the player saves a game on one map, finishes that map and then starts the next one, if they then die without saving again, the savegame for the previous map will be loaded, rather than the player “pistol starting” on the new map.
* The arch-vile’s fire attack is no longer clipped in liquid.
* A bug has been fixed in some instances whereby some objects were still being clipped and bobbing when the sector they were in was no longer liquid.
* The new `thinglist` CCMD will list all things in the current level, including their (x,y,z) coordinates.
* The new `maplist` CCMD will list all the maps available to the player.
* Graphics will now automatically be reset if the `vid_screenresolution` is changed to `desktop`.
* The player’s view height is now smooth when descending between two liquid sectors of different heights.
* The widescreen HUD will now remain on the screen while the player is dead. No ammo will be displayed, though, to be consistent with the status bar.
* The par time displayed on the intermission screen is now positioned better.
* The normal use of the <kbd><b>CAPSLOCK</b></kbd> key is now disabled in the console, and will toggle the “always run” feature on/off instead.
* A bug has been fixed whereby the value displayed by the `totalkills` command in the console wouldn’t take into account any pain elementals killed using the `kill` command.
* The console is now automatically closed when using the `endgame`, `exitmap`, `kill` and `map` CCMDs.
* A bug has been fixed whereby the map name in the automap was displayed incorrectly when using [*ZDL*](http://zdoom.org/wiki/ZDL) to launch the game.
* The “fuzzy” edges of spectre shadows are now paused while the console is open.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, April 20, 2015

### DOOM Retro v1.7

* *DOOM Retro* now uses [*SDL 2.0.3*](http://www.libsdl.org/) for its graphics and audio. This brings significant performance improvements, as the screen is now scaled using hardware acceleration when possible, as well as the following features:
  * The monitor to display the game on can now be specified.
  * V-sync can now be toggled on/off.
* *DOOM Retro* now includes a console. It may be opened at any time by pressing the <kbd><b>~</b></kbd> key.
* *DOOM Retro* now has uncapped framerates, and through use of interpolation, player, monster and sector movement is much smoother.
* Several memory leaks have been fixed, making *DOOM Retro* more stable.
* A bug has been fixed whereby if a PWAD was selected in the WAD launcher that didn’t contain any map data, *DOOM Retro* would give up and not look at the other PWADs selected.
* If none of the PWADs selected in the WAD launcher contain any map data, and no IWAD is selected, `doom2.wad` will try to be loaded.
* Many performance optimizations have been made to the rendering of blood splats and shadows.
* The following changes have been made to animated liquid sectors:
  * They are all slightly higher.
  * The edge where two adjacent liquid sectors at different heights meet is now rendered correctly.
  * The player will now move smoothly when ascending from one liquid sector to a higher one.
  * No sounds are made when they move up or down.
* A bug has been fixed whereby if more than one flight of stairs was meant to be triggered at the same time, only one would be.
* The gaps around “1” digits in the HUD have been removed.
* A bug has been fixed whereby the game could crash when rendering spectres in some instances.
* Underscores now appear under the message displayed when entering the `IDBEHOLD` cheat.
* The map title will now be displayed correctly in the automap if a PWAD is loaded using [*ZDL*](http://zdoom.org/wiki/ZDL).
* Corrupt savegames will no be created if saving a game while a button is active.
* Blood splats and shadows will no longer appear on sectors without floors.
* The blood splats produced when the corpses of barons of hell and hell knights are crushed under a lowering sector are now the correct color.
* The player’s weapon is no longer off to the right by 1 pixel in some instances.
* The bottom right hand corner of the view border is now rendered correctly.
* The message that is displayed when *DOOM Retro* is run for the first time now includes a button that opens the [*DOOM Retro Wiki*](http://wiki.doomretro.com) in the default browser.
* The cursor keys will no longer make a sound when pressed on the help screen.
* A bug has been fixed whereby decorative corpses wouldn’t smear blood when sliding in some instances.
* Smoke trails are now displayed for cyberdemon rockets as originally intended.
* The “always run” setting is now remembered between games as originally intended.
* If you load a savegame that had monsters but now the `-nomonsters` command-line parameter is enabled, the correct percentage of monsters you actually did kill now appears in the intermission.
* A bug has been fixed whereby *DOOM Retro* would crash when trying to save a game in *Final DOOM* if the savegame description was changed to anything other than a map name.
* Additional blood splats spawned under decorative corpses when a map is started now won’t be randomly shifted away from the corpse if the corpse is hanging from the ceiling.
* Several improvements have been made to *DOOM Retro’s* support of *DeHackEd* lumps and files.
* Now `.bex` files as well as `.deh` files, with the same name and in the same folder as the PWAD selected in the WAD launcher, will now be automatically loaded.
* `.deh` files are no longer automatically loaded if a PWAD is loaded from the command-line. They will need to be explicitly loaded using the `-deh` command-line parameter.
* A bug has been fixed whereby some teleporters in [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960) were animating as if they were liquid.
* The position of the player arrow is now drawn much more accurately when in the automap and rotate mode is on.
* The automap will no longer disappear, nor the game crash, when zooming out in very large maps.
* Translucency is now applied to megaspheres as originally intended.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, February 3, 2015

### DOOM Retro v1.6.7

* A bug has been fixed whereby *DOOM Retro* would crash at startup when trying to run in a screen resolution that wasn’t widescreen.
* *DOOM Retro* will no longer crash after successive presses of <kbd><b>ALT</b></kbd> + <kbd><b>ENTER</b></kbd> to switch between fullscreen and windowed modes.
* Ceilings that use liquid textures will now render correctly.
* An error will now be displayed at startup if a WAD file contains DeePBSP or ZDBSP nodes.
* The `saturation` setting in `doomretro.cfg` has been deprecated.
* A bug has been fixed whereby the shadows of dropped items weren’t also being mirrored when the `mirrorweapons` setting was `true` in `doomretro.cfg`.
* Weapons spawned at the start of a map are now also mirrored when the `mirrorweapons` setting was `true` in `doomretro.cfg`.
* *Vanilla DOOM’s* [“long wall error”](http://doomwiki.org/wiki/Long_wall_error) has been fixed.
* Further optimizations have been made to improve the overall performance and stability of *DOOM Retro*.
* Teleporters in [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960) are now drawn correctly in the automap before they have been triggered.
* Whether the automap is active or not, and any automap marks, are now saved in savegames. (Note that this change breaks savegame compatibility with previous versions of *DOOM Retro*.)
* A header comment has been added to the top of `doomretro.cfg`, with a note advising to “go to http://wiki.doomretro.com for information on changing these settings”.
* The FPS counter displayed when `-devparm` is specified on the command-line now won’t be hidden when taking a screenshot, and will continue to update when in a menu or the game is paused.
* Diminished lighting from the player has been enhanced.
* Blood splats are now only spawned at the same height as corpses as they slide.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Sunday, January 25, 2015

### DOOM Retro v1.6.6

* The new liquid animation that was introduced in *DOOM Retro v1.6.5* has been improved upon such that the entire textures of the sectors will now also rise and fall, rather than just their edges.
* Due to this change in animation, the floating objects in liquid sectors now rise and fall in sync with each other and the sector.
* The bottom of objects in liquid that don’t float (such as monsters and barrels) will now be clipped in sync with the rise and fall of the liquid.
* All liquid sectors are now animated, avoiding issues where one liquid sector would be animated, but another liquid sector adjacent to it wouldn’t be.
* A bug has been fixed whereby the liquid animation was stopping the player and/or monsters from being able to enter certain areas of some maps.
* The brightmap for the `COMP2` wall texture has been fixed.
* A bug has been fixed whereby other monsters could infight with arch-viles.
* The teleporter texture used in [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960) no longer animates like a liquid.
* Savegames will now be placed in the `savegames\DOOM2.WAD\` folder rather than the `savegames\unknown.wad\` folder when `nerve.wad` is loaded.
* The player’s weapon is now recentered after teleporting.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Monday, January 19, 2015

### DOOM Retro v1.6.5

* Many optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby secret sectors that lower and change their texture may not be displayed in the automap correctly.
* Decorative corpses spawned at the start of a map are now randomly mirrored as intended.
* A bug has been fixed whereby the player sometimes wouldn’t trigger a teleport when running over it in some instances.
* Changes have been made to how lighting is calculated.
* Liquid sectors, and the partially submerged objects that are on them, now animate up and down. This feature may be disabled by changing the `animatedliquid` setting in `doomretro.cfg` to `false`.
* A bug has been fixed whereby objects on a sector that lowers and becomes liquid wouldn’t update immediately (that is, the bottom of their sprites wouldn’t be clipped, their shadow wouldn’t be removed, and blood splats wouldn’t be removed either).
* Blood splats that are spawned around decorative corpses at the start of a map are now spawned in a more natural-looking circular pattern, and may also be offset slightly from the corpse to give the impression that the corpse may have slid.
* Blood splats that are spawned when crushing a corpse are now also spawned in a circular pattern.
* The positions of shadows for several monsters have been improved.
* The total amount of blood splats that can be spawned when a corpse slides across the floor has been doubled.
* The total amount of blood splats that can be spawned when a corpse slides across the floor is now saved in savegames. This breaks savegame compatibility with previous versions of *DOOM Retro*.
* Blood splats that are spawned around decorative corpses at the start of a map now come from this same total.
* A bug has been fixed whereby it was possible for the screen to switch between the `TITLEPIC` and `CREDIT` lumps before wiping the screen after the player selects a skill level in the menu to start a new game.
* The top and bottom edges of spectre shadows are now “fuzzy”.
* The top and bottom edges of spectre shadows are now “fuzzy”, and also lighter, in *DOOM II’s* cast sequence, to match how they appear in the game.
* A savegame slot that isn’t empty is now automatically selected in the load game menu if the player previously exited the save game menu while a savegame slot that is empty was selected.
* The screen will now flash the same amount for keycards and skull keys as for other pickups.
* The window can now be resized when in widescreen mode.
* Several changes have been made so that *DOOM Retro* will behave better when custom sprites are present in a PWAD.
* *DOOM Retro* will no longer exit with an error if a flat’s texture is missing in a WAD.
* Zooming in and out of the automap using a gamepad has now been fixed.
* The textures `PLANET1`, `SW2BLUE` and `SW2MARB` now have brightmaps.
* `nerve.wad` will now be automatically loaded if `doom2.wad` is selected in the WAD launcher and there are also one or more PWADs selected that don’t contain any map data.
* The translucency of blood splats has been reduced slightly, and their edges are no longer softened.
* Green blood splats are now slightly darker.
* A bug has been fixed whereby some shootable switches would only work once.
* The edges of shadows are now black when the `translucency` setting in `doomretro.cfg` to `false`.
* The `sfxvolume` and `musicvolume` settings in `doomretro.cfg` will no longer round down to `0%` when set to `6%`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Wednesday, December 10, 2014

### DOOM Retro v1.6.4

* Several optimizations have been made in an attempt to further improve the overall performance of *DOOM Retro*.
* *DeHackEd* files (`*.deh` and `*.bex`) are now displayed and can be loaded in the WAD launcher. <kbd><b>CTRL</b></kbd>-click to select them along with the WAD file(s) you want to load.
* When multiple PWADs are selected in the WAD launcher, and with no IWAD, every PWAD will now be checked until the IWAD required is determined, rather than potentially failing if the first PWAD checked contains no map data.
* The FPS counter will now work correctly when `-devparm` is specified on the command-line.
* Problems with movement of the mouse have been fixed when starting *DOOM Retro* in windowed mode.
* Panning when follow mode is off in the automap is now back to working correctly.
* Several changes have been made so that the window caption is now updated correctly.
* Smoke trails and bullet puffs are now grayer then before, and displayed using 33% translucency rather than additive translucency.
* The player’s view will no longer be lowered further due to the `footclip` setting when standing in a self-referencing sector.
* Improvements have been made to switching weapons using a gamepad.
* For non-widescreen displays, the status bar is no longer displayed when in the automap if it isn’t displayed during a game.
* A bug has been fixed whereby the `screensize` setting was being reset at startup for non-widescreen displays.
* The “dead zones” of the left and right thumbsticks of gamepads can now be adjusted using the `gamepad_leftdeadzone` and `gamepad_rightdeadzone` settings in `doomretro.cfg`. They are `24%` and `26.5%` by default.
* There are now settings for every control in the automap, for both the keyboard and gamepad, in `doomretro.cfg`.
* The `sfx_volume` and `music_volume` settings in `doomretro.cfg` have had their underscores removed, and their values are now displayed as percentages.
* Smoke trails are no longer displayed for revenants’ non-homing rockets, as intended.
* The menu may still be opened with the gamepad’s <kbd><b>START</b></kbd> button if `gamepad_sensitivity` has been reduced to `0`.
* A bug has been fixed whereby the help screens wrongly indicated that the <kbd><b>A</b></kbd> and <kbd><b>D</b></kbd> keys were used to turn rather than strafe.
* The overall gamepad sensitivity has been increased even more, and the default of `gamepad_sensitivity` has now been doubled from `16` to `32`.
* The player will now bob as intended when `playerbob` is greater than `75%`.
* Changes have been made to the messages displayed when adding and clearing marks.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, November 25, 2014

### DOOM Retro v1.6.3

* Several internal optimizations have been made in an attempt to improve the overall performance of *DOOM Retro*.
* Decorative corpses are now randomly mirrored when a map is loaded.
* A potential overflow has been fixed if there was more than 32 characters in a savegame description.
* Cheat sequences can now be overridden in *DeHackEd* lumps and files.
* The bobbing up and down of floating power-ups can now be disabled by setting `floatbob` to `false` in `doomretro.cfg`.
* The window caption is now updated when ending a game through the options menu.
* `doom2.wad` savegames are no longer placed in the `nerve.wad` folder.
* A bug has been fixed whereby extreme slowdown would occur if a large amount of monsters (and blood splats) were on a moving sector.
* Pressing the window’s maximize button will now switch to fullscreen mode.
* The messages displayed when the player picks up ammo are now accurate, since the amount can vary depending on the skill level chosen and/or whether a monster dropped it or not.
* A bug has been fixed whereby monsters were not always remembering their last target when attacked by something else. This would affect monster infighting in some instances.
* Widescreen mode is now supported in windowed mode. Rather than displaying pillarboxes, the window will expand and contract horizontally as necessary when toggling widescreen mode on and off.
* The display of the “screen size” slider in the options menu is now fixed when in windowed mode.
* The width and height of the window are no longer set to be even numbers.
* A stretched cursor will no longer sometimes briefly appear in the center of the window when opening the menu in windowed mode.
* *DOOM Retro* should no longer crash when switching between fullscreen and windowed modes with <kbd><b>ALT</b></kbd> + <kbd><b>ENTER</b></kbd>, and when resizing the window.
* If `nerve.wad` is loaded, the window caption will now be updated to indicate the selected expansion when in the expansion and skill level menus before starting a game.
* Shadows are now saved in savegames, even when the `shadows` setting in `doomretro.cfg` is `false`.
* A bug has been fixed whereby the value of the `bloodsplats` setting in `doomretro.cfg` was always set to the default of `unlimited` regardless of what it was changed to.
* If a setting in `doomretro.cfg` that has a range of values is set out of range, it will be capped at the minimum or maximum, rather than changed back to the default.
* A `mapfixes` setting in `doomretro.cfg` has been implemented to allow the several hundred map-specific fixes that *DOOM Retro* applies to be enabled or disabled by type. The default is `linedefs|sectors|things|vertexes`.
* The weapon number keys can now be changed by altering the `key_weapon1` to `key_weapon7` settings in `doomretro.cfg`.
* A bug has been fixed whereby some combinations of flags used by the `corpses` setting in `doomretro.cfg` weren’t being considered valid.
* Key settings in `doomretro.cfg` can no longer be set to function keys.
* The controls set by `gamepad_prevweapon` and `gamepad_nextweapon` can no longer be used when the game is paused.
* Faster switching to the next and previous weapons is now allowed.
* Improvements have been made to the menu in [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* Smoke trails are no longer applied to revenant non-homing rockets by default.
* Instead of just `on` and `off`, the `smoketrails` setting in `doomretro.cfg` has been changed to allow smoke trails to be enabled or disabled depending on who fired the projectile. The setting can be any combination of the following flags: `player` (smoke trails for player rockets), `revenant1` (smoke trails for revenant non-homing rockets), `revenant2` (smoke trails for revenant homing rockets) and `cyberdemon` (smoke trails for cyberdemon rockets). The default is `player|revenant2|cyberdemon`.
* A bug has been fixed whereby pressing the <kbd><b>CAPSLOCK</b></kbd> key on the title screen will cause the use of the key to then become inverted (that is, turning <kbd><b>CAPSLOCK</b></kbd> on would turn “always run” off, and vice versa).
* The display of the asterisk character is now allowed in a savegame description.
* The player arrow in the automap is now displayed correctly when zoomed in.
* A bug has hopefully been fixed that caused objects to sometimes disappear when standing on sector boundaries.
* Several changes have been made to improve *DOOM Retro’s* *DeHackEd* support.
* A bug has been fixed present in *Vanilla DOOM* whereby the vertical position of an arch-vile’s fire attack could be set incorrectly in some instances.
* There is no longer any small upward thrust when the player is receives an arch-vile’s fire attack while “no clipping mode” is on using the `IDCLIP` cheat.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, November 8, 2014

### DOOM Retro v1.6.2

* A bug, inadvertently introduced in v1.6.1, has been fixed whereby the wrong colored keys would appear in the HUD.
* The default value for the `saturation` setting in `doomretro.cfg` has been changed from `0.75` to `1.00`.
* The `gamma` setting in `doomretro.cfg` has been renamed to `gammacorrectionlevel`, and the alias `off` can now be used when it is set to `0`.
* Blood splats will now appear in maps from PWADs that use a *DeHackEd* file or lump.
* The clipping of the bottom of things will now be updated when the sector they’re in changes to/from liquid.
* Spectres now have slightly lighter shadows than the other monsters, with fuzzy edges.
* Minor changes have been made to the values of settings in `doomretro.cfg`. The `true`/`false` aliases now appear as `on`/`off`, and the `none` alias now appears as a hyphen. Aliases `yes`/`no` may also be used.
* Setting the mouse sensitivity to 0 will no longer disable the gamepad as well.
* The range of gamepad sensitivity has been increased.
* Several minor changes have been made to the properties of pickups in [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* The <kbd><b>,</b></kbd> and <kbd><b>.</b></kbd> keys can now also be used to strafe left and right, as they could in *Vanilla DOOM*. They are changed using the `key_strafeleft2` and `key_straferight2` settings in `doomretro.cfg`.
* Shifted characters are now allowed when entering savegame descriptions. (In *Vanilla DOOM*, pressing <kbd><b>SHIFT</b></kbd> + <kbd><b>/</b></kbd>, for example, would still display “/” rather than “?”.)

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, November 4, 2014

### DOOM Retro v1.6.1

* If a *DeHackEd* file (with a `.deh` extension) is present with the same name and in the same folder as the selected PWAD, it will be automatically opened as well.
* Improvements have been made to when the player slides against walls.
* A bug has been fixed whereby the screen would not render fully after switching from fullscreen to windowed modes when pressing <kbd><b>ALT</b></kbd> + <kbd><b>ENTER</b></kbd>.
* Several compatibility fixes have been made when using *DeHackEd* files and lumps.
* Savegames for [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960) are now separated by episode.
* A bug has been fixed whereby [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/) wouldn’t load at all. Specific support has now been added for it.
* Fake contrast is now applied to outdoor areas again.
* Thing triangles no longer appear for the invisible corpses in *Chex Quest* when using the `IDDT` cheat in the automap.
* The default value of `snd_maxslicetime_ms` has been changed from `120` to `28`. This is consistent with *Chocolate DOOM’s* default, and reduces the slight lag when playing sounds.
* The `centeredweapon` setting has been added to `doomretro.cfg`. Setting it to `true` will center the player’s weapon each time it’s fired (the default). If `false`, *Vanilla DOOM’s* behavior is used.
* A bug has been fixed whereby input would momentarily become stuck if the splash screen was skipped at startup.
* Blood splats are now green in *Chex Quest* as intended.
* A bug has been fixed whereby switching to and from the chainsaw using the number keys really quickly would cause either a crash, or the player’s weapon to disappear completely.
* The player’s weapon bob is now consistent with *Vanilla DOOM*.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, October 25, 2014

### DOOM Retro v1.6

* Further enhancements have been made to *DOOM Retro’s* overall performance and stability.
* Widescreen mode is no longer stretched horizontally on displays with a greater aspect ratio than 16:10.
* Monsters, pickups, corpses and rockets now all cast dynamic shadows on the ground. These shadows are not cast when in or over liquid, nor when the player has either the light amplification visor or invulnerability power-ups. Shadows may be disabled by setting `shadows` to `false` in `doomretro.cfg`. Shadows are translucent, but if `translucency` is set to `false` in `doomretro.cfg`, they are solid black.
* All objects are now partially submerged when standing in liquid. This feature may be disabled by setting `footclip` to `false` in `doomretro.cfg`.
* The player’s view is now lowered slightly when standing in liquid. Setting `footclip` to `false` in `doomretro.cfg` will also disable this.
* Less friction is applied to corpses and dropped items when in liquid.
* Blood splats are now drawn regardless of how far they are away from the player.
* The firing animation of the heavy weapon dude when facing to the right is now smoother.
* Since they all cast shadows now, the lost soul, cacodemon and pain elemental are higher off the ground in *DOOM II’s* cast sequence.
* Deaths are now randomly flipped in the cast sequence.
* It is now possible to warp to episodes beyond episode 4 using the `-warp` command-line parameter.
* *DOOM Retro* now supports [*DeHackEd*](http://doomwiki.org/wiki/Dehacked) files and lumps, including those with *BOOM* extensions. *DeHackEd* files may be opened by using the `-deh` or `-bex` command-line parameters. `DEHACKED` lumps will automatically be parsed unless `-nodeh` is specified on the command-line.
* If a map from a PWAD is loaded, and no `DEHACKED` lump is present in the PWAD specifying its name, then the PWAD’s name will also be included in the automap.
* The intermission screen is now displayed at the end of `ExM8`.
* The amount of kills is now correctly capped at 100% on the intermission screen in all instances.
* Walls are now drawn with even greater precision, fixing many graphic anomalies that may appear when the player stands on a line with a change in height.
* Fake contrast is no longer applied to walls in outdoor areas.
* The position of floating items has been raised off the ground slightly.
* A message is now displayed in `stdout.txt` when an arch-vile resurrects a monster.
* The mouse controls that select next and previous weapons can now be set by changing `mouse_nextweapon` and `mouse_prevweapon` in `doomretro.cfg`. They are set to `wheeldown` and `wheelup` by default.
* The use action may now be bound to a mouse button by changing the `mouse_use` setting in `doomretro.cfg`. It is set to `none` by default.
* The maximum mouse sensitivity has been doubled from `64` to `128`.
* Mouse and gamepad sensitivity are now two separate settings.
* Both the mouse and gamepad are now completely disabled when sensitivity is set to `0` and not in a menu. Previously, only the turning movement of both devices was disabled.
* The overall gamepad sensitivity has now been reduced slightly.
* Moving sliders in the menu with the gamepad is now faster.
* When a game is loaded, including when a savegame is autoloaded after a player’s death, a message is now displayed.
* A version number is now embedded in each savegame to avoid a crash when attempting to load an older and incompatible savegame.
* Minor changes have been made to the help screen.
* The red screen tint when the player has the berserk power-up has been reduced slightly.
* All drop shadows in the menus, in the HUD and on messages are now solid when `translucency` is `false`.
* Specific support has been added for *Chex Quest*:
  * The window caption is displayed as “Chex Quest”.
  * `chex.deh` is automatically loaded if it’s present in the same folder as `chex.wad` and `-nodeh` isn’t specified on the command-line.
  * The episode menu is skipped.
  * No obituaries are printed to `stdout.txt`.
  * No items are dropped.
  * No additional blood is spawned around map decorations.
  * All blood splats are green.
  * Since corpses are effectively invisible, there are no blood splats or crunch sounds made if an invisible corpse happens to be under a door.
  * The screen will flash green rather than red when the player is injured.
* Specific support has also been added for [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower In The Fountain Of Sparks*](https://www.doomworld.com/forum/topic/69960):
  * The window caption is displayed as the PWAD’s full title.
  * *DOOM Retro’s* custom sprite offsets, previously only used with the official *DOOM* IWADS, are now used. This fixes a bug that caused some level decorations to “twitch” as they animated.
  * If only `btsx_e2a.wad` is loaded from the WAD launcher, then `btsx_e2b.wad` is automatically loaded as well, and vice-versa.
  * The map number in the message displayed when using the `IDCLEVxy` cheat is of the form “E*x*M*yy*” to match what’s displayed in the automap.
  * To avoid a crash, you are no longer able to warp to a *DOOM II* map that is not replaced by a map in the PWAD, using either the `-warp` command-line parameter or the `IDCLEVxy` cheat.
* The amount of blood splats produced when crushing corpses is now based on their width.
* The edges of blood splats have now been softened slightly.
* Decorative corpses can now be crushed.
* Changes have been made to the text on the splash screen.
* The splash screen now fades onto and off of the screen at startup.
* The splash screen may now be accelerated by pressing a key or button.
* Monsters will now try to move away from tall drop offs.
* When spawning blood splats around decorations when a map is loaded, blood splats will no longer be spawned on floors close to but higher than the decoration itself.
* Textures `RROCK05` to `RROCK08`, and `SLIME09` to `SLIME12`, are no longer considered liquids, and therefore blood splats can now appear on them.
* Further improvements have been made in detecting mancubus fireball collisions.
* Spawn cubes are no longer spawned when the `-nomonsters` command-line parameter is used.
* A bug has been fixed whereby a flashing skull key in the HUD would cause other keys next to it to shift slightly.
* In the automap, lines won’t be shown as being teleport lines (that is, in dark red) unless:
  * it is part of an obvious teleport by being adjacent to a floor with a `GATEx` texture,
  * the player has been teleported by it, or,
  * the player is using the `IDDT` cheat.
* All settings in `doomretro.cfg` are now validated at startup. If any setting is found to be invalid, it will be reset to its default.
* Improvements have been made to the smoke trails of player and cyberdemon rockets, and revenant projectiles. The smoke will take slightly longer to dissipate, and is better randomized.
* The heights of revenants and arch-viles have been restored to their lower defaults. Many maps rely on this, and so therefore this fixes instances whereby sometimes these monsters would be stuck and wouldn’t attack the player.
* A monster will no longer go fullbright when firing a projectile if they are facing away from the player.
* A bug has been fixed whereby if an action was assigned to the <kbd><b>I</b></kbd> key, it wouldn’t work since “`I`” is the first character of every cheat.
* The patch offsets for textures `SKY1` and `BIGDOOR7` have been corrected.
* Long map titles in the automap and when saving a game, as well as player messages, are now truncated and followed with an ellipsis.
* The read-only `xinput` setting has been removed from `doomretro.cfg`.
* A read-only version setting has been added to `doomretro.cfg`.
* If `doomretro.cfg` was deleted, and then regenerated by running *DOOM Retro*, the defaults for the `corpses` and `videodriver` settings are now correct.
* The *DOOM Retro* icon is now always used in the window caption.
* All trademark symbols have been removed from the window caption.
* Multiple PWADs may now be selected in the WAD launcher without an IWAD.
* Monsters will no longer go to sleep after killing a monster and not seeing the player anymore.
* All palette effects are now removed while a menu is displayed or the game is paused.
* Improvements have been made to the translucency of the blue armor in the HUD.
* The player can now still pass under solid hanging corpses if there is sufficient room.
* A bug has been fixed whereby the <kbd><b>,</b></kbd> key was being incorrectly mapped, causing it to act like the <kbd><b>&ndash;</b></kbd> key if it was bound to anything.
* Use of the <kbd><b>ALT</b></kbd> and <kbd><b>CTRL</b></kbd> keys in the menu has been disabled.
* Empty savegame slots may no longer be selected in the load game menu.
* Only update savegame descriptions of the form `ExMy` in *DOOM* games, not *DOOM II* games.
* The text caret in the save game menu is now better positioned when using a PWAD with a custom character set.
* A bug has been fixed whereby the position of the skull cursor could be misplaced when exiting and then returning to the menu using the gamepad in some instances.
* The number of characters that can be entered in a savegame slot is now calculated correctly.
* If the <kbd><b>CAPSLOCK</b></kbd> key was on before *DOOM Retro* was run, it is now turned back on as necessary when quitting.
* Player messages and the map name in the automap are now translucent and have drop shadows when using a PWAD with a custom character set.
* A bug has been fixed whereby an arch-vile could resurrect a monster for it to instantly become stuck in another monster.
* The player’s screen will no longer flash red if they are hit by a projectile while already dead.
* A bug has been fixed whereby the game would crash when using the `-nosfx` or `-nosound` command-line parameters.
* A bug has been fixed whereby you could switch weapons when zooming in the automap if both actions were set to the gamepad’s left and right shoulder buttons.
* Skies with heights other than 128 pixels are now rendered correctly.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, August 15, 2014

### DOOM Retro v1.5.2

* Monster targets are now completely restored upon loading a game, regardless of whether they were targeting the player, or they were infighting.
* A Boss Brain no longer needs to be in MAP30 for the monsters it spawns to telefrag the player.
* A bug has been fixed whereby monsters were allowed to be above or below other monsters after teleporting, when normally they wouldn’t be.
* Improvements have been made to the position of some elements in the menu and HUD.
* Optimizations have been made to the loading of maps, and the lighting of the player’s weapon.
* A bug has been fixed whereby the game could crash when trying to draw the player’s weapon in pitch black areas in some instances.
* A bug has been fixed whereby the <kbd><b>CAPSLOCK</b></kbd> key was not being turned off when quitting the game.
* Pressing <kbd><b>CAPSLOCK</b></kbd> during a game will now display an `ALWAYS RUN ON/OFF` message. If when quitting the game the <kbd><b>CAPSLOCK</b></kbd> key is still on, it will be turned back on the next time *DOOM Retro* is started.
* In the previous version of *DOOM Retro*, the default video driver was changed from *Windows GDI* to *DirectX* to help in improving performance in fullscreen mode. If *DirectX* wasn’t installed, *DOOM Retro* would exit with an error. Now, if one video driver fails, *DOOM Retro* will try the other driver before exiting with an error.
* All in-game messages are now output to `stdout.txt`, whether messages are enabled or not.
* Whenever the player or a monster is killed, a message is displayed in `stdout.txt`.
* Minor changes have been made to a few messages.
* A bug has been fixed whereby *DOOM Retro* would exit with an error if the `IDDQD` cheat was used to resurrect a dead player.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, August 7, 2014

### DOOM Retro v1.5.1

* A splash screen is now displayed briefly when *DOOM Retro* is opened.
* Several optimizations have been made to improve *DOOM Retro’s* overall performance and reduce the size of its executable.
* *DOOM Retro* is now back to using the desktop resolution by default.
* If `doom2.wad` is selected by itself in the WAD launcher, `nerve.wad` will be automatically loaded if it’s in the same folder, and *Hell on Earth* will be preselected in the expansion menu. If `nerve.wad` is selected by itself, *No Rest for the Living* will be preselected instead.
* The `DOOMWADDIR` environment variable is now checked when automatically looking for IWAD files.
* No longer is anything output to the `stdout.txt` and `stderr.txt` files generated by SDL.
* A bug has been fixed whereby sometimes the press of a mouse button would register twice in a menu.
* The <kbd><b>PAUSE</b></kbd> key is now disabled on the title screen.
* The limit on the length of music in PWADs has been removed.
* A bug has been fixed whereby a crash would occur when changing to some levels.
* The sounds heard when selecting a skill level to start a new game, and using the `IDCLEVxy` cheat, will no longer be cut off.
* In *Vanilla DOOM*, when a game is saved, all monsters would lose their targets. This has now been changed so that if a monster has a target when a game is saved, when that game is then loaded, they will target the player.
* A bug has been fixed whereby the player could still switch to their fist even though they had a chainsaw and a berserk power-up in the previous map.
* A bug present in *Vanilla DOOM* has been fixed whereby the player’s fist would be selected at the start of a map even though they had a chainsaw if they had the berserk power-up in the previous map.
* `doomretro.wad` is no longer renamed to `doomretro.wad.temp` while the WAD launcher is open.
* Each time the player switches to their fist while they have the berserk power-up, the power-up sound will now be heard.
* The red palette effect of the berserk power-up is now slightly redder.
* Palette effects will no longer be displayed while a menu is on the screen, or while the player is dead.
* The player’s view will now lower if they happen to die while on top of something else.
* Further improvements have been made to how custom menu graphics from PWADs are displayed.
* *DOOM Retro* will no longer crash if the player walks over a very deep hole.
* The intensity of the vibration effect for XInput-compatible controllers when the player fires a weapon or is injured has been increased. Previously, the effect on the *Logitech F710 controller* could barely be felt.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Thursday, July 17, 2014

### DOOM Retro v1.5

* An extensive amount of enhancements have been made to *DOOM Retro’s* overall performance and stability.
* By default, *DOOM Retro* will now try running at a resolution of 1280×800. If that isn’t available, the desktop resolution will be used.
* The following changes have been made to corpses:
  * Corpses, hanging corpses, and pools of blood and/or guts that prepopulate maps are now each surrounded by an additional pool of blood.
  * Corpses (including those that prepopulate maps) will now slide in reaction to the blasts from projectile and barrel explosions.
  * The masses of some of the lighter monsters have been increased so they slide more realistically.
  * As corpses slide, blood will now be smeared on the ground. In previous versions of *DOOM Retro*, blood splats were only ever produced when monsters were injured by hitscan weapons, but now, since corpses tend to slide from the force of impact, blood splats are produced when they are injured by projectile weapons as well.
  * Corpses (and dropped items) now have torque applied to them, allowing them to slide around more realistically.
  * Arch-viles can now resurrect corpses that prepopulate maps.
  * Blood will no longer be produced as monsters are being crushed by a moving sector, but when they do finally die, instead of a small pile of blood and guts, a larger pool of blood in the correct color is produced. The same happens for corpses under closing doors.
  * These various new corpse-related changes can be toggled on and off by using any combination of flags with the new `corpse` setting in `doomretro.cfg`. It’s set to `mirror|slide|smearblood|moreblood` by default.
* A bug has been fixed whereby mouse movement wasn’t smooth for some users.
* The lower mouse sensitivity settings are now slightly slower. The default setting has been increased from `12` to `16` to accommodate for this.
* Blood splats will no longer be spawned on additional animated flats present in PWADs.
* Weapons dropped by shotgun guys and heavy weapon dudes when they die are no longer randomly mirrored by default. This feature can be reenabled by changing the new `mirrorweapons` setting in `doomretro.cfg` to `true`.
* Blood is now randomly mirrored.
* The limit has been removed on the number of monsters a Boss Brain can spawn.
* A bug from *Vanilla DOOM* has been fixed whereby spawn cubes would miss east and west targets. See the [*DOOM Wiki*](http://doomwiki.org/wiki/Spawn_cubes_miss_east_and_west_targets) for more information.
* A bug has been whereby shooting at a monster being raised by an arch-vile could cause the game to crash.
* Floating monsters (cacodemons, pain elementals and lost souls) can no longer get stuck together.
* The corpses of cacodemons will no longer sometimes get suspended in midair.
* A bug has been fixed whereby a frame in the cacodemon’s death sequence wasn’t displaying correctly.
* Elements on the intermission screen are now positioned better if custom graphics from PWADs are used.
* The player’s bob has been reduced by 25% to correspond with the latest official versions of *DOOM*. A `playerbob` setting has been created in `doomretro.cfg` to change this. The default value is `75%`.
* The player’s weapon bob is now slightly smoother, and is centered when a shot is fired.
* *XBOX 360* controllers will now vibrate when a weapon is fired, and also when the chainsaw is idle.
* The limits on the number of moving ceilings and platforms in a level have been removed.
* The message displayed when *DOOM Retro* is run for the first time has been updated to indicate that “additional PWAD files may also be selected by CTRL-clicking on them” in the WAD launcher.
* PWADs can now be selected without an IWAD in the WAD launcher. *DOOM Retro* will first look for the required IWAD in the same folder as the PWAD, and if it isn’t found, will then look in the last folder an IWAD was loaded successfully.
* Similarly, PWADs can now be specified on the command-line without an IWAD. If no `-iwad` command-line parameter is present, *DOOM Retro* will first look for the required IWAD in the same folder as the PWAD, and if it isn’t found, will then look in the last folder an IWAD was loaded successfully.
* A bug has been fixed whereby *DOOM Retro* may have crashed silently after selecting a WAD in the WAD launcher.
* The command-line parameter `-expansion` may now be used. Use `-expansion 1` to autostart *DOOM II: Hell On Earth*, and use `-expansion 2` to autostart *DOOM II: No Rest For The Living* (if `nerve.wad` is specified using `-file` or `-pwad`).
* The value that can be specified by the `-warp` command-line parameter can now be of the form `ExMy` or `MAPxy`.
* If a [*Freedoom*](https://freedoom.github.io/) IWAD is loaded in *DOOM Retro*, a message is displayed explaining that “FREEDOOM requires a BOOM-compatible source port, and is therefore unable to be opened”. This message won’t be displayed if the IWAD is loaded with an additional PWAD, so [*Freedoom*](https://freedoom.github.io/) can be used as a resource for the maps present in that PWAD.
* Further improvements have been made when using custom graphics in PWADs, particularly if [*Freedoom*](https://freedoom.github.io/) is being used.
* A bug from *Vanilla DOOM* has been fixed whereby corrupt texture names would be displayed in the error if a texture couldn’t be found. See the [*DOOM Wiki*](http://doomwiki.org/wiki/Absurd_texture_name_in_error_message) for more information.
* A bug has been fixed whereby pressing the <kbd><b>ENTER</b></kbd> key or the left mouse button when in the help screen wouldn’t restore widescreen mode.
* The speed of turning with the gamepad’s right thumbstick when holding down the left trigger to run has been reduced slightly.
* The screen is now a constant tint of red while the player has the berserk power-up and their fist selected.
* The `blur` and `grayscale` settings that control the menu background have been removed from `doomretro.cfg`. The menu background will now always be blurred and gray.
* A bug has been fixed whereby the position of some items would be affected by moving platforms nearby.
* To better replicate the look of CRT monitors, which are/were not as bright as current LCD monitors, desaturation is now applied to *DOOM Retro’s* graphics. Changing the `saturation` setting in `doomretro.cfg` to `0` gives a grayscale effect, `1.0` is normal saturation, and `0.75` is the default.
* `gammalevel` has been changed to just `gamma` in `doomretro.cfg`.
* A bug has been fixed whereby non-solid hanging corpses would drop to the floor when above a moving sector in some instances.
* <kbd><b>ALT</b></kbd> + <kbd><b>F4</b></kbd> will now quit the game instantly without prompting.
* A bug has been fixed whereby the music on certain maps wouldn’t loop correctly.
* Monsters of the same species are now able to infight using projectiles when the player dies.
* The map names *Keen* and *IDKFA* in *DOOM II (BFG Edition)* are now correctly identified when saving a game.
* To allow neater sorting in the `Pictures\DOOM Retro` folder, determiners are now put at the end of screenshot names. For example, the file `The Inmost Dens.bmp` becomes `Inmost Dens, The.bmp`.
* `MUS_DDTBLU` is now played instead of `MUS_DDTBL2` when using the `IDMUS09` cheat in `nerve.wad`.
* The lighting of sprites has been changed slightly.
* The edges of the automap have been darkened slightly.
* Sounds are no longer cut off once an object has been removed. For example, rocket and barrel explosions are now slightly longer.
* When exiting a level, the exit switch will now be updated before toggling widescreen mode off.
* Brightmaps can now be toggled off using the `brightmaps` setting in `doomretro.cfg`.
* A bug has been fixed whereby the `IDKFA` cheat would still register when the player already had everything the cheat provided.
* Armor will no longer flash in the HUD when it is low.
* The clip graphic in the HUD has been shifted upwards by 1 pixel.
* Monsters are now smarter, and will avoid crushing ceilings and other damaging areas.
* The game will no longer crash if `bloodsplats` is set to `0` in `doomretro.cfg`.
* Translucency effects are now improved for white objects on a blue background.
* A `homindicator` setting has been added to `doomretro.cfg`. It is `false` by default.
* A bug has been fixed whereby the screen wouldn’t flash red if the player received only 1% damage.
* The smoke trail of revenant projectiles has been positioned better.
* The `bloodsplatsvisible` setting has been removed.
* Widescreen mode is now retained the next time *DOOM Retro* is run if it happens to crash in a 4:3 mode.
* The chance of the super shotgun gibbing a monster at point blank range has been increased slightly.
* If a screenshot is taken with the <kbd><b>PRINTSCREEN</b></kbd> key when not in a game, the resulting file will be named accordingly: `Title.bmp`, `Help.bmp`, `Intermission.bmp` or `Finale.bmp`.
* The <kbd><b>CAPSLOCK</b></kbd> key is turned off if on when quitting the game.
* Some of the less-used mouse controls from *Vanilla DOOM* have been reimplemented. Disabled by default, they can be reenabled using the following settings in `doomretro.cfg`:
  * Set `novert` to `true` to allow vertical mouse movement to move the player forward/back.
  * Set `mouse_forward` to a mouse button to move forward.
  * Set `mouse_strafe` to a mouse button to strafe.
  * Set `dclick_use` to `true` so double-clicking the mouse buttons set by `mouse_forward` and `mouse_strafe` above will perform a use action.
* A bug has been fixed whereby the values `middle` and `right` representing mouse buttons in `doomretro.cfg` were switched.
* Now pressing a key the first time on a finale text screen will display all the text, and a second press will then advance to the next map.
* A bug has been fixed whereby pressing <kbd><b>SPACE</b></kbd> to advance a finale text screen would carry over to the following map, and cause the player to use a switch if they started directly in front of one (such as is the case for *MAP07: Dead Simple* in *DOOM II: Hell On Earth*).
* The flashing key in the HUD will now be updated if the player tries opening another locked door that requires a different key to the one currently flashing.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, May 13, 2014

### DOOM Retro v1.4.3

* The screen size can now be adjusted correctly in the options menu when not in a game.
* The size of the pixels when the graphic detail is “LOW” can now be changed by editing the `pixelwidth` and `pixelheight` settings in `doomretro.cfg`. Both are set to a default of `2`.
* A bug has been fixed whereby pressing <kbd><b>ENTER</b></kbd> to save a game in an empty savegame slot wouldn’t clear that slot and replace it with the name of the current map.
* Barrels are no longer randomly mirrored when they explode.
* Some settings in `doomretro.cfg` are now sorted differently.
* A bug has been fixed whereby graphic anomalies could appear in some parts of some maps.
* The correct graphics will now be used for the titles of the load and save game menus, if custom graphics are present in a PWAD.
* If there are custom sprites present in a PWAD, they will now animate correctly.
* `nerve.wad` is now correctly recognized by the `-file` command-line parameter.
* `-pwad` is now allowed as an alternate to the `-file` command-line parameter.
* Checking what things are on a floor that raises or lowers is now more accurate.
* Optimizations have been made when drawing the sky.
* The sky is now flipped vertically when repeated, as originally intended.
* A bug has been fixed whereby randomly colored pixels would appear along the very top of the screen when drawing the sky in some instances.
* There are now two settings in `doomretro.cfg` that control how many blood splats appear. `bloodsplats_total` (previously known as just `bloodsplats`) controls the total number of blood splats spawned in a level. It’s default is still `unlimited`. `bloodsplats_visible` is a new setting that specifies the number of blood splats that will appear on the screen at any one time. It’s default is `1024`.
* A bug has been fixed whereby use of the `-episode`, `-skill` and `-warp` command-line parameters would cause the game to silently crash.
* Gamepads now work correctly.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, May 6, 2014

### DOOM Retro v1.4.2

* Several more limits have been removed, allowing larger and more detailed maps to be loaded without crashing.
* *DOOM Retro* will now try to fix some common map errors before loading a map.
* Optimizations have been made to the loading of large levels, and the handling of gamepads.
* The smoke trails of the player’s and cyberdemon’s rockets can now be disabled by setting `smoketrails` to `false` in `doomretro.cfg`. It is `true` by default.
* The limit on the size of savegames has been removed, allowing the game to be saved on larger maps without exiting with an error.
* Several changes have been made to the HUD:
  * The HUD is now displayed in non-widescreen modes.
  * The drop shadows of the numbers in the HUD are now slightly lighter.
  * The HUD has been shifted downwards slightly.
  * Values in the HUD will now start flashing at 20 (rather than 10), and the speed at which they flash will increase the closer they get to 0.
  * When the player is invulnerable, only the red part of the medikit turns gold, and not the health value.
  * Keys are no longer seen to disappear from the HUD before wiping the screen when using the `IDCLEVxy` cheat to warp to a new map.
* The positions of blood splats are now randomized better.
* A memory leak has been fixed when loading a savegame.
* A bug has been fixed where the gamma correction level was being reset to 0.75 each time *DOOM Retro* was loaded, regardless of what it was previously set to.
* A bug has been fixed whereby some users were experiencing jerky mouse movement.
* A flashing HOM (“Hall of Mirrors”) indicator has been implemented. The screen will flash red in place of missing textures, but not when the player has enabled “no clipping mode” using the `IDCLIP` or `IDSPISPOPD` cheats.
* When saving a game, the savegame description will only be updated to the current map name if it hasn’t been changed to something other than map name previously.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, April 29, 2014

### DOOM Retro v1.4.1

* A bug has been fixed whereby the sound would become disabled if adjusting the volume through the menu or pausing and then unpausing the game.
* The visplane limit has been removed, allowing for more detailed levels to run without crashing.
* A sound will now be heard if a wrong selection is made in the WAD launcher and it needs to reopen.
* When a heavy weapon dude is killed, their corpse is no longer randomly mirrored.
* Projectiles will now pass through map decorations like they do in *Vanilla DOOM*. (Please note that this particular change means savegames from previous versions of *DOOM Retro* won’t work with *DOOM Retro v1.4.1*.)
* If music can’t be loaded for a particular map, that map will still load without music rather than the game exiting with an error.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, April 25, 2014

### DOOM Retro v1.4

* Several optimizations have been made that improve the overall performance of *DOOM Retro*.
* When *DOOM Retro* is opened for the first time the following message is now displayed:
  > Thank you for downloading DOOM RETRO!
  >
  > Please note that, as with all DOOM source ports, no actual map data is distributed with DOOM RETRO.
  >
  > In the dialog box that follows, please navigate to where an official release of DOOM or DOOM II has been installed and select a “WAD file” that DOOM RETRO requires (such as DOOM.WAD or DOOM2.WAD).
* There are now 2 levels of graphic detail, “HIGH” and “LOW”, adjustable through either the options menu or by pressing the <kbd><b>F5</b></kbd> key. The “HIGH” level has the same graphic detail as in previous versions of *DOOM Retro*. It has an effective resolution of 640×400 (or 640×336 in widescreen) and is the default. The “LOW” level is new, and has an effective resolution of 320×200 (the resolution used in *Vanilla DOOM*).
* When the title screen is displayed, it will now alternate with the credits screen.
* The gray text in the status bar is now twice the resolution when the graphic detail is set to “HIGH”.
* A bug has been fixed whereby the sounds of revenant and cyberdemon projectiles would become corrupted in some instances.
* The file `default.cfg` has been renamed to `doomretro.cfg`.
* The background when a menu is displayed or the game is paused is now in grayscale, as well as being blurred and darkened. This may be disabled by changing `grayscale` to `false` in `doomretro.cfg`.
* The menu background’s blur is now calculated differently, and should resolve the performance issues some users were experiencing.
* Minor corrections have been made to the drop shadows and kerning of text in the menus.
* Menus are now always centered vertically.
* Many changes have been made so that custom graphics in PWADs are now handled better, when previously they may have either ignored completely or misaligned.
* When changing the music using the `IDMUS` cheat, the name of the music will be displayed.
* When changing the map using the `IDCLEVxy` cheat, the name of the map will be displayed.
* When using the `IDBEHOLDx` cheats, the message displayed will indicate if the power-up is being toggled on or off.
* In `doomretro.cfg`, `menublur` has been renamed to just `blur`, `show_messages` to `messages`, `usegamma` to `gammalevel`, and `screenblocks` to `screensize`.
* When the player’s health, ammo or armor drops to 10 or less, those values will start flashing in the widescreen HUD as a warning.
* When the player is invulnerable (either by having picked up an invulnerability power-up or using the `IDDQD` cheat), their health will be displayed in gold in the widescreen HUD. This corresponds with the gold eyes of the player’s face in the status bar.
* The widescreen HUD now indicates when the player is invulnerable.
* There are now 31 different gamma correction levels between 0.50 and 2.0 inclusive, in increments of 0.05. Gamma correction level 0.75 is still the default.
* The first press of the <kbd><b>F11</b></kbd> key will now display the current gamma correction level. Further presses of the key before the message disappears will then increase the level (or decrease the level if the <kbd><b>SHIFT</b></kbd> key is held down).
* Autorepeat is now allowed for the <kbd><b>F11</b></kbd> key.
* The gamma correction level is now saved the moment it is changed.
* The weapons dropped by shotgun guys and heavy weapon dudes when they die are now mirrored horizontally at random, independent of their corpses.
* A bug has been fixed whereby the player was unable to pick up stimpacks or medikits in the *BFG Editions* of *DOOM* or *DOOM II*.
* There is now slightly more blood.
* Because of some significant improvements to the drawing of blood splats, the number of blood splats that may be in a map is now unlimited. The `bloodsplats` setting in `doomretro.cfg` may still be changed from unlimited to a value between `0` and `32768` inclusive.
* Blood splats are now mirrored horizontally at random for some additional variation.
* The blood and blood splats from spectres, as well as the player when they have the partial invisibility power-up, now appear with the same “fuzz effect”.
* Blood splats are now drawn first and no longer overlap other sprites, making them appear closer to the ground.
* When the mouse sensitivity is set to 0, the mouse is disabled. (And similarly, when a gamepad is in use, it is also disabled.)
* The translucency of blue and green blood has been reduced slightly.
* `doomretro.wad` no longer appears in the WAD launcher.
* Any PWADs selected with `doom1.wad` in the WAD launcher will now be ignored rather than displaying an error.
* `nerve.wad` can now be selected without `doom2.wad` in the WAD launcher, even when another PWAD is selected with it, and `doom2.wad` will be looked for automatically.
* A bug has been fixed whereby the keys the player had previously found weren’t being restored when loading a savegame.
* The location of GUS patches can now be specified using `timidity_cfg_path` in `doomretro.cfg`.
* Rotate mode is now enabled by default in the automap.
* Some minor changes have been made to the help screen.
* Screenshots are now saved as `Untitled.bmp` if taken while in the help screen.
* Messages will now timeout when the help screen is displayed.
* The lighting in maps is now calculated differently.
* The small ‘3’ and ‘8’ digits used for marks in the automap, and the small ‘3’ digit in the status bar, have been altered slightly.
* There is now a chance that the super shotgun may gib a monster when fired at point blank range. (This idea has been taken from Fabian Greffrath’s *Crispy DOOM*.)

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Tuesday, April 1, 2014

### DOOM Retro v1.3

* An extensive number of optimizations have been made that improve the overall performance of *DOOM Retro*.
* A heads-up display (HUD) is now shown in widescreen mode.
* Each element of the HUD is slightly translucent.
* An additional press of the <kbd><b>+</b></kbd> key, or move of the “SCREEN SIZE” slider to the right in the options menu, will hide the HUD.
* The HUD isn’t displayed while the player is dead.
* If the player has the berserk power-up, and has his fist selected, the berserk power-up will replace the medikit in the HUD.
* The keys that the player has picked up are displayed in the order they were found, from right to left.
* If the player attempts to open a door they don’t have the key for, that key will flash in the HUD.
* If the player has no armor, the keys are displayed along the right side of the screen.
* The type of armor the player has (either green or blue armor) is displayed.
* The background is now blurred when in a menu or the game is paused. This effect may be disabled by changing the `menublur` setting in `default.cfg` to false.
* The green blood of hell knights and barons of hell is now slightly darker.
* A bug has been fixed that existed in *Vanilla DOOM* that caused monsters to be able to see the player through walls, or not see the player when they should have, in some instances. Thank you to 倉敷楠花 (Nanka Kurashiki) for bringing this to my attention.
* The <kbd><b>PRINTSCREEN</b></kbd> key and <kbd><b>ALT</b></kbd> + <kbd><b>ENTER</b></kbd> have been added to the help screen.
* The “HELP” title has been added to the help screen in *DOOM II* to be consistent with the other versions of *DOOM*.
* The player’s weapon is no longer displayed in the help screen’s background.
* Switching between widescreen and non-widescreen modes is now much cleaner. The status bar will no longer briefly appear at the bottom of the screen.
* Several bugs have been fixed whereby some IWADs and PWADs would sometimes fail to load from the WAD launcher.
* If the selection made in the WAD launcher is incorrect, it will reopen rather than exit with an error.
* By popular demand, `default.cfg` is now saved in the same folder as `doomretro.exe`. To make it clear where the settings are, a copy of `default.cfg` is now included in the distribution.
* The settings in `default.cfg` are now sorted alphabetically.
* *DOOM Retro* is now less likely to crash if certain settings in `default.cfg` are set incorrectly.
* The `bloodsplats` setting in `default.cfg` is now a number rather than `true` or `false`, and is `1024` by default.
* A bug has been fixed whereby over 700 different level-specific fixes (replacing missing or incorrect textures, moving stuck objects, etc.) weren’t being applied.
* Tweaks have been made to the animations of zombiemen, shotgun guys and mancubi.
* Screenshots are now saved as a 256-color *Windows* BMP, reducing their size in kilobytes by more than 66%.
* Pillarboxes are no longer saved in screenshots.
* The <kbd><b>PRINTSCREEN</b></kbd> key now no longer saves the screen to the clipboard when taking a screenshot.
* Rotation in the automap is now more accurate.
* A bug has been fixed whereby the crosshair could still decelerate from panning while the menu was displayed.
* The red crosses in stimpacks are now darker to be consistent with medikits.
* If a sprite is replaced with a custom sprite in a PWAD, any translucency will be removed and custom offsets won’t be applied.
* The width of pickups are now calculated differently such that they are less likely to be suspended in midair when close to a change in height.
* The `TEKWALL1` texture (for example, as used on the green armor platform in *E1M1: Hangar*) is now displayed the same way as in *Vanilla DOOM*.
* *Windows* accessibility shortcut keys are disabled during the game.
* The <kbd><b>WINDOWS</b></kbd> key is now also disabled when in a window.
* The pistol sound is now used when toggling messages in the options menu using the <kbd><b>&larr;</b></kbd> and <kbd><b>&rarr;</b></kbd> cursor keys.
* Changes have been made to how mouse sensitivity is calculated such that it is now exactly the same as *Chocolate DOOM*.
* The default mouse sensitivity has been increased.
* Blood splats no longer appear on `RROCK0x` animated flats.
* A bug has been fixed whereby if the “reject matrix” in a PWAD is empty, it will create an overflow and cause monsters to behave strangely. Thank you to jeff-d on the *Doomworld* forums for providing a solution to this.
* The berserk power-up may now be toggled off using `IDBEHOLDS` cheat.
* A bug has been fixed whereby the lost soul wouldn’t rotate correctly in *DOOM II’s* cast sequence.
* Translucency may be disabled by setting translucency setting in `default.cfg` to false.
* The fuzz effect is now applied to the muzzle flash of the player’s weapon when they have the partial invisibility power-up.
* A bug has been fixed whereby some floors weren’t rising or lowering when they should. Thank you to Jon Krazov for bringing this to my attention.
* `key_prevweapon` and `key_nextweapon` will no longer work when in a menu.
* Savegame descriptions are no longer updated to the current map’s name when the player saves a game if they have previously edited it.
* The correct skill level is now saved in savegames.
* The <kbd><b>ENTER</b></kbd> key on the numeric keypad can now be used wherever the main <kbd><b>ENTER</b></kbd> key can be used.
* `DMENUPIC` is now used on the intermission screen in *DOOM II (BFG Edition)*.
* There are no longer any overlapping drop shadows in the menus.
* Keys are now positioned correctly in the status bar.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Saturday, March 1, 2014

### DOOM Retro v1.2.1

* *DOOM Retro* is now compiled using *Microsoft Visual Studio Express 2013 for Windows Desktop*.
* *DOOM Retro* is now distributed with version 1.2.14 of `SDL.dll` and version 1.2.12 of `SDL_mixer.dll`.
* `doomretro.exe` now has a new icon.
* If no IWAD file is specified using the `–iwad` command-line parameter, a standard *Windows* dialog box entitled “Where’s All the Data?” will now appear where one IWAD, and optionally one or more PWADs, can be selected.
* *DOOM Retro* is now considerably more stable. The game will no longer crash when a spectre is on the screen in some instances.
* Many internal optimizations have been made.
* A bug has been fixed that was present in *Vanilla DOOM* whereby bullets would pass through monsters in some instances.
* Much greater mouse sensitivity can now be selected in the options menu.
* Minor visual tweaks have been made to the status bar.
* The game will no longer switch to widescreen mode in the options menu if the screen slider is moved all the way to the right and no game is being played.
* The <kbd><b>+</b></kbd> and <kbd><b>–</b></kbd> keys, as well as moving left and right on a gamepad, can no longer be used to toggle messages on and off in the options menu.
* When `default.cfg` is created for the first time, the keyboard control variables will now be saved as their actual character values rather than their scan codes.
* Blood splats are now left on the ground wherever blood falls. (They may be disabled by setting `bloodsplats` to `false` in `default.cfg`.)
* The fuzz effect of spectres now looks better while the game is paused or a menu is displayed.
* A bug has been fixed whereby the chainsaw could not be selected by the player unless they also had a berserk power-up.
* The screen will now be wiped at the same speed in widescreen mode.
* If a value is out of range in `default.cfg`, the default for that value will be used rather than the closest valid value.
* The `usegamma` value is now checked that it is in range when the game starts.
* When the player stands where there is a change in height (either on the floor or ceiling), that edge is now drawn more accurately.
* When in a confined area, pain elementals no longer try to spawn lost souls in the wrong places only for them to explode straight away.
* When lost souls are killed, they now explode on the spot, rather than their explosion sometimes drifting upwards.
* When pain elementals are killed, their explosion is now centered better.
* A bug has been fixed whereby messages weren’t always being cleared before taking a screenshot.
* The state of flickering lights, active switches and moving platforms are now saved in savegames. This means that savegames from previous versions of *DOOM Retro* will no longer work.
* When a monster is killed, there is a better chance of its corpse being mirrored horizontally if the corpse of the last monster to be killed wasn’t mirrored.
* When more than one monster is killed at exactly the same time, there is now a chance that they will fall randomly out of sync.
* Settings are now saved to `default.cfg` the moment they change, rather than when quitting the game, so if the game crashes or exits with an error, those settings will be restored.
* The player’s weapon now isn’t as distorted at reduced screen sizes.
* A bug has been fixed whereby the muzzle of the super shotgun was translucent in some instances.
* 33% alpha translucency rather additive translucency is now used for the soulsphere, megasphere, invincibility and partial invisibility power-ups.
* The <kbd><b>+</b></kbd> and <kbd><b>–</b></kbd> keys can no longer be used while the help screen is displayed.
* In those levels that require one or more monsters to be killed for a sector to move to complete the level, if the `–nomonsters` command-line parameter is specified, those sectors will now automatically move.
* Replicating what happens in *Heretic* and *Hexen*, the remaining monsters in the level will turn on each other once the player has been killed. The player will face their killer when they die, but unlike those games, their view won’t continue to follow their killer around.
* A bug has been fixed whereby *DOOM’s* episode menu would be displayed when pressing the <kbd><b>ESC</b></kbd> key on *DOOM II’s* skill level menu.
* The `IDCHOPPERS` cheat will now be canceled (by removing the invulnerability power-up and the chainsaw) when the player switches to or picks up a weapon other than the chainsaw.
* Many monsters are now positioned and animate better in *DOOM II’s* cast sequence.
* Monsters now can’t be rotated in the cast sequence until they are actually on the screen.
* Z-coordinates are now taken into account when telefragging.
* Arch-viles no longer resurrect monsters such that they become stuck in other monsters, or under doors.
* A bug has been fixed whereby the game may crash when an arch-vile resurrects a monster after a savegame has been loaded.
* The boss in *MAP30: Icon Of Sin* at the end of *DOOM II: Hell On Earth* will now still make its alert sound when the level starts if `–nomonsters` is specified on the command-line.
* A “rotate mode” is now available in the automap. Toggled on and off using the <kbd><b>R</b></kbd> key, it will dynamically orientate the level such that the player is always pointing towards the top of the screen. Rotate mode is off by default, and whether it is on or off is remembered between games.
* Only one instance of `doomretro.exe` can be run at a time.
* The player arrow in the automap will now be translucent while the player has a partial invisibility power-up.
* In *DOOM II (BFG Edition)*, since `TITLEPIC` isn’t present in the IWAD, the otherwise unused `DMENUPIC` is now used instead of `INTERPIC`.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)

###### Friday, January 24, 2014

### DOOM Retro v1.1

* The source code is no longer distributed with *DOOM Retro* itself, and has instead been made separately available in a [*GitHub* repository](http://github.com/bradharding/doomretro). Please visit this site to follow the latest daily developments, and report any [issues](http://github.com/bradharding/doomretro/issues) that you may encounter.
* The correct and complete version information is now displayed when right-clicking on `doomretro.exe` and selecting “Properties”.
* The `–file` command-line parameter may no longer be used with *DOOM Shareware’s* WAD file, `doom1.wad`.
* If a `default.cfg` file is present in the game folder, it will now be used in preference to the `default.cfg` saved in `AppData\Local\DOOM RETRO` for the current *Windows* user.
* While still maintaining backwards compatibility, the values that may be specified in the `default.cfg` file are now much more readable, and easier to edit manually.
* Gamepad controls can now be customized by editing the `default.cfg` file.
* A bug has been fixed whereby the game wouldn’t be rendered correctly for displays with an aspect ratio less than 4:3 (that is, displays that aren’t widescreen).
* In fullscreen mode on a widescreen display, increasing the screen size with the <kbd><b>+</b></kbd> key to the maximum will now show a widescreen mode without the status bar, and without any of the horizontal stretching prevalent in many other source ports. *DOOM Retro* will revert to the standard 4:3 aspect ratio when on the title, intermission, finale and help screens.
* A bug has been fixed whereby parts of *MAP01: Entryway* and *MAP02: Underhalls* in *DOOM II: Hell On Earth* would become corrupted when using older versions of `doom2.wad`. Two barrels and a shotgun guy were missing from MAP02 as well. (Credit goes to Jon Krazov for his assistance.)
* For a majority of translucent or partially translucent objects, their translucency is now calculated using additive blending rather than alpha blending, resulting in them appearing considerably brighter.
* The blue lights in tall and short techno floor lamps (`MT_MISC29` and `MT_MISC30`) are now translucent.
* The red and green lights in all switches, as well as the exit signs and many computer terminals that appear in most levels are now consistently bright regardless of the surrounding light levels, and the distance from the player (that is, they are “fullbright”).
* Some minor cosmetic changes have been made to the status bar.
* The corpses of cyberdemons are no longer flipped horizontally at random.
* When the player ends a level by flicking a switch, that switch will now turn on before the screen is wiped.
* If the player has both the invulnerability and the light amplification visor power-ups, and the invulnerability power-up runs out first, the screen will now flash correctly between the inverted grayscale palette and the “fullbright” palette.
* If the player has both a chainsaw and a berserk power-up, pressing the <kbd><b>1</b></kbd> key will now directly switch to either the chainsaw or the fist, depending on which weapon was selected last, rather than always switching to the chainsaw. This selection is also remembered when saving a game.
* If the <kbd><b>SHIFT</b></kbd> key is held down when the <kbd><b>CAPSLOCK</b></kbd> key is on (or vice-versa), the player will walk instead of run, as originally intended.
* Monsters can no longer pass through tall level decorations.
* A bug has been fixed whereby it took approximately twice as many rockets to kill the boss in *MAP30: Icon Of Sin* at the end of *DOOM II: Hell On Earth*.
* Like what can be done at the end of *DOOM 64*, each monster can now be rotated using the <kbd><b>&larr;</b></kbd> and <kbd><b>&rarr;</b></kbd> cursor keys during the cast sequence in *DOOM II*.
* The lost soul in the cast sequence in *DOOM II* is now partially translucent.
* The explosions when the lost soul and the pain elemental die in the cast sequence in *DOOM II* are now translucent.
* A bug has been fixed whereby the cast sequence in *DOOM II* could not be advanced by pressing the <kbd><b>CTRL</b></kbd> key.
* The help screen has been updated to include the new controls for the keyboard, mouse and gamepad, and fixing several inconsistencies. (Credit goes to Robin “FrightNight” Reisinger for his assistance.)
* The text on the help screen now has drop shadows.
* The help screen’s background is now a low resolution snapshot of the game screen with a dark blue tint.
* The controls for selecting the plasma rifle and BFG 9000 are no longer present on the help screen in *DOOM Shareware*.
* When using a PWAD, screenshot filenames will now be of the format `ExMy.bmp` or `MAPxx.bmp` rather than incorrectly use the name of the map this map replaces in the main IWAD.
* When a screenshot is taken using the <kbd><b>PRINTSCREEN</b></kbd> key, any messages are now cleared from the top of the screen first.
* A bug has been fixed whereby if a game is saved while a platform is moving, it could potentially cause the game to crash when that savegame is loaded.
* When using the `IDCLEVxy` cheat, keycards and skull keys are no longer removed from the status bar before the screen is wiped.
* The correct message is now displayed when entering the `IDKFA` cheat.

![](https://github.com/bradharding/www.doomretro.com/raw/master/wiki/bigdivider.png)
