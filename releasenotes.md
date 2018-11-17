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

---

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
* If the `+alwaysrun` action is bound to the <kbd>CAPSLOCK</kbd> key, then that key will now be toggled on or off as necessary when *DOOM Retro’s* window gains or loses focus, and not just when it is closed.
* Any screen shake or palette effect will now be canceled when pressing <kbd>F7</kbd> to end a game, or <kbd>F9</kbd> to quicksave a game.
* The <kbd>F12</kbd> key can now be bound to an action using the `bind` CCMD.
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
* Savegames will now be saved in the correct folder when playing [*Freedoom: Phase 1*](http://freedoom.github.io/) or [*Freedoom: Phase 2*](http://freedoom.github.io/).
* The behavior of the `-savedir` command-line parameter has changed. Savegames will now be placed directly in the folder specified, rather than in a subfolder based on the name of the WAD loaded.
* `-save` may be now be used as an alternative to `-savedir` on the command-line.
* A bug has been fixed whereby the player’s path in the automap wasn’t being shown correctly if both the `am_path` CVAR and no clipping mode were on.
* If the player has more than one power-up, the countdown bar in the alternate widescreen HUD will now always show the power-up that will run out first.
* If an SFX lump in a PWAD is in an unrecognized format, the original lump in the IWAD will be played instead.
* The `+use` action can no longer be used if the `autouse` CVAR is `on`.
* Items dropped by monsters when they are killed will now be rendered correctly if dropped on a moving platform and the `vid_capfps` CVAR is a value other than `35`.

---

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

---

###### Friday, August 24, 2018

### DOOM Retro v2.7.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Slightly more blood splats are now spawned when blood hits the floor.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* A bug has been fixed whereby a crash could occur when loading a savegame in some instances.
* The number in the leftmost column of the output to the `thinglist` CCMD is now the actual ID of each thing that is spawned in the current map.
* Further improvements have been made to the support of `DEHACKED` lumps.
* Pressing <kbd>ALT</kbd> + <kbd>F4</kbd> will now instantly quit *DOOM Retro* as originally intended.
* A bug has been fixed whereby the `+zoomin` and `+zoomout` actions couldn’t be rebound from their default <kbd>+</kbd> and <kbd>&ndash;</kbd> keys using the `bind` CCMD.
* Mouse acceleration can now be disabled using the new `m_acceleration` CVAR. It is `on` by default and `off` when vanilla mode is enabled.
* Movement of a gamepad's thumbsticks can now be either analog or digital using the new `gp_analog` CVAR. It is `on` by default and `off` when vanilla mode is enabled.
* The number of thumbsticks used on a gamepad can now be set using the new `gp_thumbsticks` CVAR. If set to `2` (the default), the left thumbstick is used to strafe left/right and move forward/back, and the right thumbstick is used to turn left/right (and look up/down if the `mouselook` CVAR is `on`). If set to `1` (which it is when vanilla mode is enabled), one thumbstick is used to turn left/right and move forward/back.
* A bug has been fixed whereby monsters could be spawned at an incorrect height in some rare instances.
* Some translucency effects have been improved.

---

###### Saturday, August 4, 2018

### DOOM Retro v2.7.1

* The player will now move correctly when using a gamepad.
* The `r_diskicon` CVAR will now be turned `on` when vanilla mode is enabled.
* Minor changes have been made to text that is output to the console.
* The `restartmap` CCMD will now restart the correct map when playing *E1M4B: Phobos Mission Control* or *E1M8B: Tech Gone Bad*.
* The help screen’s background when pressing the <kbd>F1</kbd> key is now displayed better when using a custom colormap from a PWAD.
* If the super shotgun was selected by the player more recently than the shotgun, it will now be selected when pressing the <kbd>3</kbd> key, and vice versa.
* Improvements have been made to the gradual lighting effect under doors and crushing ceilings.

---

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
* <kbd>SPACE</kbd> can now be pressed to respawn the player, as well as advance the intermission and finale screens, even if the key isn’t bound to the `+use` action.
* The direction the player is looking is no longer recentered vertically when they go through a teleport and the `mouselook` CVAR is `on`.
* An obituary is now displayed when the player is crushed to death by a moving ceiling and the `con_obituaries` CVAR is `on`.
* Whether sound effects are played in mono or stereo can now be changed using the new `s_stereo` CVAR. It is `on` by default and when vanilla mode is enabled.

---

###### Saturday, March 31, 2018

### DOOM Retro v2.6.9

* The targets of monsters will now be restored correctly when loading a savegame.
* The player’s view will no longer go past the floor or ceiling in some rare instances.
* A bug has been fixed whereby the player would fire their weapon when the game was unpaused using the <kbd>PAUSE</kbd> key.

---

###### Thursday, March 29, 2018

### DOOM Retro v2.6.8

* *DOOM Retro* now uses [*SDL v2.0.8*](http://libsdl.org) and [*SDL_image v2.0.3*](http://libsdl.org/SDL_image).
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Improvements have been made to how both sprites and the shadows they cast are rendered.
* Minor changes have been made to text that is output to the console.
* Player messages can no longer be present in screenshots taken using the <kbd>PRINTSCREEN</kbd> key.
* Spectres and the shadows they cast are now displayed correctly when the `r_textures` CVAR is `off`.
* Further improvements have been made to the support of `DEHACKED` and `MAPINFO` lumps.
* The player’s face is no longer updated in either the status bar or the default widescreen HUD when freeze mode is on.
* The screen is now rendered correctly while the player has an invulnerability power-up and the `r_textures` CVAR is `off`.
* A bug has been fixed whereby some map-specific fixes enabled using the `r_fixmaperrors` CVAR weren’t being applied.
* Hanging corpses no longer bob up and down if above liquid.
* Corpses can no longer trigger line specials when sliding over them.
* Fixing a bug present in *Vanilla DOOM*, monsters will no longer momentarily freeze when trying to open certain locked doors.
* The correct map names will now be displayed when playing [*Freedoom*](http://freedoom.github.io/).
* An error will no longer occur when trying to load [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
* The screen will now fade to black upon quitting from either [*Freedoom*](http://freedoom.github.io/) or [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/).
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

---

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

---

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

---

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

---

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

---

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
* The `-shotdir` command-line parameter can now be used to specify the folder that screenshots will be saved in when the <kbd>PRINTSCREEN</kbd> key is pressed.
* Objects will no longer be lit incorrectly in some rare instances.
* The shadows cast by monsters will now be displayed correctly in areas with a custom colormap.
* The shadows cast by spectres will now be displayed correctly when the `r_shadows_translucency` CVAR is `off`.
* Using the `nomonsters` CCMD will now instantly remove all monsters in the current map.
* The brightmaps for several wall textures are now fixed.
* A bug present in *Vanilla DOOM* has been fixed whereby [Mancubi projectiles would sometimes pass through walls](https://doomwiki.org/wiki/Mancubus_fireball_clipping).

---

###### Saturday, December 16, 2017

### DOOM Retro v2.6.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The effects of changing the `gp_swapthumbsticks` CVAR are now immediate.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* A bug has been fixed whereby changing the `vid_screenresolution` CVAR to a value other than `desktop` wouldn’t change the screen resolution.
* Pressing <kbd>ALT</kbd> + <kbd>ENTER</kbd> to toggle between fullscreen and a window will now work when the `vid_screenresolution` CVAR is a value other than `desktop`.
* Both player messages and the map title in the automap are no longer truncated in the middle of the screen in some instances.
* Sprites that are replaced in PWADs will now be offset correctly.

---

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

---

###### Saturday, November 25, 2017

### DOOM Retro v2.6

* *DOOM Retro* now uses [*SDL v2.0.7*](http://libsdl.org), [*SDL_mixer v2.0.2*](http://libsdl.org/SDL_mixer) and [*SDL_image v2.0.2*](http://libsdl.org/SDL_image).
* Extensive optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The format of savegames has changed, breaking compatibility with previous versions of *DOOM Retro*.
* A bug has been fixed whereby the help screen accessed using the <kbd>F1</kbd> key had a solid blue background.
* Minor changes have been made to text that is output to the console.
* Further improvements have been made to the console’s autocomplete feature.
* The player’s field of view can now be changed using the new `r_fov` CVAR. This CVAR can be a value between `45` and `135`, and is `90` by default and when vanilla mode is enabled.
* Using the `vanilla` CCMD in an alias will now work correctly.
* Strings of commands, separated by semi-colons, can now be entered directly in the console.
* Most actions can now be entered directly in the console.
* The `bind` CCMD can now be used to bind a string of commands to a control. For example, to press the <kbd>V</kbd> key to enable vanilla mode without lowering the graphic detail, enter `bind 'v' "vanilla; r_detail high"` in the console.
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
* A crash will no longer occur when trying to switch between fullscreen and a window by pressing <kbd>ALT</kbd> + <kbd>ENTER</kbd> while on the title screen.
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

---

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

---

###### Saturday, September 30, 2017

### DOOM Retro v2.5.6

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The effects of changing the `r_corpses_color` CVAR are now immediate.
* A bug has been fixed whereby the player would restart the current map immediately upon death in some rare instances.
* <kbd>TAB</kbd> and <kbd>SHIFT</kbd> + <kbd>TAB</kbd> may now also be used in the console to autocomplete the parameters of most CCMDs and CVARs.
* When entering an alias previously created using the `alias` CCMD, the alias itself will now be added to the console’s input history rather than the contents of the alias.
* Minor changes have been made to some of the text in the console.
* The console’s background will now be updated if opened on the credits screen.
* Changes have been made to the status bar’s background when the `r_detail` CVAR is `high`.
* The maximum value that the `s_channels` CVAR can be set to is now `64`.
* IWADs and PWADs can now be specified on the command-line without a `.wad` extension.
* The slight current enabled using the `r_liquid_current` CVAR will no longer be applied to liquid sectors that also have a *BOOM*-compatible scrolling effect.
* When using the `idclip` cheat, `idclip` will now be displayed in the console rather than `idspispopd`.
* A crash will no longer occur when trying to display the spectre in *DOOM II’s* cast sequence.

---

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

---

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
* When pressing the <kbd>PRINTSCREEN</kbd> key while the `am_external` CVAR is `on`, a screenshot of both screens will now be taken rather than two screenshots of the same screen.
* The `vid_capfps`, `vid_scalefilter` and `vid_vsync` CVARs will now affect the external automap when the `am_external` CVAR is `on`.
* Obituaries in the console now correctly reflect when the player or a monster is telefragged.
* Pressing <kbd>CTRL</kbd> + <kbd>&uarr;</kbd>/<kbd>&darr;</kbd> can now be used as well as <kbd>PGUP</kbd>/<kbd>PGDN</kbd> to scroll the output in the console up and down.
* The `r_gamma` CVAR can now correctly be set to `2.0` in the console and at startup.
* When the `vid_capfps` CVAR is a value other than `35`, rockets and plasma rifle and BFG-9000 projectiles are now slightly further away from the player when fired.
* Further improvements have been made to lowering the player’s view in liquid sectors when the `r_liquid_lowerview` CVAR is `on`.
* Reducing the `health` CVAR will now work correctly when playing the *I’m too young to die* skill level.
* If no IWAD is specified, *DOOM Retro* will now also check for an installation of *DOOM 3: BFG Edition* purchased through [*GOG.com*](https://www.gog.com/game/doom_3_bfg_edition).

---

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

---

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
* Frames from *DOOM’s* rocket launcher are no longer shown when firing the missile launcher in [*Freedoom*](http://freedoom.github.io/).

---

###### Wednesday, July 5, 2017

### DOOM Retro v2.5.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* If no IWAD is found when loading a PWAD using the WAD launcher (either in the same folder as the PWAD or the folder specified by the `iwadfolder` CVAR), several common installation folders will now be checked.
* The introductory message is no longer displayed when opening *DOOM Retro* for the first time.
* Minor changes have been made to text that is output to the console.
* A bug has been fixed whereby a crash could occur when exiting a map in some instances.

---

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
  * [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960),
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

---

###### Friday, April 7, 2017

### DOOM Retro v2.4.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Sprites taller than 255 pixels are now supported.
* The map title in the automap is now positioned correctly when the `r_messagescale` CVAR is `small`.
* Frames from *DOOM’s* rocket launcher are no longer shown when firing the missile launcher in [*Freedoom*](http://freedoom.github.io/).
* Entering the `IDMYPOS` cheat will no longer cause a crash.
* Pressing the <kbd>DEL</kbd> key when in the save or load game menus will now delete the currently selected savegame.
* Minor changes have been made to text that is output to the console.
* The inverted gray color palette is now applied to the sky when the player has the invulnerability power-up, as originally intended.
* A bug has been fixed whereby blood splats would no longer be spawned after loading a savegame in some instances. Please note that because of this, savegames created with previous versions of *DOOM Retro* are not compatible with this version.
* Another 29 map-specific fixes, enabled using the `r_fixmaperrors` CVAR, have been applied to maps in the `doom.wad`, `doom2.wad` and `plutonia.wad` IWADs.
* The text caret shown when entering a savegame description in the save game menu is now always a vertical line using the dominant color of the character set. (Previously, the `STCFN121` lump was used. In the *DOOM* and *DOOM II* IWADs this lump is a vertical pipe character, but in some PWADs it is replaced with a “Y” character.)
* The sound of the chainsaw will no longer cut off sounds made by the player.
* A bug has been fixed whereby translucent sprites would become less bright when the player had the light amplification visor power-up.
* A bug present in *Vanilla DOOM* has been fixed whereby homing rockets fired by revenants would randomly become non-homing, and vice versa, when loading a savegame or when pausing then unpausing a game.

---

###### Monday, March 27, 2017

### DOOM Retro v2.4.4

* A bug has been fixed whereby a crash would often occur when the player died and the `vid_widescreen` CVAR was `off`.
* The value of the `r_messagescale` CVAR is now displayed correctly in `doomretro.cfg`.
* Over 200 additional map-specific fixes, enabled using the `r_fixmaperrors` CVAR, have been applied to maps in `doom.wad`.

---

###### Sunday, March 26, 2017

### DOOM Retro v2.4.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The <kbd>ENTER</kbd> key may now be used as an alternative to the <kbd>Y</kbd> key when responding to messages requiring a yes/no answer.
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

---

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

---

###### Tuesday, February 28, 2017

### DOOM Retro v2.4.1

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The following changes have been made to accommodate for the release of [*Back To Saturn X E1: Get Out Of My Stations v1.0*](https://www.doomworld.com/vb/post/1721926):
  * Teleports are no longer treated as liquid.
  * If only `btsx_e1a.wad` is opened using the WAD launcher, then `btsx_e1b.wad` is automatically opened as well, and vice-versa.
* The header of WADs specified on the command-line using the `-file` parameter will no longer be checked.
* A bug has been fixed whereby the super shotgun would appear entirely translucent when the player fired it.

---

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

---

###### Friday, January 13, 2017

### DOOM Retro v2.3.9

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* A bug has been fixed whereby multiple files couldn’t be loaded using *DOOM Retro’s* WAD launcher.
* Improvements have been made to translucent wall textures when the `r_dither` CVAR is `on`.
* Savegames no longer become corrupted in some instances. Consequently, savegames created using previous versions of *DOOM Retro* are not compatible with this version.

---

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
* A bug has been fixed whereby the <kbd>,</kbd> key couldn’t be bound nor unbound in the console.
* The bound controls displayed by the `bindlist` CCMD are now enumerated correctly.

---

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

---

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

---

###### Tuesday, November 15, 2016

### DOOM Retro v2.3.5

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to text that is output to the console.
* The console is automatically opened at startup when `-devparm` is specified on the command-line.
* If the `vid_scaleapi` CVAR is `"opengl"` and the version of the available *OpenGL* API is less than v2.1, then it will be changed to `"direct3d"` instead.
* If `-cdrom` is specified on the command-line and the `r_diskicon` CVAR is `on`, the `STCDROM` lump will be used instead of the `STDISK` lump.
* The size of the grid in the automap can now be changed using the `am_gridsize` CVAR. It is `128x128` by default.
* The last menu item to be selected is now remembered when using the <kbd>F2</kbd>, <kbd>F3</kbd> or <kbd>F4</kbd> keys to display a menu.
* Further improvements have been made to make sure objects are lit correctly in all instances.
* The music volume is now properly set at startup.
* The console now opens and closes at a consistent speed, slowing down as it is almost completely opened.
* The title of the currently playing music track, as well as the number of secret sectors, are now displayed in the output of the `mapstats` CCMD.

---

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
  * Using the <kbd>A</kbd> key to strafe left now works.
  * Windows are no longer shattered when using `kill all` in the console.
* The music will now be stopped if *DOOM Retro* crashes.
* A bug has been fixed whereby the music volume and sound effects volume were set incorrectly at startup in some instances.
* The text carets in both the save game menu and the console will no longer be displayed, and the skull in the menu will no longer animate, while the window doesn’t have focus.
* Objects will no longer be lit incorrectly in some rare instances.
* A bug has been fixed whereby certain secrets wouldn’t be counted in some *BOOM*-compatible maps.
* Sectors with multiple effects in some *BOOM*-compatible maps will now behave correctly.
* The weapon keys <kbd>1</kbd> to <kbd>7</kbd> will no longer momentarily fail to work after entering an invalid parameter for the `IDMUSxy` cheat.

---

###### Saturday, October 29, 2016

### DOOM Retro v2.3.3

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* The limited MIDI support in *Windows Vista* and above has now been overcome, allowing the music volume to be adjusted independently of the sound effects volume. To allow this to happen, an additional file called `midiproc.exe` is now included and needs to remain in the same folder as `doomretro.exe`.
* The `s_musicvolume` CVAR is now `66%` by default.
* The `r_diskicon` CVAR is now `off` by default.
* Minor changes have been made to text that is output to the console.
* The console will now fill the entire screen when opened using the <kbd>~</kbd> key on the title screen.
* The scrollbar in the console is now hidden if all the text in the console fits entirely on the screen.
* The extreme edges of both the menu and console backgrounds have been softened slightly.
* *DOOM Retro’s* title and version in the console are now white.
* A bug has been fixed whereby using the `map` CCMD when no game was being played would cause a crash.
* The player will now be thrust away with the correct amount of force when attacked by an Arch-vile, or within the blast radius of a rocket or barrel explosion.
* A time limit for each map can now be set using the `-timer` command-line parameter.

---

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

---

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

---

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
* Toggling “always run” using the <kbd>CAPSLOCK</kbd> key while in the console will no longer inadvertently affect player messages from appearing.
* Many minor changes have been made to text that is output to the console.
* A new `-nomapinfo` command-line parameter has been implemented that will stop any `MAPINFO` lumps from being parsed in PWADs at startup.
* If there is a `MAPINFO` lump present in `nerve.wad` that contains invalid map markers, the PWAD will no longer exit with an error, and a warning will be displayed in the console instead.
* The <kbd>SHIFT</kbd> key will now be ignored when pressing <kbd>Y</kbd> or <kbd>N</kbd> in response to a centered message.
* A bug has been fixed whereby no value would be displayed when entering the `r_hud` CVAR in the console without a value.
* When entering a CVAR in the console without a value, the CVAR’s description, current value and default value will now be displayed.
* The shadows of cyberdemons have been raised slightly.
* The values of CVARs in `doomretro.cfg` now have thousands delimiters.
* Thousands delimiters may now be used when entering values of CVARs in the console.
* Monster spawners are now disabled when using `kill all` in the console.
* All automap controls (pressing the <kbd>G</kbd> key to toggle the grid for instance) may now be used when there’s an external automap, provided they don’t conflict with any other controls.
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
* The digits in the status bar are no longer lowered by 1 pixel in [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960).
* The “Cheated” stat in the `playerstats` CCMD now increases when using some CCMDs and command-line parameters that would be considered cheating.
* The console is now automatically closed when the `ammo`, `armor` and `health` CVARs are changed.
* If the `health` CVAR is changed to a smaller value, the effects of the damage to the player will now be shown.
* If the `ammo`, `armor` and `health` CVARs are changed to a larger value, the screen will now flash.
* The player will now be resurrected if the `health` CVAR is changed in the console when they are dead.
* There is now a read-only `version` CVAR that may be used to determine which version of *DOOM Retro* created a `doomretro.cfg` file.
* The super shotgun will now be displayed correctly when fired in [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens).
* The default gamepad sensitivity (set using the `gp_senstivity` CVAR) has been increased from `48` to `64`.
* The `+forward2`, `+back2`, `+strafeleft2`, `+straferight2` and `+use2` actions have been removed. The controls that were bound to these actions are now bound to `+forward`, `+back`, `+strafeleft`, `+straferight` and `+use`, respectively.
* The right thumbstick on gamepads is now bound to the `+use` action and may be pressed as an alternative to the <kbd>A</kbd> button to open doors, use switches, etc.
* A bug has been fixed whereby certain player stats were being reset to `0` at startup.
* The effects of the `IDDT` cheat are now removed from the automap when the player changes levels.
* The shaking of the screen when the player is injured and the `r_shakescreen` CVAR is `on` has been improved slightly.
* A bug has been fixed whereby firing the chaingun would increase the “Shots Fired” stat by 1, but would increase the “Shots Hit” stat by 2 if the shot successfully hit a monster.
* If the player has the invulnerability power-up when using `kill player` in the console, the inverted screen effect will now be removed.
* The map title in the automap is now positioned better when using a taller character set from a PWAD (such as [*Ancient Aliens*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/aaliens)).
* The folder where savegames are saved and loaded can now be specified using the `-savedir` command-line parameter.
* The suicide bombers in [*Valiant*](https://www.doomworld.com/idgames/levels/doom2/Ports/megawads/valiant) will now explode as intended.
* If a `TITLEPIC` lump exists in a PWAD, and there is no `CREDIT` lump to accompany it, then the `CREDIT` lump in the IWAD won’t be displayed during the title sequence.

---

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

---

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

---

###### Tuesday, July 5, 2016

### DOOM Retro v2.2.3

* *DOOM Retro* is now completely portable. The configuration file, `doomretro.cfg`, is now saved in the same folder as the executable, savegames are saved in a `savegames\` folder and screenshots are saved in a `screenshots\` folder.
* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Minor changes have been made to some of the text in the console.
* The width of any selected text is now accounted for when inputting text in the console.
* The number of available displays are now rechecked before creating an external automap if the `am_external` CVAR is enabled through the console.
* An `RMAPINFO` lump will now be used if present in preference to a `MAPINFO` lump to avoid conflicts with other *DOOM* source ports.
* The keyboard shortcuts <kbd>SHIFT</kbd> + <kbd>HOME</kbd> and <kbd>SHIFT</kbd> + <kbd>END</kbd> are now allowed in the console to select all text to the left and right of the caret.
* The `r_berserkintensity` CVAR now accepts a value between `0` and `8` inclusive instead of a percentage. It has a default of `2`.
* The `expansion` CVAR is no longer changed if `nerve.wad` is automatically loaded.
* The player’s view will no longer jump slightly when dead and their corpse is sliding down stairs.
* A `teleport` CCMD has been implemented that allows the player to be teleported to another location in the current map.
* Fuzzy shadows are now applied to any thing whose `SHADOW` bit has been set in a `DEHACKED` lump.
* The map number in the console and automap is now shown in the format `E2Mxy` in [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960).
* The `r_bloodsplats_total` CVAR is now calculated correctly once it reaches `r_bloodsplats_max`.
* A bug has been fixed whereby palette effects from power-ups would remain on the screen after ending a game from the options menu in some instances.
* The value of `r_lowpixelsize` will no longer affect the display of the title screen when the menu is open.
* The <kbd>F5</kbd> key can no longer be used to change the graphic detail when the automap is open.

---

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
* Taking screenshots can now be bound to a key other than <kbd>PRINTSCREEN</kbd> using the `bind` CCMD with the new `+screenshot` action.
* Parameters can no longer be entered at the end of CCMDs that don’t use them.
* The player’s view will no longer jump slightly when dropping down between two liquid sectors greater than 24 units apart.

---

###### Thursday, June 9, 2016

### DOOM Retro v2.2.1

* *DOOM Retro* is now back to supporting *Windows XP* again.
* A crash will no longer occur when pressing the <kbd>PRINTSCREEN</kbd> key to take a screenshot on a display with an aspect ratio less than 4:3 (such as 1280×1024).
* A missing texture has been added to linedef 445 in E3M9 in `doom.wad`.
* Messages are now paused while the console is open.
* A bug has been fixed whereby IWADs weren’t being identified correctly.
* The player’s view is now only lowered if they are actually touching a liquid sector.
* Bobbing liquid sectors will now animate correctly if adjacent to a masked midtexture.
* The `centerweapon` CVAR can now also be entered as `centreweapon`.
* The `centered` value for the `vid_windowpos` CVAR can now also be entered as `centred`.

---

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
* [*ZDoom’s*](http://zdoom.org/) obituary strings are now ignored in `DEHACKED` patches so warnings aren’t displayed in the console at startup.
* A bug has been fixed whereby a frame would be skipped when rotating monsters in the *DOOM II* cast sequence.

---

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

---

###### Sunday, April 24, 2016

### DOOM Retro v2.1.2

* Optimizations have been made to further improve the overall performance and stability of *DOOM Retro*.
* Changes have been made to the format of savegames that breaks compatibility with previous versions of *DOOM Retro*.
* Any momentum from the player is now stopped when exiting a map so updating the exit switch is smoother.
* The `r_shakescreen` CVAR is now a value between `0%` and `100%`, instead of just `on` or `off`. Its default is `100%`.
* A bug has been fixed whereby some screen resolutions weren’t displaying correctly in the console at startup.
* *DOOM Retro’s* settings and savegames are now placed in `C:\<username>\AppData\Local\DOOM Retro\`. Remember, to change settings, open the console using the <kbd>~</kbd> key when *DOOM Retro* is running.
* Monster counts in the `playerstats` CCMD are no longer increased if “No monsters” has been set.
* Hanging decorations will no longer drop to the ground when over a liquid sector that moves.
* The direction items are dropped when a monster is killed is now better randomized.
* A bug has been fixed whereby pressing the <kbd>ENTER</kbd> key to close the help screen would cause the screen’s aspect ratio to be set incorrectly.
* The effects of changing the `r_translucency` CVAR are now instantaneous.
* A stray black pixel has been removed from under “N” characters in the menu.
* The <kbd>WINDOWS</kbd> key can no longer be pressed when fullscreen, as intended.
* The *Windows* screensaver is now disabled while *DOOM Retro* is running.

---

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

---

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
* *DOOM II’s* cast sequence now works correctly when using [`smoothed.wad`](https://www.doomworld.com/vb/wads-mods/85991-smoothed-wip-smooth-monsters-for-doom-retro/).
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
* The <kbd>WINDOWS</kbd> key can no longer be used at any time when fullscreen. It can only be used when in a window, and the game is paused, or the menu or console is open.
* Wall textures that are both animated and translucent can now be rendered correctly without causing a crash.
* The <kbd>E</kbd> key may now be pressed as an alternative to <kbd>SPACE</kbd> to use doors, switches, etc. It is bound to the `+use2` action.
* When the `vid_showfps` CVAR is enabled, the frames per second is now displayed correctly while the screen shakes when the player is injured.

---

###### Thursday, February 18, 2016

### DOOM Retro v2.0.5

* Bugs have been fixed whereby using `map next` in the console would warp the player to the next episode rather than the next map, and `map ExMy` wouldn’t warp at all.
* 100 additional sprites, named `SP00` to `SP99` and numbered 145 to 244, have been added for use in *DeHackEd* lumps.
* The amount of negative health a monster must receive to be gibbed can now be changed using a `Gib health` parameter in *DeHackEd* lumps.
* An invalid character will no longer be displayed in the console when changing the music or SFX volume in the menu.
* A bug has been fixed whereby when adjusting the SFX volume in the menu, the music volume was being displayed in the console instead.

---

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
* CCMDs and CVARs now appear in the correct order when pressing the <kbd>TAB</kbd> key in the console to autocomplete.
* The brightmap for the `SW2METAL` wall texture has been fixed.
* `SLIMExx` flats will no longer animate as liquid in `epic2.wad`.
* A small icon is now shown next to each warning in the console.
* The `STARTUP5` string is now displayed correctly in the console when playing [*Freedoom*](http://freedoom.github.io/).
* The `SDL2_mixer.dll` file supplied with *DOOM Retro* is now compiled with [*libmad 0.15.1b*](http://www.underbit.com/products/mad/), fixing the tempo of some MP3 lumps. Consequently, `smpeg2.dll` is no longer required.
* A bug has been fixed whereby using the `map` CCMD would cause the game to crash in some instances.
* The selected episode or expansion in the menu is set as necessary when using the `map` CCMD.

---

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
* Pressing <kbd>ALT</kbd> + <kbd>F4</kbd> to quit *DOOM Retro* now works again.
* Stylized quotes are now used in place of double quotes in the console.
* Text in the console is now slightly translucent.
* A random static effect has been applied to the console’s background.
* The effects of changing the `vid_windowpos` and `vid_windowsize` CVARs while in the console and in a window is now immediate.

---

###### Sunday, January 17, 2016

### DOOM Retro v2.0.2

* A rare bug has been fixed whereby the player’s view would continuously move or turn in one direction by itself.
* The `+run` action now works correctly when bound to a mouse button.
* The sound of a door closing is no longer played if the player walks over a line to trigger the door, and the door is already closed.
* It is now possible to warp to a map using `first`, `prev`/`previous`, `next` and `last` as the parameter for the `map` CCMD.
* A bug has been fixed whereby the muzzle flash of some weapons could be offset from the muzzle in some rare instances.
* The file `smpeg2.dll` is now included with *DOOM Retro* again.

---

###### Sunday, January 10, 2016

### DOOM Retro v2.0.1

* A bug has been fixed whereby the screen wouldn’t stop shaking after the player was killed in some instances.
* The `+run` action may now be bound to a mouse button.
* The player’s weapon will no longer be fullbright while the player is injured.

---

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
* Pressing <kbd>SHIFT</kbd> + <kbd>[</kbd>, <kbd>SHIFT</kbd> + <kbd>]</kbd> or <kbd>SHIFT</kbd> + <kbd>&#92;</kbd> in the console will now display the correct characters.
* The error displayed when `am_external` is `on` and an external automap can’t be created is now only displayed once at startup rather than each time the graphics subsystem is reset.
* The external automap is now blurred when the main display is.
* The `pm_walkbob` CVAR has been renamed to `movebob`.
* The `pm_alwaysrun` CVAR has been renamed to `alwaysrun`.
* The `pm_centerweapon` CVAR has been renamed to `centerweapon`.
* A bug has been fixed whereby the texture offsets for sectors that change from liquid to solid weren’t reset.
* An error is now displayed in the console if pressing the <kbd>PRINTSCREEN</kbd> key fails to take a screenshot.
* Pillarboxes are now cropped from screenshots.
* Rudimentary support has now been added for `MAPINFO` lumps. `MUSIC`, `NEXT`, `PAR`, `SECRETNEXT`, `SKY1` and `TITLEPATCH` keywords are supported. Additionally, the following keywords are supported specific to *DOOM Retro*:
  * `AUTHOR <author>`: Display the author’s name in the console when the map starts.
  * `LIQUID "<flat>"`: Specify a flat that will be treated as liquid in the map.
  * `NOLIQUID "<flat>"`: Specify an animated flat that won’t be treated as liquid in the map.
* *DOOM Retro* now has an alternate widescreen heads up display, inspired by the new *DOOM* released on May 13, 2016. It is disabled by default, and can be enabled using the `r_althud` CVAR. Widescreen mode, (displayed by pressing the <kbd>+</kbd> key to increase the screen size during a game, or through the options menu), needs to be enabled for it to be displayed.
* The player’s view no longer shifts at the start of a map when in windowed mode.
* The background is now redrawn whenever pressing the <kbd>ENTER</kbd> key in the console.
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
* The parameter for the `map` CCMD when using [*Freedoom*](http://freedoom.github.io/) can now be of the format `CxMy`.
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

---

###### Wednesday, October 21, 2015

### DOOM Retro v1.9

* *DOOM Retro* now allows the automap to be shown on a second display. This feature may be enabled using the `am_external` CVAR, and will display a fullscreen 16:10 automap in the first display it finds that is not the main display set by the `vid_display` CVAR. While this external automap is displayed, the <kbd>TAB</kbd> key is disabled, and the `IDDT` cheat can be entered at any time. Also, the automap’s usual controls are unavailable, but the grid and rotate mode may still be toggled in the console using the relevant CVARs.
* Optimizations have been made to further improve the overall performance of *DOOM Retro*.
* A new filter is now available to scale *DOOM Retro* onto the display. It is enabled by changing the value of the `vid_scalefilter` CVAR to `"nearest_linear"`, and is a combination of the existing two filters, `"nearest"` (which uses nearest-neighbor interpolation, the default) and `"linear"` (which uses linear filtering).
* The screen will no longer “bleed” along the edges when the `vid_scaledriver` CVAR is set to `""` or `"direct3d"` and the `vid_scalefilter` CVAR is set to `"linear"` on some systems.
* A bug has been fixed whereby screenshots couldn’t be taken by pressing the <kbd>PRINTSCREEN</kbd> key if characters that can’t be used in a filename were present in the current map’s title.
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
* The red screen tint when the player is injured, and the gold tint when the player picks up something will now show through while the player has a berserk power-up and their fists up.
* The `r_altlighting` CVAR has been removed.
* A slight current is now applied to liquid sectors, in a random direction determined at the start of each map. It may be disabled using the `r_liquid_current` CVAR.
* A bug has been fixed whereby the bottom wall texture between adjacent liquid sectors would show through in some instances.
* The player’s weapon sprite will no jump slightly when switching to and from the automap while moving.
* Although a majority of animated flats in *DOOM* are liquid, in some PWADs there are some that are not. There are now several instances in some popular PWADs where *DOOM Retro’s* liquid effects won’t be applied.
* The `r_lowpixelsize` CVAR will now be correctly parsed at startup.
* The `r_lowpixelsize` CVAR can now be set to values of `2×1` and `1×2`.
* The amount of blood splats spawned when corpses slide along the ground has been halved to 256.
* A bug has been fixed whereby generalized floors could become stuck after loading a savegame.
* The help screen has been updated to show that the <kbd>~</kbd> key opens the console.
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

---

###### Saturday, September 5, 2015

### DOOM Retro v1.8.5

* More than one instance of `-file` may now appear on the command-line.
* The amount of “map revealed” in the output of the `playerstats` CCMD is now always calculated correctly.
* The number of times the player cheats, both in the current map and overall, as well as the overall amount of time spent playing *DOOM Retro*, are now displayed in the output of the `playerstats` CCMD.
* *BOOM’s* `MF_TRANSLUCENT` flag is now supported in *DeHackEd* lumps and files.
* When binding an action to a control using the `bind` CCMD, any other actions that are bound to that same control will now be unbound.
* A bug has been fixed whereby the mouse pointer wouldn’t be released when pressing <kbd>ALT</kbd> + <kbd>TAB</kbd> to switch to the desktop.
* The game will now pause slightly when the player uses a switch to exit a map, to stop the switch’s sound from stuttering.
* Support has been added for certain hacks to the `NODE` lump of a map. See [here](http://doomwiki.org/wiki/Linguortal) for more information.
* The chaingunner’s refire frame is now fullbright.

---

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

---

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
* The game will no longer crash when trying to use the <kbd>F9</kbd> to quickload a game in some rare instances.

---

###### Wednesday, August 12, 2015

### DOOM Retro v1.8.2

* Although quite often the same folder, *DOOM Retro* will now put savegames in the same folder as the executable, rather than the current working folder.
* A bug has been fixed whereby sprites would appear through closed doors in some instances.

---

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

---

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
* Support has been added for maps with *DeepBSP* extended nodes v4 and [*ZDoom*](http://zdoom.org/) uncompressed normal nodes.
* Several rendering anomalies in maps have been resolved.
* Any flats that are missing in a map will now be rendered as sky, and a warning displayed in the console, rather than *DOOM Retro* exiting with an error.
* Further improvements have been made to the support for *DeHackEd* lumps.
* The translucency of the chaingun’s muzzle flash has been improved slightly.
* The “always run” feature may now be bound to a key other than <kbd>CAPSLOCK</kbd> in the console by using the `+alwaysrun` action with the `bind` CCMD.
* Movement of the player’s weapon is now interpolated to appear smoother.
* Rather than using the standard animation, which is only updated every 125 milliseconds, a much smoother swirl effect is now applied to every liquid sector. It is on by default, and can be turned off using the `r_liquid_swirl` CVAR.
* The speed of liquid sectors bobbing up and down has now been doubled.
* Things in liquid sectors no longer bob in time with each other.
* If the blockmap of a map is invalid or not present, it will now be recreated.
* The position of keycards and skull keys in the widescreen HUD when the player has no armor has been improved.
* The input in the console will now be restored after viewing the input history using the <kbd>&uarr;</kbd> key.
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
* MAP05C and MAP16C in [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960) may now be loaded using the `map` CCMD.
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
  * <kbd>SHIFT</kbd> + <kbd>&larr;</kbd>: Select the character to the left of the text caret.
  * <kbd>SHIFT</kbd> + <kbd>&rarr;</kbd>: Select the character to the right of the text caret.
  * <kbd>CTRL</kbd> + <kbd>A</kbd>: Select all of the text.
  * <kbd>CTRL</kbd> + <kbd>C</kbd>: Copy the currently selected text to the clipboard.
  * <kbd>CTRL</kbd> + <kbd>V</kbd>: Paste text from the clipboard.
  * <kbd>CTRL</kbd> + <kbd>X</kbd>: Cut the currently selected text to the clipboard.
  * <kbd>CTRL</kbd> + <kbd>Z</kbd>: Undo the last change that was made to the text.
* A bug has been fixed whereby no evil grin would be displayed in the status bar when the player picked up a new weapon.
* `warp` can now be used as an alternative to the `map` CCMD.
* A feature has been implemented that causes corpses to be moved slightly if a monster walks over them. It is enabled by default, and may be disabled using the `r_corpses_nudge` CVAR.
* A bug has been fixed whereby monsters would sometimes fall off tall ledges.
* Friction in liquid is now only reduced for corpses.
* Active crushers will now be active again when loading a savegame.
* Improvements have been made to the player using a switch, lift or door if very close to another.
* The display of the value of the `r_lowpixelsize` CVAR in the console has been fixed.
* There is no longer any blue or green blood or blood splats in [*Freedoom*](http://freedoom.github.io/).
* A bug has been fixed whereby no maps were listed by the `maplist` CCMD in [*Freedoom*](http://freedoom.github.io/).
* Blues are now emphasized better in translucent sprites.
* The console now automatically closes when using the `map` CCMD or the `IDCLEVxy` cheat.
* The HOM indicator is now paused while the console is open.
* A bug has been fixed whereby warnings weren’t being displayed in the console.
* The <kbd>WINDOWS</kbd> key is now only disabled during a game, and not while the game is in a menu, paused, in the console, or on the title screen.

---

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
* The <kbd>HOME</kbd> and <kbd>END</kbd> keys will now scroll to the top and bottom of the console output, if the player has started scrolling up with the <kbd>PGUP</kbd> key.
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
* The normal use of the <kbd>CAPSLOCK</kbd> key is now disabled in the console, and will toggle the “always run” feature on/off instead.
* A bug has been fixed whereby the value displayed by the `totalkills` command in the console wouldn’t take into account any pain elementals killed using the `kill` command.
* The console is now automatically closed when using the `endgame`, `exitmap`, `kill` and `map` CCMDs.
* A bug has been fixed whereby the map name in the automap was displayed incorrectly when using [*ZDL*](http://zdoom.org/wiki/ZDL) to launch the game.
* The “fuzzy” edges of spectre shadows are now paused while the console is open.

---

###### Monday, April 20, 2015

### DOOM Retro v1.7

* *DOOM Retro* now uses [*SDL 2.0.3*](http://www.libsdl.org/) for its graphics and audio. This brings significant performance improvements, as the screen is now scaled using hardware acceleration when possible, as well as the following features:
  * The monitor to display the game on can now be specified.
  * V-sync can now be toggled on/off.
* *DOOM Retro* now includes a console. It may be opened at any time by pressing the <kbd>~</kbd> key.
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
* A bug has been fixed whereby some teleporters in [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960) were animating as if they were liquid.
* The position of the player arrow is now drawn much more accurately when in the automap and rotate mode is on.
* The automap will no longer disappear, nor the game crash, when zooming out in very large maps.
* Translucency is now applied to MegaSpheres as originally intended.

---

###### Tuesday, February 3, 2015

### DOOM Retro v1.6.7

* A bug has been fixed whereby *DOOM Retro* would crash at startup when trying to run in a screen resolution that wasn’t widescreen.
* *DOOM Retro* will no longer crash after successive presses of <kbd>ALT</kbd> + <kbd>ENTER</kbd> to switch between fullscreen and windowed modes.
* Ceilings that use liquid textures will now render correctly.
* An error will now be displayed at startup if a WAD file contains DeePBSP or ZDBSP nodes.
* The `saturation` setting in `doomretro.cfg` has been deprecated.
* A bug has been fixed whereby the shadows of dropped items weren’t also being mirrored when the `mirrorweapons` setting was `true` in `doomretro.cfg`.
* Weapons spawned at the start of a map are now also mirrored when the `mirrorweapons` setting was `true` in `doomretro.cfg`.
* *Vanilla DOOM’s* [“long wall error”](http://doomwiki.org/wiki/Long_wall_error) has been fixed.
* Further optimizations have been made to improve the overall performance and stability of *DOOM Retro*.
* Teleporters in [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960) are now drawn correctly in the automap before they have been triggered.
* Whether the automap is active or not, and any automap marks, are now saved in savegames. (Note that this change breaks savegame compatibility with previous versions of *DOOM Retro*.)
* A header comment has been added to the top of `doomretro.cfg`, with a note advising to “go to http://wiki.doomretro.com for information on changing these settings”.
* The FPS counter displayed when `-devparm` is specified on the command-line now won’t be hidden when taking a screenshot, and will continue to update when in a menu or the game is paused.
* Diminished lighting from the player has been enhanced.
* Blood splats are now only spawned at the same height as corpses as they slide.

---

###### Sunday, January 25, 2015

### DOOM Retro v1.6.6

* The new liquid animation that was introduced in *DOOM Retro v1.6.5* has been improved upon such that the entire textures of the sectors will now also rise and fall, rather than just their edges.
* Due to this change in animation, the floating objects in liquid sectors now rise and fall in sync with each other and the sector.
* The bottom of objects in liquid that don’t float (such as monsters and barrels) will now be clipped in sync with the rise and fall of the liquid.
* All liquid sectors are now animated, avoiding issues where one liquid sector would be animated, but another liquid sector adjacent to it wouldn’t be.
* A bug has been fixed whereby the liquid animation was stopping the player and/or monsters from being able to enter certain areas of some maps.
* The brightmap for the `COMP2` wall texture has been fixed.
* A bug has been fixed whereby other monsters could infight with arch-viles.
* The teleporter texture used in [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960) no longer animates like a liquid.
* Savegames will now be placed in the `savegames\DOOM2.WAD\` folder rather than the `savegames\unknown.wad\` folder when `nerve.wad` is loaded.
* The player’s weapon is now recentered after teleporting.

---

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

---

###### Wednesday, December 10, 2014

### DOOM Retro v1.6.4

* Several optimizations have been made in an attempt to further improve the overall performance of *DOOM Retro*.
* *DeHackEd* files (`*.deh` and `*.bex`) are now displayed and can be loaded in the WAD launcher. <kbd>CTRL</kbd>-click to select them along with the WAD file(s) you want to load.
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
* The menu may still be opened with the gamepad’s <kbd>START</kbd> button if `gamepad_sensitivity` has been reduced to `0`.
* A bug has been fixed whereby the help screens wrongly indicated that the <kbd>A</kbd> and <kbd>D</kbd> keys were used to turn rather than strafe.
* The overall gamepad sensitivity has been increased even more, and the default of `gamepad_sensitivity` has now been doubled from `16` to `32`.
* The player will now bob as intended when `playerbob` is greater than `75%`.
* Changes have been made to the messages displayed when adding and clearing marks.

---

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
* *DOOM Retro* should no longer crash when switching between fullscreen and windowed modes with <kbd>ALT</kbd> + <kbd>ENTER</kbd>, and when resizing the window.
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
* A bug has been fixed whereby pressing the <kbd>CAPSLOCK</kbd> key on the title screen will cause the use of the key to then become inverted (that is, turning <kbd>CAPSLOCK</kbd> on would turn “always run” off, and vice versa).
* The display of the asterisk character is now allowed in a savegame description.
* The player arrow in the automap is now displayed correctly when zoomed in.
* A bug has hopefully been fixed that caused objects to sometimes disappear when standing on sector boundaries.
* Several changes have been made to improve *DOOM Retro’s* *DeHackEd* support.
* A bug has been fixed present in *Vanilla DOOM* whereby the vertical position of an arch-vile’s fire attack could be set incorrectly in some instances.
* There is no longer any small upward thrust when the player is receives an arch-vile’s fire attack while “no clipping mode” is on using the `IDCLIP` cheat.

---

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
* The <kbd>,</kbd> and <kbd>.</kbd> keys can now also be used to strafe left and right, as they could in *Vanilla DOOM*. They are changed using the `key_strafeleft2` and `key_straferight2` settings in `doomretro.cfg`.
* Shifted characters are now allowed when entering savegame descriptions. (In *Vanilla DOOM*, pressing <kbd>SHIFT</kbd> + <kbd>/</kbd>, for example, would still display “/” rather than “?”.)

---

###### Tuesday, November 4, 2014

### DOOM Retro v1.6.1

* If a *DeHackEd* file (with a `.deh` extension) is present with the same name and in the same folder as the selected PWAD, it will be automatically opened as well.
* Improvements have been made to when the player slides against walls.
* A bug has been fixed whereby the screen would not render fully after switching from fullscreen to windowed modes when pressing <kbd>ALT</kbd> + <kbd>ENTER</kbd>.
* Several compatibility fixes have been made when using *DeHackEd* files and lumps.
* Savegames for [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960) are now separated by episode.
* A bug has been fixed whereby [*HacX: Twitch ’n Kill*](http://www.drnostromo.com/hacx/) wouldn’t load at all. Specific support has now been added for it.
* Fake contrast is now applied to outdoor areas again.
* Thing triangles no longer appear for the invisible corpses in *Chex Quest* when using the `IDDT` cheat in the automap.
* The default value of `snd_maxslicetime_ms` has been changed from `120` to `28`. This is consistent with *Chocolate DOOM’s* default, and reduces the slight lag when playing sounds.
* The `centeredweapon` setting has been added to `doomretro.cfg`. Setting it to `true` will center the player’s weapon each time it’s fired (the default). If `false`, *Vanilla DOOM’s* behavior is used.
* A bug has been fixed whereby input would momentarily become stuck if the splash screen was skipped at startup.
* Blood splats are now green in *Chex Quest* as intended.
* A bug has been fixed whereby switching to and from the chainsaw using the number keys really quickly would cause either a crash, or the player’s weapon to disappear completely.
* The player’s weapon bob is now consistent with *Vanilla DOOM*.

---

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
* Specific support has also been added for [*Back To Saturn X E1: Get Out Of My Stations*](https://www.doomworld.com/idgames/levels/doom2/megawads/btsx_e1) and [*Back To Saturn X E2: Tower in the Fountain of Sparks*](https://www.doomworld.com/vb/thread/69960):
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
* A bug has been fixed whereby if an action was assigned to the <kbd>I</kbd> key, it wouldn’t work since “`I`” is the first character of every cheat.
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
* A bug has been fixed whereby the <kbd>,</kbd> key was being incorrectly mapped, causing it to act like the <kbd>-</kbd> key if it was bound to anything.
* Use of the <kbd>ALT</kbd> and <kbd>CTRL</kbd> keys in the menu has been disabled.
* Empty savegame slots may no longer be selected in the load game menu.
* Only update savegame descriptions of the form `ExMy` in *DOOM* games, not *DOOM II* games.
* The text caret in the save game menu is now better positioned when using a PWAD with a custom character set.
* A bug has been fixed whereby the position of the skull cursor could be misplaced when exiting and then returning to the menu using the gamepad in some instances.
* The number of characters that can be entered in a savegame slot is now calculated correctly.
* If the <kbd>CAPSLOCK</kbd> key was on before *DOOM Retro* was run, it is now turned back on as necessary when quitting.
* Player messages and the map name in the automap are now translucent and have drop shadows when using a PWAD with a custom character set.
* A bug has been fixed whereby an arch-vile could resurrect a monster for it to instantly become stuck in another monster.
* The player’s screen will no longer flash red if they are hit by a projectile while already dead.
* A bug has been fixed whereby the game would crash when using the `-nosfx` or `-nosound` command-line parameters.
* A bug has been fixed whereby you could switch weapons when zooming in the automap if both actions were set to the gamepad’s left and right shoulder buttons.
* Skies with heights other than 128 pixels are now rendered correctly.

---

###### Friday, August 15, 2014

### DOOM Retro v1.5.2

* Monster targets are now completely restored upon loading a game, regardless of whether they were targeting the player, or they were infighting.
* A Boss Brain no longer needs to be in MAP30 for the monsters it spawns to telefrag the player.
* A bug has been fixed whereby monsters were allowed to be above or below other monsters after teleporting, when normally they wouldn’t be.
* Improvements have been made to the position of some elements in the menu and HUD.
* Optimizations have been made to the loading of maps, and the lighting of the player’s weapon.
* A bug has been fixed whereby the game could crash when trying to draw the player’s weapon in pitch black areas in some instances.
* A bug has been fixed whereby the <kbd>CAPSLOCK</kbd> key was not being turned off when quitting the game.
* Pressing <kbd>CAPSLOCK</kbd> during a game will now display an `ALWAYS RUN ON/OFF` message. If when quitting the game the <kbd>CAPSLOCK</kbd> key is still on, it will be turned back on the next time *DOOM Retro* is started.
* In the previous version of *DOOM Retro*, the default video driver was changed from *Windows GDI* to *DirectX* to help in improving performance in fullscreen mode. If *DirectX* wasn’t installed, *DOOM Retro* would exit with an error. Now, if one video driver fails, *DOOM Retro* will try the other driver before exiting with an error.
* All in-game messages are now output to `stdout.txt`, whether messages are enabled or not.
* Whenever the player or a monster is killed, a message is displayed in `stdout.txt`.
* Minor changes have been made to a few messages.
* A bug has been fixed whereby *DOOM Retro* would exit with an error if the `IDDQD` cheat was used to resurrect a dead player.

---

###### Thursday, August 7, 2014

### DOOM Retro v1.5.1

* A splash screen is now displayed briefly when *DOOM Retro* is opened.
* Several optimizations have been made to improve *DOOM Retro’s* overall performance and reduce the size of its executable.
* *DOOM Retro* is now back to using the desktop resolution by default.
* If `doom2.wad` is selected by itself in the WAD launcher, `nerve.wad` will be automatically loaded if it’s in the same folder, and *Hell on Earth* will be preselected in the expansion menu. If `nerve.wad` is selected by itself, *No Rest for the Living* will be preselected instead.
* The `DOOMWADDIR` environment variable is now checked when automatically looking for IWAD files.
* No longer is anything output to the `stdout.txt` and `stderr.txt` files generated by SDL.
* A bug has been fixed whereby sometimes the press of a mouse button would register twice in a menu.
* The <kbd>PAUSE</kbd> key is now disabled on the title screen.
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

---

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
* If a [*Freedoom*](http://freedoom.github.io/) IWAD is loaded in *DOOM Retro*, a message is displayed explaining that “FREEDOOM requires a BOOM-compatible source port, and is therefore unable to be opened”. This message won’t be displayed if the IWAD is loaded with an additional PWAD, so [*Freedoom*](http://freedoom.github.io/) can be used as a resource for the maps present in that PWAD.
* Further improvements have been made when using custom graphics in PWADs, particularly if [*Freedoom*](http://freedoom.github.io/) is being used.
* A bug from *Vanilla DOOM* has been fixed whereby corrupt texture names would be displayed in the error if a texture couldn’t be found. See the [*DOOM Wiki*](http://doomwiki.org/wiki/Absurd_texture_name_in_error_message) for more information.
* A bug has been fixed whereby pressing the <kbd>ENTER</kbd> key or the left mouse button when in the help screen wouldn’t restore widescreen mode.
* The speed of turning with the gamepad’s right thumbstick when holding down the left trigger to run has been reduced slightly.
* The screen is now a constant tint of red while the player has the berserk power-up and their fist selected.
* The `blur` and `grayscale` settings that control the menu background have been removed from `doomretro.cfg`. The menu background will now always be blurred and gray.
* A bug has been fixed whereby the position of some items would be affected by moving platforms nearby.
* To better replicate the look of CRT monitors, which are/were not as bright as current LCD monitors, desaturation is now applied to *DOOM Retro’s* graphics. Changing the `saturation` setting in `doomretro.cfg` to `0` gives a grayscale effect, `1.0` is normal saturation, and `0.75` is the default.
* `gammalevel` has been changed to just `gamma` in `doomretro.cfg`.
* A bug has been fixed whereby non-solid hanging corpses would drop to the floor when above a moving sector in some instances.
* <kbd>ALT</kbd> + <kbd>F4</kbd> will now quit the game instantly without prompting.
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
* If a screenshot is taken with the <kbd>PRINTSCREEN</kbd> key when not in a game, the resulting file will be named accordingly: `Title.bmp`, `Help.bmp`, `Intermission.bmp` or `Finale.bmp`.
* The <kbd>CAPSLOCK</kbd> key is turned off if on when quitting the game.
* Some of the less-used mouse controls from *Vanilla DOOM* have been reimplemented. Disabled by default, they can be reenabled using the following settings in `doomretro.cfg`:
  * Set `novert` to `true` to allow vertical mouse movement to move the player forward/back.
  * Set `mouse_forward` to a mouse button to move forward.
  * Set `mouse_strafe` to a mouse button to strafe.
  * Set `dclick_use` to `true` so double-clicking the mouse buttons set by `mouse_forward` and `mouse_strafe` above will perform a use action.
* A bug has been fixed whereby the values `middle` and `right` representing mouse buttons in `doomretro.cfg` were switched.
* Now pressing a key the first time on a finale text screen will display all the text, and a second press will then advance to the next map.
* A bug has been fixed whereby pressing <kbd>SPACE</kbd> to advance a finale text screen would carry over to the following map, and cause the player to use a switch if they started directly in front of one (such as is the case for *MAP07: Dead Simple* in *DOOM II: Hell On Earth*).
* The flashing key in the HUD will now be updated if the player tries opening another locked door that requires a different key to the one currently flashing.

---

###### Tuesday, May 13, 2014

### DOOM Retro v1.4.3

* The screen size can now be adjusted correctly in the options menu when not in a game.
* The size of the pixels when the graphic detail is “LOW” can now be changed by editing the `pixelwidth` and `pixelheight` settings in `doomretro.cfg`. Both are set to a default of `2`.
* A bug has been fixed whereby pressing <kbd>ENTER</kbd> to save a game in an empty savegame slot wouldn’t clear that slot and replace it with the name of the current map.
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

---

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

---

###### Tuesday, April 29, 2014

### DOOM Retro v1.4.1

* A bug has been fixed whereby the sound would become disabled if adjusting the volume through the menu or pausing and then unpausing the game.
* The visplane limit has been removed, allowing for more detailed levels to run without crashing.
* A sound will now be heard if a wrong selection is made in the WAD launcher and it needs to reopen.
* When a heavy weapon dude is killed, their corpse is no longer randomly mirrored.
* Projectiles will now pass through map decorations like they do in *Vanilla DOOM*. (Please note that this particular change means savegames from previous versions of *DOOM Retro* won’t work with *DOOM Retro v1.4.1*.)
* If music can’t be loaded for a particular map, that map will still load without music rather than the game exiting with an error.

---

###### Friday, April 25, 2014

### DOOM Retro v1.4

* Several optimizations have been made that improve the overall performance of *DOOM Retro*.
* When *DOOM Retro* is opened for the first time the following message is now displayed:
  > Thank you for downloading DOOM RETRO!
  >
  > Please note that, as with all DOOM source ports, no actual map data is distributed with DOOM RETRO.
  >
  > In the dialog box that follows, please navigate to where an official release of DOOM or DOOM II has been installed and select a “WAD file” that DOOM RETRO requires (such as DOOM.WAD or DOOM2.WAD).
* There are now 2 levels of graphic detail, “HIGH” and “LOW”, adjustable through either the options menu or by pressing the <kbd>F5</kbd> key. The “HIGH” level has the same graphic detail as in previous versions of *DOOM Retro*. It has an effective resolution of 640×400 (or 640×336 in widescreen) and is the default. The “LOW” level is new, and has an effective resolution of 320×200 (the resolution used in *Vanilla DOOM*).
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
* The first press of the <kbd>F11</kbd> key will now display the current gamma correction level. Further presses of the key before the message disappears will then increase the level (or decrease the level if the <kbd>SHIFT</kbd> key is held down).
* Autorepeat is now allowed for the <kbd>F11</kbd> key.
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

---

###### Tuesday, April 1, 2014

### DOOM Retro v1.3

* An extensive number of optimizations have been made that improve the overall performance of *DOOM Retro*.
* A heads-up display (HUD) is now shown in widescreen mode.
* Each element of the HUD is slightly translucent.
* An additional press of the <kbd>+</kbd> key, or move of the “SCREEN SIZE” slider to the right in the options menu, will hide the HUD.
* The HUD isn’t displayed while the player is dead.
* If the player has the berserk power-up, and has his fist selected, the berserk power-up will replace the medikit in the HUD.
* The keys that the player has picked up are displayed in the order they were found, from right to left.
* If the player attempts to open a door they don’t have the key for, that key will flash in the HUD.
* If the player has no armor, the keys are displayed along the right side of the screen.
* The type of armor the player has (either green or blue armor) is displayed.
* The background is now blurred when in a menu or the game is paused. This effect may be disabled by changing the `menublur` setting in `default.cfg` to false.
* The green blood of hell knights and barons of hell is now slightly darker.
* A bug has been fixed that existed in *Vanilla DOOM* that caused monsters to be able to see the player through walls, or not see the player when they should have, in some instances. Thank you to 倉敷楠花 (Nanka Kurashiki) for bringing this to my attention.
* The <kbd>PRINTSCREEN</kbd> key and <kbd>ALT</kbd> + <kbd>ENTER</kbd> have been added to the help screen.
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
* The <kbd>PRINTSCREEN</kbd> key now no longer saves the screen to the clipboard when taking a screenshot.
* Rotation in the automap is now more accurate.
* A bug has been fixed whereby the crosshair could still decelerate from panning while the menu was displayed.
* The red crosses in stimpacks are now darker to be consistent with medikits.
* If a sprite is replaced with a custom sprite in a PWAD, any translucency will be removed and custom offsets won’t be applied.
* The width of pickups are now calculated differently such that they are less likely to be suspended in midair when close to a change in height.
* The `TEKWALL1` texture (for example, as used on the green armor platform in *E1M1: Hangar*) is now displayed the same way as in *Vanilla DOOM*.
* *Windows* accessibility shortcut keys are disabled during the game.
* The <kbd>WINDOWS</kbd> key is now also disabled when in a window.
* The pistol sound is now used when toggling messages in the options menu using the <kbd>&larr;</kbd> and <kbd>&rarr;</kbd> cursor keys.
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
* The <kbd>ENTER</kbd> key on the numeric keypad can now be used wherever the main <kbd>ENTER</kbd> key can be used.
* `DMENUPIC` is now used on the intermission screen in *DOOM II (BFG Edition)*.
* There are no longer any overlapping drop shadows in the menus.
* Keys are now positioned correctly in the status bar.

---

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
* The <kbd>+</kbd> and <kbd>–</kbd> keys, as well as moving left and right on a gamepad, can no longer be used to toggle messages on and off in the options menu.
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
* 33% alpha translucency rather additive translucency is now used for the SoulSphere, MegaSphere, Invincibility and Partial Invisibility power-ups.
* The <kbd>+</kbd> and <kbd>–</kbd> keys can no longer be used while the help screen is displayed.
* In those levels that require one or more monsters to be killed for a sector to move to complete the level, if the `–nomonsters` command-line parameter is specified, those sectors will now automatically move.
* Replicating what happens in *Heretic* and *Hexen*, the remaining monsters in the level will turn on each other once the player has been killed. The player will face their killer when they die, but unlike those games, their view won’t continue to follow their killer around.
* A bug has been fixed whereby *DOOM’s* episode menu would be displayed when pressing the <kbd>ESC</kbd> key on *DOOM II’s* skill level menu.
* The `IDCHOPPERS` cheat will now be canceled (by removing the invulnerability power-up and the chainsaw) when the player switches to or picks up a weapon other than the chainsaw.
* Many monsters are now positioned and animate better in *DOOM II’s* cast sequence.
* Monsters now can’t be rotated in the cast sequence until they are actually on the screen.
* Z-coordinates are now taken into account when telefragging.
* Arch-viles no longer resurrect monsters such that they become stuck in other monsters, or under doors.
* A bug has been fixed whereby the game may crash when an arch-vile resurrects a monster after a savegame has been loaded.
* The boss in *MAP30: Icon Of Sin* at the end of *DOOM II: Hell On Earth* will now still make its alert sound when the level starts if `–nomonsters` is specified on the command-line.
* A “rotate mode” is now available in the automap. Toggled on and off using the <kbd>R</kbd> key, it will dynamically orientate the level such that the player is always pointing towards the top of the screen. Rotate mode is off by default, and whether it is on or off is remembered between games.
* Only one instance of `doomretro.exe` can be run at a time.
* The player arrow in the automap will now be translucent while the player has a partial invisibility power-up.
* In *DOOM II (BFG Edition)*, since `TITLEPIC` isn’t present in the IWAD, the otherwise unused `DMENUPIC` is now used instead of `INTERPIC`.

---

###### Friday, January 24, 2014

### DOOM Retro v1.1

* The source code is no longer distributed with *DOOM Retro* itself, and has instead been made separately available in a [*GitHub* repository](http://github.com/bradharding/doomretro). Please visit this site to follow the latest daily developments, and report any [issues](http://github.com/bradharding/doomretro/issues) that you may encounter.
* The correct and complete version information is now displayed when right-clicking on `doomretro.exe` and selecting “Properties”.
* The `–file` command-line parameter may no longer be used with *DOOM Shareware’s* WAD file, `doom1.wad`.
* If a `default.cfg` file is present in the game folder, it will now be used in preference to the `default.cfg` saved in `AppData\Local\DOOM RETRO` for the current *Windows* user.
* While still maintaining backwards compatibility, the values that may be specified in the `default.cfg` file are now much more readable, and easier to edit manually.
* Gamepad controls can now be customized by editing the `default.cfg` file.
* A bug has been fixed whereby the game wouldn’t be rendered correctly for displays with an aspect ratio less than 4:3 (that is, displays that aren’t widescreen).
* In fullscreen mode on a widescreen display, increasing the screen size with the <kbd>+</kbd> key to the maximum will now show a widescreen mode without the status bar, and without any of the horizontal stretching prevalent in many other source ports. *DOOM Retro* will revert to the standard 4:3 aspect ratio when on the title, intermission, finale and help screens.
* A bug has been fixed whereby parts of *MAP01: Entryway* and *MAP02: Underhalls* in *DOOM II: Hell On Earth* would become corrupted when using older versions of `doom2.wad`. Two barrels and a shotgun guy were missing from MAP02 as well. (Credit goes to Jon Krazov for his assistance.)
* For a majority of translucent or partially translucent objects, their translucency is now calculated using additive blending rather than alpha blending, resulting in them appearing considerably brighter.
* The blue lights in tall and short techno floor lamps (`MT_MISC29` and `MT_MISC30`) are now translucent.
* The red and green lights in all switches, as well as the exit signs and many computer terminals that appear in most levels are now consistently bright regardless of the surrounding light levels, and the distance from the player (that is, they are “fullbright”).
* Some minor cosmetic changes have been made to the status bar.
* The corpses of cyberdemons are no longer flipped horizontally at random.
* When the player ends a level by flicking a switch, that switch will now turn on before the screen is wiped.
* If the player has both the invulnerability and the light amplification visor power-ups, and the invulnerability power-up runs out first, the screen will now flash correctly between the inverted grayscale palette and the “fullbright” palette.
* If the player has both a chainsaw and a berserk power-up, pressing the <kbd>1</kbd> key will now directly switch to either the chainsaw or the fist, depending on which weapon was selected last, rather than always switching to the chainsaw. This selection is also remembered when saving a game.
* If the <kbd>SHIFT</kbd> key is held down when the <kbd>CAPSLOCK</kbd> key is on (or vice-versa), the player will walk instead of run, as originally intended.
* Monsters can no longer pass through tall level decorations.
* A bug has been fixed whereby it took approximately twice as many rockets to kill the boss in *MAP30: Icon Of Sin* at the end of *DOOM II: Hell On Earth*.
* Like what can be done at the end of *DOOM 64*, each monster can now be rotated using the <kbd>&larr;</kbd> and <kbd>&rarr;</kbd> cursor keys during the cast sequence in *DOOM II*.
* The lost soul in the cast sequence in *DOOM II* is now partially translucent.
* The explosions when the lost soul and the pain elemental die in the cast sequence in *DOOM II* are now translucent.
* A bug has been fixed whereby the cast sequence in *DOOM II* could not be advanced by pressing the <kbd>CTRL</kbd> key.
* The help screen has been updated to include the new controls for the keyboard, mouse and gamepad, and fixing several inconsistencies. (Credit goes to Robin “FrightNight” Reisinger for his assistance.)
* The text on the help screen now has drop shadows.
* The help screen’s background is now a low resolution snapshot of the game screen with a dark blue tint.
* The controls for selecting the plasma rifle and BFG 9000 are no longer present on the help screen in *DOOM Shareware*.
* When using a PWAD, screenshot filenames will now be of the format `ExMy.bmp` or `MAPxx.bmp` rather than incorrectly use the name of the map this map replaces in the main IWAD.
* When a screenshot is taken using the <kbd>PRINTSCREEN</kbd> key, any messages are now cleared from the top of the screen first.
* A bug has been fixed whereby if a game is saved while a platform is moving, it could potentially cause the game to crash when that savegame is loaded.
* When using the `IDCLEVxy` cheat, keycards and skull keys are no longer removed from the status bar before the screen is wiped.
* The correct message is now displayed when entering the `IDKFA` cheat.
