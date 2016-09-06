### DOOM Retro v2.2.5

* Optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* The shadows of mancubi have been raised slightly.
* The <code>resetall</code> CCMD will no longer cause a crash when used while not in a game.
* A warning will now be displayed in the console if gamepad initialization fails.
* A bug has been fixed whereby the <code>vid_scalefilter</code> CVAR would default to <code>"nearest_linear"</code> rather than <code>"nearest"</code> if invalid.
* If the initialization of music fails at startup, a warning will be displayed in the console and startup will continue with music disabled, rather than <i>DOOM Retro</i> quitting with an error.
* The <code>playername</code> CVAR can now be set to <code>""</code> in the console.
* The armor bar in the alternate HUD now aligns exactly with the health bar below it.
* The <code><span style="white-space:nowrap">&ndash;respawn</span></code> command-line parameter will now work correctly.
* Minor changes have been made to some of the output in the console.
* The correct <code>INTERPIC</code> lump will now be displayed if replaced in a PWAD and using <i>DOOM II: Hell On Earth (BFG Edition)</i>.
* When using <code>kill player</code> in the console, the player will no longer turn to face their last attacker.
* A <code>respawnitems</code> CCMD has been implemented that allows most items to be respawned 30 seconds after the player picks them up, replicating what happens in <i>DOOM</i> multiplayer.
* The contents of <code>doomretro.cfg</code> have been rearranged slightly and commented.

### DOOM Retro v2.2.4

* Blood splats and shadows are now drawn at greater distances.
* Minor changes have been made to some elements of the console.
* A bug has been fixed whereby the screen’s colors may appear wrong in some rare instances.
* If in low detail mode, and the <code>r_lowpixelsize</code> has been changed from its default, the view border will no longer be affected at smaller screen sizes.
* A <code>reset</code> CCMD has been implemented which will reset a CVAR to its default value.
* A <code>resetall</code> CCMD has been implemented which will reset all CVARs to their default values.
* A <code>bindlist</code> CCMD has been implemented which will list all the bound controls. Previously, entering the <code>bind</code> CCMD without any parameters would do this.
* The individual monster kills displayed using the <code>playerstats</code> CCMD will no longer sometimes become corrupted when an Arch-vile resurrects one.
* If <i>DOOM Retro</i> fails to launch for some reason, a more descriptive error will now be displayed.
* A bug has been fixed whereby changing the <code>vid_scalefilter</code> CVAR to <code>nearest_linear</code> in the console could fail in some instances.
* The floor texture of sector 103 in MAP04 of <code>plutonia.wad</code> has been fixed.
* A bug has been fixed whereby rocket launcher frames would be shown when firing the Photonzooka in <i>HacX: Twitch ’n Kill</i>.
* The text caret’s animation now resets each time the console is open, and is hidden when the console closes.

### DOOM Retro v2.2.3

* <i>DOOM Retro</i> is now completely portable. The configuration file, <code>doomretro.cfg</code>, is now saved in the same folder as the executable, savegames are saved in a <code>savegames&#92;</code> folder and screenshots are saved in a <code>screenshots&#92;</code> folder.
* Optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* Minor changes have been made to some of the text in the console.
* The width of any selected text is now accounted for when inputting text in the console.
* The number of available displays are now rechecked before creating an external automap if the <code>am_external</code> CVAR is enabled through the console.
* An <code>RMAPINFO</code> lump will now be used if present in preference to a <code>MAPINFO</code> lump to avoid conflicts with other <i>DOOM</i> source ports.
* The keyboard shortcuts <kbd>SHIFT</kbd> + <kbd>HOME</kbd> and <kbd>SHIFT</kbd> + <kbd>END</kbd> are now allowed in the console to select all text to the left and right of the caret.
* The <code>r_berserkintensity</code> CVAR now accepts a value between <code>0</code> and <code>8</code> inclusive instead of a percentage. It has a default of <code>2</code>.
* The <code>expansion</code> CVAR is no longer changed if <code>nerve.wad</code> is automatically loaded.
* The player’s view will no longer jump slightly when dead and their corpse is sliding down stairs.
* A <code>teleport</code> CCMD has been implemented that allows the player to be teleported to another location in the current map.
* Fuzzy shadows are now applied to any thing whose <code>SHADOW</code> bit has been set in a <code>DEHACKED</code> lump.
* The map number in the console and automap is now shown in the format <code>E2M<i>xy</i></code> in <i>Back To Saturn X E2: Tower In The Fountain Of Sparks</i>.
* The <code>r_bloodsplats_total</code> CVAR is now calculated correctly once it reaches <code>r_bloodsplats_max</code>.
* A bug has been fixed whereby palette effects from power-ups would remain on the screen after ending a game from the options menu in some instances.
* The value of <code>r_lowpixelsize</code> will no longer affect the display of the title screen when the menu is open.
* The <kbd>F5</kbd> key can no longer be used to change the graphic detail when the automap is open.

### DOOM Retro v2.2.2

* One IWAD or PWAD may now be specified on the command-line without the need for the <code><span style="white-space:nowrap">&ndash;iwad</span></code> or <code><span style="white-space:nowrap">&ndash;file</span></code> command-line parameters.
* Optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* The <code>am_followmode</code> CVAR no longer appears in <code>doomretro.cfg</code>.
* A bug has been fixed whereby the case of map titles in the console would be displayed incorrectly in some instances.
* The <code>movebob</code> CVAR now only specifies the amount the player’s view bobs up and down when they are moving. The amount the player’s weapon bobs up and down is now specified using a new CVAR called <code>weaponbob</code>. Both CVARs are <code>75%</code> by default.
* The <code>savegame</code> CVAR, that specifies the savegame currently selected in the menu, can now be accessed from the console.
* The <code>map</code> CCMD can now be used with the <code>first</code> and <code>last</code> parameters when a game hasn’t started.
* Several new monster name variations can now be used with the <code>spawn</code> CCMD.
* A <code>play</code> CCMD has been implemented that allows any music or sound lump to be played.
* A bug has been fixed whereby marks couldn’t be added to the automap when rotate mode was off.
* The filename displayed when taking a screenshot while <code>am_external</code> is <code>on</code> is now fixed.
* The “picked up” player messages are no longer displayed when using the <code>give</code> CCMD.
* The position of flashing keycards and skull keys in the alternate HUD when the player tries to open a locked door has been fixed.
* Taking screenshots can now be bound to a key other than <kbd>PRINTSCREEN</kbd> using the <code>bind</code> CCMD with the new <code>+screenshot</code> action.
* Parameters can no longer be entered at the end of CCMDs that don’t use them.
* The player’s view will no longer jump slightly when dropping down between two liquid sectors greater than 24 units apart.

### DOOM Retro v2.2.1

* <i>DOOM Retro</i> is now back to supporting Windows XP again.
* A crash will no longer occur when pressing the <kbd>PRINTSCREEN</kbd> key to take a screenshot on a display with an aspect ratio less than 4:3 (such as 1280×1024).
* A missing texture has been added to linedef 445 in E3M9 in <code>doom.wad</code>.
* Messages are now paused while the console is open.
* A bug has been fixed whereby IWADs weren’t being identified correctly.
* The player’s view is now only lowered if they are actually touching a liquid sector.
* Bobbing liquid sectors will now animate correctly if adjacent to a masked texture.
* The <code>centerweapon</code> CVAR can now also be entered as <code>centreweapon</code>.
* The <code>centered</code> value for the <code>vid_windowposition</code> CVAR can now also be entered as <code>centred</code>.

### DOOM Retro v2.2

* Optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* <i>DOOM Retro’s</i> settings and savegames are now placed in <code>C:&#92;<i>username</i>&#92;DOOM Retro&#92;</code>.
* “.wad” is now removed from savegame folder paths.
* Entering <code>r_gamma off</code> in the console now works correctly.
* Player messages and the automap title are now less translucent.
* Monsters will no longer be alerted when the player punches thin air. They will still be alerted if the player punches a wall, however.
* A bug has been fixed whereby obituaries in the console would be incorrect in some instances.
* Support has been added for <code>MUSINFO</code> lumps.
* Support has also been added for music changer map objects.
* The translucency of health bonuses has been reduced from 33% to 25%.
* A slight change has been made to the background noise effect in the console.
* Screenshots are now saved as PNG files rather than Windows BMP files.
* The full path of the file is now displayed in the console when a screenshot is taken.
* The player’s view will no longer briefly change before the screen wipes when exiting a map.
* The view border is now displayed correctly for PWADs such as <code>valiant.wad</code> and <code>aaliens_rc2.wad</code>.
* The positions of the shadows of some monsters have been improved.
* The flats <code>SLIME09</code> to <code>SLIME12</code> no longer animate as liquid in <i>Ancient Aliens</i>.
* The <code>help</code> CCMD will now open the console section of the <i>DOOM Retro Wiki</i> in the default browser.
* There are now <code>ammo</code>, <code>armor</code> and <code>health</code> CVARs that allow changing the player’s ammo, armor and health to specific values.
* The texture offset of linedef 638 in MAP10 of <code>DOOM2</code> has been corrected.
* The compiler used to build <i>DOOM Retro</i>, and its version, are now displayed in the console at startup.
* A bug has been fixed whereby pressing a mouse or gamepad button at startup in the brief moment before the splash screen appears would cause the screen to stay black.
* Several visual aspects of the console have been redesigned.
* The markers are now smaller in the health and armor bars in the alternate HUD.
* If the player has more than 100% health, a second marker will appear above the health bar in the alternate HUD.
* The armor bar in the alternate HUD is now slightly thinner, and divided into five rather than four sections.
* The larger digits used in the alternate HUD are now consistent with the smaller ones.
* The screen’s green haze when the player has the radiation suit power-up will now be visible if the player also has the berserk power-up, their fists selected, but the <code>r_berserkintensity</code> CVAR set to <code>0%</code>.
* The default gamepad sensitivity has been increased from 32 to 48.
* Corpses are no longer teleported if the <code>kill</code> CCMD is used.
* Player messages are now always yellow in the console.
* Color CVARs are now set to their defaults, rather than the closest valid value, if invalid at startup.
* C++ style comments are now allowed in <code>MAPINFO</code> lumps.
* The correct colors are now preserved in the automap, the console and the alternate HUD if a PWAD contains a custom <code>PLAYPAL</code> lump. (An example of such a PWAD is skillsaw’s recently released MegaWAD, <i>Ancient Aliens</i>.)
* A bug has been fixed whereby parts of the super shotgun would be transparent in <i>Ancient Aliens</i>.
* The <code>r_corpses_color</code> CVAR is now validated at startup.
* If <code>am_external</code> is on but there’s only one display found, there will no longer be a crash if the graphics system is restarted.
* The number of logical cores and amount of system RAM is now displayed in the console at startup.
* <i>ZDoom’s</i> obituary strings are now ignored in <code>DEHACKED</code> patches so warnings aren’t displayed in the console at startup.
* A bug has been fixed whereby a frame would be skipped when rotating monsters in the <i>DOOM II</i> cast sequence.

### DOOM Retro v2.1.3

* Optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* Scrolling up in the options menu now works correctly.
* The <code>playername</code> CVAR is reset to its default if it’s empty at startup.
* The fixes that <i>DOOM Retro</i> applies to maps in the official <i>DOOM</i> and <i>DOOM II</i> IWADs are now listed in the console when a map is loaded and <code><span style="white-space:nowrap">&ndash;devparm</span></code> is specified on the command-line.
* The default mouse sensitivity has been increased from 24 to 32.
* The health cross in the alternate HUD is now squarer.
* An armor count is now displayed in the alternate HUD.
* A bug has been fixed whereby keycards and skull keys weren’t positioned correctly in the status bar in some instances.
* The brightmap for the <code>SW2BRNGN</code> wall texture is now fixed.
* Blood splats are now always spawned for monsters that bleed when using the <code>kill</code> CCMD.
* Monsters will now be alerted when the player punches a wall or the air.
* The full map title of John Romero’s new map, <a target="_blank" href="//twitter.com/romero/status/725032002244759552"><i>E1M4B: Phobos Mission Control</i></a> is now displayed in both the console and AutoMap.
* The player’s view will now bob up and down if they die on a liquid sector.
* Improvements have been made to the accuracy of “Weapon accuracy” in the output of the <code>playerstats</code> CCMD.
* The corpses of monsters are no longer spawned if “No monsters” has been set.

### DOOM Retro v2.1.2

* Optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* Changes have been made to the format of savegames that breaks compatibility with previous versions of <i>DOOM Retro</i>.
* Any momentum from the player is now stopped when exiting a map so updating the exit switch is smoother.
* The <code>r_shakescreen</code> CVAR is now a value between <code>0%</code> and <code>100%</code>, instead of just <code>on</code> or <code>off</code>. Its default is <code>100%</code>.
* A bug has been fixed whereby some screen resolutions weren’t displaying correctly in the console at startup.
* <i>DOOM Retro’s</i> settings and savegames are now placed in <code>C:&#92;<i>username</i>&#92;AppData&#92;Local&#92;DOOM Retro&#92;</code>. Remember, to change settings, open the console using the <kbd>~</kbd> key when <i>DOOM Retro</i> is running.
* Monster counts in the <code>playerstats</code> CCMD are no longer increased if “No monsters” has been set.
* Hanging decorations will no longer drop to the ground when over a liquid sector that moves.
* The direction items are dropped when a monster is killed is now better randomized.
* A bug has been fixed whereby pressing the <kbd>ENTER</kbd> key to close the help screen would cause the screen’s aspect ratio to be set incorrectly.
* The effects of changing the <code>r_translucency</code> CVAR are now instantaneous.
* A stray black pixel has been removed from under “N” characters in the menu.
* The <kbd>WINDOWS</kbd> key can no longer be pressed when fullscreen, as intended.
* The <i>Windows</i> screensaver is now disabled while <i>DOOM Retro</i> is running.

### DOOM Retro v2.1.1

* Pain elementals can now shoot lost souls through two-sided walls that have the <code>ML_BLOCKMONSTERS</code> flag, as is possible in <i>Vanilla DOOM</i>. (An example of this is at the end of MAP04 in <a target="_blank" href="//www.doomworld.com/idgames/levels/doom2/megawads/requiem"><code>requiem.wad</code></a>.)
* The screen will no longer briefly flash if the player has a berserk or radiation shielding suit power-up and then loads a savegame or starts a new game from the menu.
* The time taken to complete a map is now restored correctly when loading a savegame.
* A bug has been fixed whereby a map would become corrupted if the player triggered a generalized line with no tag (such as when the player takes the “plunge” in MAP08 of <a target="_blank" href="//www.doomworld.com/idgames/levels/doom2/Ports/megawads/jenesis"><code>jenesis.wad</code></a>).
* A crash will no longer occur when using the <code>maplist</code> CCMD.
* Multiple <code>STBAR</code> lumps are now better handled. (For example, now the correct status bar will be displayed if <code>JPCP_HUDjpn.wad</code> is loaded along with <a target="_blank" href="//www.doomworld.com/idgames/levels/doom2/Ports/megawads/jenesis"><code>JPCP_1st.wad</code></a>.)
* Double-resolution yellow and grey digits will no longer be displayed in the status bar if a <code>STBAR</code> lump from a PWAD is used.
* The correct WAD is displayed in the output of the <code>mapstats</code> in <i>DOOM II: Hell On Earth</i> if <code>nerve.wad</code> is also present.

### DOOM Retro v2.1

* Optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* Support has been added for the <code>SWITCHES</code> lump.
* Fast monsters can now be enabled by using the <code>fastmonsters</code> CCMD. Also, <code>&ndash;fastmonsters</code> as well as just <code>&ndash;fast</code> can be specified on the command-line.
* Warnings displayed when parsing <i>DeHackEd</i> files and lumps are now only displayed if <code>&ndash;devparm</code> is specified on the command-line.
* Automatically loading the last savegame when the player dies can now be disabled by using the <code>autoload</code> CVAR.
* The intensity of the red haze effect when the player has the berserk power-up and their fists selected can now be changed using the <code>r_berserkintensity</code> CVAR. The default is <code>33%</code>.
* Player sprites may now be translucent if they are replaced in a PWAD by using <code>Translucent = 1</code> for the relevant frame in <i>DeHackEd</i> files and lumps.
* Thing flags specific to <i>DOOM Retro</i> can now be changed using <code>Retro Bits</code> instead of <code>Bits2</code> in <i>DeHackEd</i> files and lumps.
* A bug has been fixed whereby the use of unknown music names in <code>MAPINFO</code> lumps would cause a crash.
* The <code>s_musicvolume</code> and <code>s_sfxvolume</code> CVARs are now corrected at startup if invalid.
* The <code>faceback</code> CVAR has been renamed to <code>facebackcolor</code>.
* The extraneous brown pixel in the super shotgun is no longer removed if the weapon is changed in a <i>DeHackEd</i> file or lump.
* Further improvements have been made to the appearance of the rocket launcher's muzzle flash.
* DoomEd numbers are now allowed as the parameter for the <code>give</code>, <code>kill</code> and <code>spawn</code> CCMDs.
* The translucency of the super shotgun's muzzle flash is now disabled when the <code>r_translucency</code> CVAR is off.
* A bug has been fixed whereby monsters would not respawn correctly when playing using the <i>Nightmare!</i> skill level.
* The shadow of the player's corpse is now removed when resurrecting using either the <code>resurrect</code> CCMD or the <code>iddqd</code> cheat.
* The console is now hidden when using the <code>iddqd</code> cheat to resurrect the player.
* The screen now goes to black sooner when starting <i>DOOM Retro</i>.
* <i>DOOM II's</i> cast sequence now works correctly when using <a target="_blank" href="//www.doomworld.com/vb/wads-mods/85991-smoothed-wip-smooth-monsters-for-doom-retro/"><i>SMOOTHED.WAD</i></a>.
* The console can no longer be opened during a screen wipe.
* The bottoms of things that bob in liquid now animate.
* A bug has been fixed whereby no monsters had lower pitches when the <code>s_randompitch</code> CVAR was enabled.
* The error message displayed when <code>doomretro.wad</code> is invalid has changed.
* <code>bfg</code> is now allowed as a parameter for the <code>spawn</code> and <code>give</code> CCMDs.
* A bug has been fixed whereby the player was unable to switch back to the selfie stick once obtained in <a target="_blank" href="//www.doomworld.com/idgames/combos/instadm"><i>InstaDoom</i></a>.
* The last savegame will no longer be automatically loaded when the player dies of the <code>pistolstart</code> CVAR is enabled.
* Individual monster kill counts used by the <code>playerstats</code> CCMD are now retained in savegames. Please note that this breaks savegame compatibility with previous versions of <i>DOOM Retro</i>.
* The speed of the player can now be changed using the <code>turbo</code> CVAR. Functioning like the command-line parameter of the same name, it can be a value between <code>10%</code> and <code>400%</code> and has a default of <code>100%</code>.
* A bug has been fixed (that is present in <i>Vanilla DOOM</i>) whereby the player would go in reverse when running if their speed was set to a value greater than <code>255%</code> using the <code>&ndash;turbo</code> command-line parameter.
* Monsters can now respawn when playing a skill level other than <i>Nightmare!</i> by using the <code>respawnmonsters</code> CCMD. The <code>&ndash;respawn</code> command-line parameter has also been reimplemented, and <code>&ndash;respawnmonsters</code> may also be used.
* The value set by <code>Max Health</code> in <i>DeHackEd</i> files and lumps is now only applied to health bonuses.
* 100 extra things (numbered 152 to 251) have been added for use in <i>DeHackEd</i> files and lumps.
* Things with the same (x,y,z) coordinates now bob in sync with each other if they are in liquid. 
* The arachnorbs in <a target="_blank" href="//www.doomworld.com/idgames/levels/doom2/Ports/megawads/valiant">valiant.wad</a> are now killed when using the <code>kill</code> CCMD.
* Map names changed using a <code>MAPINFO</code> lump are now shown in the output of the <code>maplist</code> CCMD.
* The <kbd>WINDOWS</kbd> key can no longer be used at any time when fullscreen. It can only be used when in a window, and the game is paused, or the menu or console is open.
* Wall textures that are both animated and translucent can now be rendered correctly without causing a crash.
* The <kbd>E</kbd> key may now be pressed as an alternative to <kbd>SPACE</kbd> to use doors, switches, etc. It is bound to the <code>+use2</code> action.
* When the <code>vid_showfps</code> CVAR is enabled, the frames per second is now displayed correctly while the screen shakes when the player is injured.

### DOOM Retro v2.0.5

* Bugs have been fixed whereby using <code>map next</code> in the console would warp the player to the next episode rather than the next map, and <code>map E<i>x</i>M<i>y</i></code> wouldn’t warp at all.
* 100 additional sprites, named <code>SP00</code> to <code>SP99</code> and numbered 145 to 244, have been added for use in <i>DeHackEd</i> lumps.
* The amount of negative health a monster must receive to be gibbed can now be changed using a <code>Gib health</code> parameter in <i>DeHackEd</i> lumps.
* An invalid character will no longer be displayed in the console when changing the music or SFX volume in the menu.
* A bug has been fixed whereby when adjusting the SFX volume in the menu, the music volume was being displayed in the console instead.

### DOOM Retro v2.0.4

* Using an <code>A_FireOldBFG</code> code pointer in a <i>DeHackEd</i> lump will no longer cause the game to freeze.
* The following improvements have been made to <a target="_blank" href="//www.drnostromo.com/hacx/"><i>HacX: Twitch ’n Kill</i></a> support:
  * The correct status bar is now displayed.
  * The projectiles of the nuker are no longer translucent.
  * The smoke trails have been removed from the projectiles of the photon ’zooka.
* The “B” in John Romero’s E1M8B.WAD is now displayed when the map starts, and in the automap.
* Dead players can now trigger actions that allow them to exit a map.
* The total number of monsters, and the percentage killed, are now displayed for each type of monster in the output of the <code>playerstats</code> CCMD.
* The position of keys when using a custom status bar has been corrected.
* CCMDs and CVARs now appear in the correct order when pressing the <kbd>TAB</kbd> key in the console to autocomplete.
* The brightmap for the <code>SW2METAL</code> wall texture has been fixed.
* <code>SLIME<i>xx</i></code> flats will no longer animate as liquid in EPIC2.WAD.
* A small icon is now shown next to each warning in the console.
* The <code>STARTUP5</code> string is now displayed correctly in the console when playing <a target="_blank" href="//freedoom.github.io/"><i>Freedoom</i></a>.
* The <code>SDL2_mixer.dll</code> file supplied with <i>DOOM Retro</i> is now compiled with <a target="_blank" href="//www.underbit.com/products/mad/"><i>libmad 0.15.1b</i></a>, fixing the tempo of some MP3 lumps. Consequently, <code>smpeg2.dll</code> is no longer required.
* A bug has been fixed whereby using the <code>map</code> CCMD would cause the game to crash in some instances.
* The selected episode or expansion in the menu is set as necessary when using the <code>map</code> CCMD.

### DOOM Retro v2.0.3

* “Pistol start” gameplay is now supported. By using the <code>pistolstart</code> CCMD, (or specifying <code>&ndash;pistolstart</code> on the command-line), the player’s health, armor, weapons and ammo will be reset at the start of each map. Also, a <code>PISTOLSTART</code> definition may now be used in <code>MAPINFO</code> lumps.
* The muzzle flash of the player’s rocket launcher has been fixed.
* The <code>+menu</code> action can now be bound to a key, with <code>esc</code> being the default.
* The <code>+console</code> action can now be bound to a key, with <code>tilde</code> being the default.
* The amount of bobbing has been reduced for higher values of the <code>stillbob</code> CVAR.
* A bug has been fixed whereby successive movement keys would not register if a cheat existed that started with a movement key (as is the case in <a target="_blank" href="//www.drnostromo.com/hacx/"><i>HacX: Twitch ’n Kill</i></a>).
* The armor bar in the alternate HUD is now slightly lighter.
* A bug has been fixed whereby map names from <code>MAPINFO</code> lumps weren’t being displayed.
* The map title and author of John Romero’s recently released E1M8B.WAD is now displayed when the map starts, and in the automap.
* The <code>GOTREDSKULL</code> string may now also be spelled as <code>GOTREDSKUL</code> in <i>DeHackEd</i> lumps.
* Pressing <kbd>ALT</kbd> + <kbd>F4</kbd> to quit <i>DOOM Retro</i> now works again.
* Stylized quotes are now used in place of double quotes in the console.
* Text in the console is now slightly translucent.
* A random static effect has been applied to the console’s background.
* The effects of changing the <code>vid_windowposition</code> and <code>vid_windowsize</code> CVARs while in the console and in a window is now immediate.

### DOOM Retro v2.0.2

* A rare bug has been fixed whereby the player’s view would continuously move or turn in one direction by itself.
* The <code>+run</code> action now works correctly when bound to a mouse button.
* The sound of a door closing is no longer played if the player walks over a line to trigger the door, and the door is already closed.
* It is now possible to warp to a map using <code>first</code>, <code>prev</code>[<code>ious</code>], <code>next</code> and <code>last</code> as the parameter for the <code>map</code> CCMD.
* A bug has been fixed whereby the muzzle flash of some weapons could be offset from the muzzle in some rare instances.
* The file <code>smpeg2.dll</code> is now included with <i>DOOM Retro</i> again.

### DOOM Retro v2.0.1

* A bug has been fixed whereby the screen wouldn’t stop shaking after the player was killed in some instances.
* The <code>+run</code> action may now be bound to a mouse button.
* The player’s weapon will no longer be fullbright while the player is injured.

### DOOM Retro v2.0

* Optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* <i>DOOM Retro</i> now uses the official releases of <a target="_blank" href="//libsdl.org"><i>SDL v2.0.4</i></a> and <a target="_blank" href="//libsdl.org/SDL_mixer"><i>SDL_mixer v2.0.1</i></a>.
* Alun “Viggles” Bestor’s Cacoward 2015 winner <code>BREACH.WAD</code> is now bundled with <i>DOOM Retro!</i>
* The mouse wheel controls to cycle through the player’s weapons has been inverted.
* Support has now been added for <code>TRANMAP</code> lumps.
* The warning stating a “music lump can’t be played” will no longer be displayed in the console when <code>&ndash;nomusic</code> or <code>&ndash;nosound</code> are specified on the command-line.
* The keys the player has already picked up are no longer reordered when using the <code>idkfa</code> cheat.
* Blood splats are no longer spawned on the ground when corpses are nudged by the player or a monster moving over them.
* A <code>s_randommusic</code> CVAR has been implemented. When it is enabled, music will be chosen randomly rather than using the default music for each map.
* The following changes have been made to the <code>playerstats</code> CCMD:
  * The percentage of map revealed is now slightly more accurate.
  * The amount of shots fired, the shots hit, and the weapon accuracy are now displayed.
  * The number of player deaths for the current map, not just the total player deaths, is now displayed.
  * The number of monsters killed is now broken down by the type of monster.
  * The CCMD may now be entered in the console when not playing a game.
* A <code>stillbob</code> CVAR has been implemented. When it is changed from its default of <code>0%</code>, the player will bob when idle.
* A <code>faceback</code> CVAR has been implemented. It may be used to change the background color of the player’s face in the status bar. The default is <code>5</code> (a dark gray), and can be set to a <a target="_blank" href="//doomwiki.org/wiki/PLAYPAL"><code>PLAYPAL</code> color index</a> between <code>0</code> and <code>255</code> inclusive.
* Pressing <kbd>SHIFT</kbd> + <kbd>[</kbd>, <kbd>SHIFT</kbd> + <kbd>]</kbd> or <kbd>SHIFT</kbd> + <kbd>&#92;</kbd> in the console will now display the correct characters.
* The error displayed when <code>am_external</code> is <code>on</code> and an external automap can’t be created is now only displayed once at startup rather than each time the graphics subsystem is reset.
* The external automap is now blurred when the main display is.
* The <code>pm_walkbob</code> CVAR has been renamed to <code>movebob</code>.
* The <code>pm_alwaysrun</code> CVAR has been renamed to <code>alwaysrun</code>.
* The <code>pm_centerweapon</code> CVAR has been renamed to <code>centerweapon</code>.
* A bug has been fixed whereby the texture offsets for sectors that change from liquid to solid weren’t reset.
* An error is now displayed in the console if pressing the printscreen key fails to take a screenshot.
* Pillarboxes are now cropped from screenshots.
* Rudimentary support has now been added for <code>MAPINFO</code> lumps. <code>MUSIC</code>, <code>NEXT</code>, <code>PAR</code>, <code>SECRETNEXT</code>, <code>SKY1</code> and <code>TITLEPATCH</code> keywords are supported. Additionally, the following keywords are supported specific to <i>DOOM Retro</i>:
  * <code>AUTHOR</code> <i>author</i>: Display the author’s name in the console when the map starts.
  * <code>LIQUID</code> <code>"</code><i>flat</i><code>"</code>: Specify a flat that will be treated as liquid in the map.
  * <code>NOLIQUID</code> <code>"</code><i>flat</i><code>"</code>: Specify an animated flat that won’t be treated as liquid in the map.
* <i>DOOM Retro</i> now has an alternate widescreen heads up display, inspired by the new <i>DOOM</i> released on May 13, 2016. It is disabled by default, and can be enabled using the <code>r_althud</code> CVAR. Widescreen mode, (displayed by pressing the <kbd>+</kbd> key to increase the screen size during a game, or through the options menu), needs to be enabled for it to be displayed.
* The player’s view no longer shifts at the start of a map when in windowed mode.
* The background is now redrawn whenever pressing the <kbd>ENTER</kbd> key in the console.
* Support is now included for MOD, XM, IT, S3M and FLAC music lumps.
* The pitch of barrel explosions is no longer randomized when the <code>s_randompitch</code> CVAR is on.
* Green marine corpses are now randomly colored. This feature may be disabled using the <code>r_corpses_color</code> CVAR.
* The message displayed when a gamepad is detected is now only displayed once in the console.
* A bug has been fixed that stopped a door from opening in <code>MAP10</code> of <code>DOOM2.WAD</code>.
* The vertical position of the large digits in the status bar has now been fixed when using an <code>STBAR</code> lump from a PWAD.
* The z-height of line attacks when in liquid sectors is no longer adjusted.
* Monsters now recognize when they are standing on <i>BOOM</i>-compatible lifts.
* Corpses are now nudged with more momentum when they are in liquid.
* Pain Elementals will no longer appear to open and close their mouth for no reason. They will now still try to spawn Lost Souls that won’t fit in the map, but they will explode instantly.
* Whether Lost Souls spawned by Pain Elementals are above the ceiling or below the floor is now checked.
* The possibility of a key flashing when switching to the widescreen HUD has been fixed.
* Ceilings are now marked in the automap as no longer secret once they move.
* Flashing elements in the widescreen HUD are now paused correctly when the game is paused.
* Improvements have been made to the rendering of really long walls.
* A bug has been fixed whereby <i>BOOM</i> general crushers were not able to be triggered by walkover.
* Some animated flats in <code>BTSX_E1.WAD</code>, <code>ETERNALL.WAD</code>, <code>FREEDOOM.WAD</code> and <code>TVR!.WAD</code> are no longer incorrectly displayed as liquid.
* The maximum number of mouse buttons supported has been increased from 8 to 16.
* The following CVARs have been implemented to allow the colors in the automap to be changed:
  * <code>am_allmapcdwallcolor</code>
  * <code>am_allmapfdwallcolor</code>
  * <code>am_allmapwallcolor</code>
  * <code>am_backcolor</code>
  * <code>am_cdwallcolor</code>
  * <code>am_fdwallcolor</code>
  * <code>am_gridcolor</code>
  * <code>am_markcolor</code>
  * <code>am_playercolor</code>
  * <code>am_teleportercolor</code>
  * <code>am_thingcolor</code>
  * <code>am_tswallcolor</code>
  * <code>am_wallcolor</code>
  * <code>am_xhaircolor</code>
* The space between words in the console has been condensed.
* A bug has been fixed whereby the scale filter wouldn’t change straight away if the <code>vid_scalefilter</code> CVAR was changed to an empty string (the default) in the console.
* The player’s weapon is no longer centered upon firing if it's state's <code>misc1</code> or <code>misc2</code> values are set in a <i>DeHackEd</i> patch. This fixes an issue with <a target="_blank" href="//www.doomworld.com/idgames/combos/instadm">InstaDoom</a>.
* The <code>r_detail</code> CVAR can now be set correctly in the console.
* The maximum number of blood splats that can appear in a map can no longer be unlimited. The default of the <code>r_bloodsplats_max</code> CVAR is now <code>32768</code>.
* Some CVAR descriptions in the output of the <code>cvarlist</code> CCMD now span over 2 lines.
* The menu can no longer be opened while the console is closing.
* The gamma correction level is now calculated correctly.
* The parameter for the <code>map</code> CCMD when using <i>Freedoom</i> can now be of the format <code>C<i>x</i>M<i>y</i></code>.
* The gradual light effect that is applied to opening and closing door sectors is no longer applied to sectors that have light specials.
* When entering the <code>idkfa</code> cheat, if a keycard and a skull key of the same color are present in the map, only the keycard is given to the player.
* A bug has been fixed whereby updating the <code>gp_sensitivity</code> CVAR in the console was having no immediate effect.
* Changes have been made to how gamepad sensitivity is calculated.
* The misalignment of patches with negative horizontal offsets has been fixed. This issue was evident in some textures, such as <code>BIGDOOR7</code>, and some switches in <code>BTSX_E1.WAD</code>.
* If the same CVAR is changed more than once in succession, there will now only be one line of output in the console.
* The status bar’s background has been enhanced.
* Timestamps in the console are now updated to the newest message when player messages are combined.
* The small digits used in the status bar and for marks in the automap are now consistent with those used in the console.
* A bug has been fixed whereby if the <code>r_gamma</code> CVAR was set to 2.0, the gamma correction level would still be set to the default of 0.75 at startup.

### DOOM Retro v1.9

* <i>DOOM Retro</i> now allows the automap to be shown on a second display. This feature may be enabled using the <code>am_external</code> CVAR, and will display a fullscreen 16:10 automap in the first display it finds that is not the main display set by the <code>vid_display</code> CVAR. While this external automap is displayed, the <kbd>TAB</kbd> key is disabled, and the <code>iddt</code> cheat can be entered at any time. Also, the automap’s usual controls are unavailable, but the grid and rotate mode may still be toggled in the console using the relevant CVARs.
* Optimizations have been made to further improve the overall performance of <i>DOOM Retro</i>.
* A new filter is now available to scale <i>DOOM Retro</i> onto the display. It is enabled by changing the value of the <code>vid_scalefilter</code> CVAR to <code>"nearest_linear"</code>, and is a combination of the existing two filters, <code>"nearest"</code> (which uses nearest-neighbor interpolation, the default) and <code>"linear"</code> (which uses linear filtering).
* The screen will no longer “bleed” along the edges when the <code>vid_scaledriver</code> CVAR is set to <code>""</code> or <code>"direct3d"</code> and the <code>vid_scalefilter</code> CVAR is set to <code>"linear"</code> on some systems.
* A bug has been fixed whereby screenshots couldn’t be taken by pressing the <kbd>PRINTSCREEN</kbd> key if characters that can’t be used in a filename were present in the current map’s title.
* A disk icon (the <code>STDISK</code> lump) is now displayed in the top righthand corner of the screen when saving and loading a game, and when loading a map. It may be disabled using the <code>r_diskicon</code> CVAR.
* A <code>nomonsters</code> CCMD has been implemented in the console which will toggle whether monsters are spawned or not in the next map. This CCMD will override when <code>&ndash;nomonsters</code> is specified on the command-line.
* The entire screen will now shake when the player is injured. This feature may be disabled using the <code>r_shakescreen</code> CVAR.
* Several improvements have been made to the support of DeHackEd patches and files.
* If a savegame fails to load, the menu will now close and the console will open displaying an error explaining why.
* A crash will no longer occur when trying to open a map that contains only one subsector.
* A bug has been fixed whereby some CVARs weren’t being validated at startup.
* Timestamped obituaries can now be displayed in the console each time something is killed, exploded or resurrected during a game. They are disabled by default, and may be enabled by using the <code>con_obituaries</code> CVAR.
* A bug has been fixed whereby a HOM could appear in maps that use texture 0 as a placeholder.
* Changing the <code>r_liquid_swirl</code> CVAR will now always update visible liquid sectors immediately.
* The darkest of the gradually transitioning lighting under doors is now slightly darker than all adjacent sectors to make the feature more prominent.
* Blood splats are no longer left behind if Lost Souls are close enough to the ground when the <code>kill</code> CCMD is used to kill them.
* <code>nerve.wad</code> will now be recognized correctly when specified using <code>&ndash;file</code> on the command-line.
* A bug has been fixed whereby entering a cheat in the console could stop player messages from appearing.
* The <code>iwadfolder</code> CVAR will now be set correctly when more than one PWAD is selected in the WAD launcher.
* Map titles that have been changed using a DeHackEd file or lump will be capitalized as necessary.
* “DOOM RETRO” is now displayed as “DOOM Retro” in all instances.
* The output displayed in the console at startup regarding <code>doomretro.cfg</code> now distinguishes between the file not being present because <i>DOOM Retro</i> is being run for the first time, or because the file is missing.
* Grammar has been fixed in the output in the console when only one lump is present in a PWAD.
* The game will no longer crash when <code>&ndash;nosfx</code> or <code>&ndash;nosound</code> are specified on the command-line.
* A bug has been fixed whereby floating and bobbing items would get stuck in the floor or ceiling in some instances.
* The error displayed when an invalid <code><i>+action</i></code> is passed to the <code>bind</code> CCMD has been fixed.
* Which WAD the <code>COLORMAP</code> is being used from is now output to the console at startup.
* Spacing may now be used in the parameters passed to the <code>kill</code> and <code>spawn</code> commands in the console. For example, <code>spawn baron of hell</code> can be used as well as <code>spawn baronofhell</code>.
* The UK English spelling of “armor” (that is, “armour”) may now be used as the parameter for CCMDs that allow it.
* Entering the <code>kill</code> CCMD without any parameters will no longer kill the player, and instead will display a list of parameters. To kill the player, now use <code>kill player</code>.
* The <code>give</code> CCMD can now also be used to give the player anything that they can pick up during a game. For example, <code>give berserk</code> and <code>give soulsphere</code>.
* The console is now automatically closed when using the <code>give</code> CCMD.
* A bug has been fixed whereby corpses weren’t sliding even when the <code>r_corpses_slide</code> CVAR was <code>on</code>.
* The intensity of the red screen tint when the player has a berserk power-up and their fists up has been reduced slightly.
* The red screen tint when the player is injured, and the gold tint when the player picks up something will now show through while the player has a berserk power-up and their fists up.
* The <code>r_altlighting</code> CVAR has been removed.
* A slight current is now applied to liquid sectors, in a random direction determined at the start of each map. It may be disabled using the <code>r_liquid_current</code> CVAR.
* A bug has been fixed whereby the bottom wall texture between adjacent liquid sectors would show through in some instances.
* The player’s weapon sprite will no jump slightly when switching to and from the automap while moving.
* Although a majority of animated flats in <i>DOOM</i> are liquid, in some PWADs there are some that are not. There are now several instances in some popular PWADs where <i>DOOM Retro’s</i> liquid effects won’t be applied.
* The <code>r_lowpixelsize</code> CVAR will now be correctly parsed at startup.
* The <code>r_lowpixelsize</code> CVAR can now be set to values of <code>2×1</code> and <code>1×2</code>.
* The amount of blood splats spawned when corpses slide along the ground has been halved to 256.
* A bug has been fixed whereby generalized floors could become stuck after loading a savegame.
* The help screen has been updated to show that the <kbd>~</kbd> key opens the console.
* All instances of “DOOM” and “DOOM Retro” that appear in the console and the splash screen have been italicized.
* If an IWAD is specified on the command-line using <code>&ndash;iwad</code> but with no path, <i>DOOM Retro</i> will now check for an installation of <i>DOOM</i> purchased through <a target="_blank" href="//www.gog.com/game/the_ultimate_doom">GOG.com</a>.
* If a PWAD file is loaded, savegames are now separated based on that file, rather than the IWAD file. (Because of this, and also because a change in how the folders savegames are saved into are named, savegames from previous versions of <i>DOOM Retro</i> will no longer appear in the save and load game menus.)
* The <code>savegamefolder</code> CVAR has been removed. Instead, the folder savegames are saved to and loaded from is displayed in the console at startup.
* A bug has been fixed whereby if the player died in a sector with special 11, they would appear in the next map with zero health and be unable to pick up items or alert enemies.
* The default mouse sensitivity has been increased from 16 to 24.
* Shadows are now opaque when the <code>r_translucency</code> CVAR is <code>off</code>.
* The pitch of the player’s sounds is no longer randomized when the <code>s_randompitch</code> CVAR is <code>on</code>.
* Generalized linedefs without tags are now applied locally, rather than not working at all.
* The positions that blood splats are spawned are now more accurate when blood falls to the ground.
* The <code>%</code> symbol will no longer disappear from the end of the <code>pm_walkbob</code> CVAR’s value.
* The CVAR names will no longer be shown when entering the <code>gp_deadzone_left</code> or <code>gp_deadzone_right</code> CVARs to display their values.
* <i>DOOM Retro</i> will now play MP3 and Ogg music lumps. This requires the files <code>libogg-0.dll</code>, <code>libvorbis-0.dll</code>, <code>libvorbisfile-3.dll</code> and <code>smpeg2.dll</code> all to be in the same folder as <code>doomretro.exe</code>.
* A warning is now displayed in the console when a music lump can’t be played.
* Tilde characters are now removed from the files saved using the <code>condump</code> CCMD.

### DOOM Retro v1.8.5

* More than one instance of <code>&ndash;file</code> may now appear on the command-line.
* The amount of “map revealed” in the output of the <code>playerstats</code> CCMD is now always calculated correctly.
* The number of times the player cheats, both in the current map and overall, as well as the overall amount of time spent playing <i>DOOM Retro</i>, are now displayed in the output of the <code>playerstats</code> CCMD.
* <i>BOOM’s</i> <code>MF_TRANSLUCENT</code> flag is now supported in DeHackEd lumps and files.
* When binding an action to a control using the <code>bind</code> CCMD, any other actions that are bound to that same control will now be unbound.
* A bug has been fixed whereby the mouse pointer wouldn’t be released when pressing <kbd>ALT</kbd>&nbsp;+&nbsp;<kbd>TAB</kbd> to switch to the desktop.
* The game will now pause slightly when the player uses a switch to exit a map, to stop the switch’s sound from stuttering.
* Support has been added for certain hacks to the <code>NODE</code> lump of a map. See <a href="//doomwiki.org/wiki/Linguortal">here</a> for more information.
* The chaingunner’s refire frame is now fullbright.

### DOOM Retro v1.8.4

* Entering the <code>playername</code> CVAR without a value will now display the value it is currently set to.
* Gradual lighting is now applied to the sectors under all doors as they open and close, similar to the effect introduced in <i>BOOM</i>.
* The player is no longer gibbed when their corpse is under a door.
* Whether there are <i>BOOM</i> line specials present in maps with <i>DeepBSPv4</i> extended nodes is now displayed correctly in the output of the <code>mapstats</code> CCMD.
* The correct author is now displayed in the output of the <code>mapstats</code> CCMD.
* The effects of enabling the <code>s_randompitch</code> CVAR is now immediate.
* A bug has been fixed whereby the game could crash when trying to render the fuzz effect while paused in some rare instances.
* Whether the player’s view is lowered when in liquid sector is now toggled by changing the new <code>r_liquid_lowerview</code> CVAR. Now the existing <code>r_liquid_clipsprites</code> will only toggle whether the bottom of sprites are clipped in liquid, as expected.
* A bug has been fixed whereby messages wouldn’t appear in some instances.
* Tabs are now converted to spaces in the file created by the <code>condump</code> CCMD.
* Although quite often the same folder, the file created by the <code>condump</code> CCMD will now be put in the same folder as the executable, rather than the current working folder.
* Timestamps are now displayed next to each player message in the console. They may be disabled using the new <code>con_timestamps</code> CVAR.
* The <code>btsx_e1.wad</code> PWAD may now be used with <code>freedoom2.wad</code> as the IWAD without crashing.
* The game will no longer crash when trying to display patches with dimensions larger than 320×200.
* The game will no longer crash when trying to bind an action to an invalid control using the <code>bind</code> CCMD. And now a warning will be displayed in the console advising that it couldn’t be bound.
* The <code>+alwaysrun</code> action can now be bound to a gamepad button.

### DOOM Retro v1.8.3

* A bug has been fixed whereby some platforms would move silently.
* The brightmap for walls with the <code>SW2BRN2</code> texture has been fixed.
* Lifts will no longer often become stuck when restoring a savegame.
* A bug has been fixed whereby the player wouldn’t die in some very rare instances when a “voodoo doll” was present in the map.
* Sectors that don’t start as liquid will now always sync with sectors that do when <code>r_liquid_bob</code> is <code>on</code>.
* Additional momentum is no longer applied to barrels in liquid.
* Music will now start after a map is loaded, and not before.
* The <code>playerstats</code> CCMD now shows the amount of damage the player has inflicted and received, and the number of times they have died, both in the current map, and in total. Accumulative totals of the number of items picked up, the monsters killed and the secrets found are also displayed.
* A bug has been fixed whereby the game would crash when trying to draw teleport lines in the automap in some instances.
* The game will no longer crash when trying to use the <kbd>F9</kbd> to quickload a game in some rare instances.

### DOOM Retro v1.8.2

* Although quite often the same folder, <i>DOOM Retro</i> will now put savegames in the same folder as the executable, rather than the current working folder.
* A bug has been fixed whereby sprites would appear through closed doors in some instances.

### DOOM Retro v1.8.1

* The width of the text is now checked before pasting in the console.
* <i>DOOM Retro</i> will now look in the same folder as the executable, rather than the current working folder, to find <code>doomretro.wad</code> and <code>doomretro.cfg</code>.
* Extra mouse buttons are now handled correctly.
* The alternate lighting of the player’s weapon may now be disabled using the new <code>r_altlighting</code> CVAR.
* The alternate lighting of the player’s weapon is now slightly lighter.
* A bug has been fixed whereby the player’s weapons would start to cycle when entering the <code>idbehold</code> cheat.
* Whether Windows 10 is the Home edition or not is now shown in the console at startup.
* The revision number of <code>sdl2.dll</code> is now shown in the console at startup.
* The default of the <code>vid_windowsize</code> CVAR is now <code>768x480</code>, giving the window a 16:10 aspect ratio.
* All liquid sectors are now slightly lower.
* <i>BOOM</i> elevators no longer shudder when they move.
* The liquid swirl animation is no longer applied to flats using the <code>SLIME09</code> texture.
* The liquid swirl animation is now applied to flats using the <code>SLIME05</code> texture.
* Brightmaps will no longer be rendered when the player has an invulnerability powerup, or in areas with a <i>BOOM</i> colormap.
* A crash will no longer occur when a <i>BOOM</i> pusher or puller thing is present in a map.

### DOOM Retro v1.8

* An extensive number of optimizations have been made to improve the overall performance and stability of <i>DOOM Retro</i>.
* <i>DOOM Retro</i> is now compiled using <a target="_blank" href="//www.visualstudio.com/vs-2015-product-editions">Microsoft Visual Studio Community 2015</a>. Visual Studio’s runtime library is now statically linked to the binary, meaning it doesn’t need to be installed.
* <i>DOOM Retro</i> now uses a prerelease version of <a target="_blank" href="//www.libsdl.org/">SDL v2.0.4</a>.
* <i>DOOM Retro</i> now supports <i>BOOM</i>-compatible maps with the following features:
  * Deep water effects.
  * Scrolling walls, floors and ceilings.
  * Translucent walls.
  * Friction effects.
  * Custom colormaps.
  * Support for the ANIMATED lump.
  * Silent teleporters.
  * Elevators.
  * Generalized linedef types.
* The lighting of the player’s weapon is now slightly darker and more dynamic.
* Direct3D is now the default renderer, which does not have the flicker that occurs with the OpenGL renderer in fullscreen. As before, the renderer can be changed using the <code>vid_scaledriver</code> CVAR.
* The pillarboxes and letterboxes on the screen are now cleared each frame.
* Configuration files (with the extension <code>*.CFG</code>) may now be loaded through the WAD launcher.
* If a CFG file exists in the same folder as a PWAD file with the same name, it will automatically be loaded.
* Support has been added for maps with <i>DeepBSP</i> extended nodes v4 and <i>ZDoom</i> uncompressed normal nodes.
* Several rendering anomalies in maps have been resolved.
* Any flats that are missing in a map will now be rendered as sky, and a warning displayed in the console, rather than <i>DOOM Retro</i> exiting with an error.
* Further improvements have been made to the support for <i>DeHackEd</i> lumps.
* The translucency of the chaingun’s muzzle flash has been improved slightly.
* The “always run” feature may now be bound to a key other than <kbd>CAPSLOCK</kbd> in the console by using the <code>+alwaysrun</code> action with the <code>bind</code> CCMD.
* Movement of the player’s weapon is now interpolated to appear smoother.
* Rather than using the standard animation, which is only updated every 125 milliseconds, a much smoother swirl effect is now applied to every liquid sector. It is on by default, and can be turned off using the <code>r_liquid_swirl</code> cvar.
* The speed of liquid sectors bobbing up and down has now been doubled.
* Things in liquid sectors no longer bob in time with each other.
* If the blockmap of a map is invalid or not present, it will now be recreated.
* The position of keycards and skull keys in the widescreen HUD when the player has no armor has been improved.
* The input in the console will now be restored after viewing the input history using the <kbd>&uarr;</kbd> key.
* The <code>r_playersprites</code> cvar has now been implemented allowing the player’s weapon to be hidden.
* Several changes have been made to the descriptions of CCMDs and CVARs when using the <code>cmdlist</code> and <code>cvarlist</code> CCMDs in the console.
* A new <code>mapstats</code> CCMD has been implemented that will show the following information about the current map: map title, map author, node format, if the blockmap was recreated, total vertices, total sides, total lines, if <i>Boom</i>-compatible line specials are present, total sectors, total things, map size and music title.
* The <code>r_maxbloodsplats</code> CVAR has been renamed to <code>r_bloodsplats_max</code>. Also, when it is set to <code>0</code>, it will now be shown as <code>0</code> rather than <code>off</code>.
* The <code>totalbloodsplats</code> CVAR has been renamed to <code>r_bloodsplats_total</code>.
* The <code>r_mirrorweapons</code> CVAR has been renamed to <code>r_mirroredweapons</code>.
* The <code>mapfixes</code> CVAR has been renamed to <code>r_fixmaperrors</code>.
* The <code>spritefixes</code> CVAR has been renamed to <code>r_fixspriteoffsets</code>.
* A bug has been fixed whereby weapons spawned at the start of a map weren’t being randomly mirrored if <code>r_mirroredweapons</code> was <code>on</code>.
* The format of <code>doomretro.cfg</code> has changed considerably, and is divided into two parts: cvars and bindings.
* Tall textures are now supported.
* The wall texture between two liquid sectors (often an animated waterfall texture) will no longer rise and fall with those sectors.
* The randomness of such things as mirrored corpses and the spawning of blood splats has been improved.
* The positions of the numbers in the widescreen HUD are now improved when custom lumps are used.
* The translucency of the elements in the widescreen HUD has been increased slightly.
* The health, ammo and armor counts in the widescreen HUD will now flash briefly when the player picks up the corresponding items during a game.
* The game will now exit with an error if no subsectors are present in a map.
* Improvements have been made to the consistency by which blood splats are spawned when a monster is shot, and when a corpse slides along the floor.
* A feature is now available that randomizes the pitch of monster sounds. It is disabled by default, and can be enabled using the <code>s_randompitch</code> CVAR.
* If a PWAD is loaded, the window caption will now be changed to its filename while no map is loaded.
* A bug has now been fixed whereby the operation of the mousewheel to select the previous/next weapon was reversed, and would no longer work at all if the user attempted to change it using the <code>bind</code> CCMD.
* The console now opens and closes slightly faster.
* The background of the console now has a slight diagonal pattern, and a drop shadow.
* The scrollbar track and dividers in the console are now translucent.
* If a PWAD is loaded that uses a custom character set, the color of the player messages in the console will now reflect the color of those characters.
* Widescreen mode will now be enabled or disabled correctly when setting the <code>vid_widescreen</code> CVAR.
* The contents of the window now updates dynamically as it is being resized.
* A bug has been fixed whereby the screen size couldn’t be adjusted in the options menu when not in a game.
* The mouse pointer is now released while the console is open.
* The window caption will no longer be reset to “DOOM RETRO” when the graphics subsystem is restarted by entering certain cvars in the console.
* The blink rate of the text caret in the console is now the same speed as the <i>Windows</i> setting.
* The window is no longer reset to a 4:3  aspect ratio at startup.
* The position of the window is now restored correctly at startup, and when switching from fullscreen mode, if using multiple displays.
* The minimum size that the window can be resized to is now 320×240.
* The console is now closed when pressing the close button in the window’s title bar.
* If a masked texture is used on a one-sided line, the transparent parts will now be displayed as black rather than randomly-colored pixels. Code by Fabian Greffrath.
* Autocomplete and input history are now reset if a character is deleted in the console.
* The output in the console is now correct when the music and SFX volumes are changed in the menu.
* The graphics subsystem will now be reset when the <code>vid_display</code> CVAR is changed in the console, so displays can now be switched during a game.
* If the <code>vid_display</code> CVAR is found to be invalid at startup, it will no longer be restored to its default, in case the display it points to happens to be off. Instead, a warning will be displayed in the console, and display 1 will be used.
* An acronym for the screen resolution, and the correct aspect ratio, will now be displayed in the console at startup.
* Whether <i>Windows</i> is 32 or 64-bits will now be displayed in the console at startup.
* A small amount of ammo is now given to the player when using the <code>give backpack</code> CCMD, to be consistent with what the player is given when picking up a backpack during a game.
* A bug has been fixed whereby an additional character could be entered into a cheat sequence in some instances.
* The use of a <i>TiMidity</i> configuration file is now displayed in the console at startup.
* <code>MAP05C</code> and <code>MAP16C</code> in <i>Back To Saturn X Episode 2</i> may now be loaded using the <code>map</code> CCMD.
* Monsters will no longer be alerted when the player makes a noise while “No Target” mode is on using the <code>notarget</code> CCMD.
* The <code>s_maxslicetime</code> CVAR has been removed.
* Since it produces the same result as using <code>linear</code>, the <code>anisotropic</code> value for the <code>vid_scalefilter</code> CVAR has been removed.
* A bug has been fixed whereby the <code>kill</code> CCMD wasn’t killing the player when they had armor.
* A bug has been fixed whereby pressing the media keys on some keyboards would cause the player to change weapons.
* The <code>showitems</code>, <code>showkills</code>, <code>showmapped</code> and <code>showsecrets</code> CCMDs, as well as the <code>maptime</code> CVAR, have all been combined into one <code>playerstats</code> CCMD.
* The <code>com_showfps</code> CVAR has been renamed to <code>vid_showfps</code>.
* The <code>com_showmemoryusage</code> CVAR has been removed.
* The <code>r_liquid_animatedheight</code> CVAR has been renamed to <code>r_liquid_bob</code>.
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
* <code>warp</code> can now be used as an alternative to the <code>map</code> CCMD.
* A feature has been implemented that causes corpses to be moved slightly if a monster walks over them. It is enabled by default, and may be disabled using the <code>r_corpses_nudge</code> CVAR.
* A bug has been fixed whereby monsters would sometimes fall off tall ledges.
* Friction in liquid is now only reduced for corpses.
* Active crushers will now be active again when loading a savegame.
* Improvements have been made to the player using a switch, lift or door if very close to another.
* The display of the value of the <code>r_lowpixelsize</code> CVAR in the console has been fixed.
* There is no longer any blue or green blood or blood splats in <i>Freedoom</i>.
* A bug has been fixed whereby no maps were listed by the <code>maplist</code> CCMD in <i>Freedoom</i>.
* Blues are now emphasized better in translucent sprites.
* The console now automatically closes when using the <code>map</code> CCMD or the <code>idclev</code> cheat.
* The HOM inidicator is now paused while the console is open.
* A bug has been fixed whereby warnings weren't being displayed in the console.
* The <kbd>WINDOWS</kbd> key is now only disabled during a game, and not while the game is in a menu, paused, in the console, or on the title screen.

### DOOM Retro v1.7.1

* Optimizations have been made to further improve the overall performance of <i>DOOM Retro</i>.
* The stray brown pixel in the super shotgun’s sprite has now been removed from all the frames that it appears.
* Red blood splats are now slightly darker.
* A warning is now displayed in the console when a map contains an unknown object.
* Warnings are now displayed in the console when the player enters a sector, crosses a line, or shoots a switch with an unknown special.
* Sounds are now made again when liquid sectors move up or down.
* The file <code>doom2.wad</code> is no longer autoloaded if the <code><span style="white-space:nowrap">&ndash;FILE</span></code> command-line parameter without <code><span style="white-space:nowrap">&ndash;IWAD</span></code>.
* A warning is now displayed in the console if the versions of <code>sdl2.dll</code> and <code>SDL2_mixer.dll</code> don’t match the versions that <i>DOOM Retro</i> was compiled with.
* A bug has been fixed whereby the game would crash when trying to render sprites that had been manipulated with DeHackEd.
* The “screen shot” string may now be changed with DeHackEd.
* Several further improvements have been made to DeHackEd support.
* The <kbd>HOME</kbd> and <kbd>END</kbd> keys will now scroll to the top and bottom of the console output, if the player has started scrolling up with the <kbd>PGUP</kbd> key.
* A bug has been fixed whereby the game would crash when using the <code>bind</code> command in the console to bind an action to some keyboard controls.
* <code>save</code> and <code>load</code> commands have now been implemented in the console.
* Savegames are now saved with a file extension of <code>.save</code> rather than <code>.dsg</code>.
* The new <code>savegamefolder</code> CVAR shows where savegame files are saved.
* The new <code>totalbloodsplats</code> CVAR shows the total number of blood splats currently in the map.
* The <code>r_bloodsplats</code> CVAR has been renamed to <code>r_maxbloodsplats</code>.
* If the player saves a game on one map, finishes that map and then starts the next one, if they then die without saving again, the savegame for the previous map will be loaded, rather than the player “pistol starting” on the new map.
* The Arch-vile’s fire attack is no longer clipped in liquid.
* A bug has been fixed in some instances whereby some objects were still being clipped and bobbing when the sector they were in was no longer liquid.
* The new <code>thinglist</code> CCMD will list all things in the current level, including their (x,y,z) coordinates.
* The new <code>maplist</code> CCMD will list all the maps available to the player.
* Graphics will now automatically be reset if the <code>vid_screenresolution</code> is changed to <code>desktop</code>.
* The player’s view height is now smooth when descending between two liquid sectors of different heights.
* The widescreen HUD will now remain on the screen while the player is dead. No ammo will be displayed, though, to be consistent with the status bar.
* The par time displayed on the intermission screen is now positioned better.
* The normal use of the <kbd>CAPSLOCK</kbd> key is now disabled in the console, and will toggle the “always run” feature on/off instead.
* A bug has been fixed whereby the value displayed by the <code>totalkills</code> command in the console wouldn’t take into account any Pain Elementals killed using the <code>kill</code> command.
* The console is now automatically closed when using the <code>endgame</code>, <code>exitmap</code>, <code>kill</code> and <code>map</code> CCMDs.
* A bug has been fixed whereby the map name in the automap was displayed incorrectly when using <i>ZDL</i> to launch the game.
* The “fuzzy” edges of Spectre shadows are now paused while the console is open.

### DOOM Retro v1.7

* <i>DOOM Retro</i> now uses <a target="_blank" href="//www.libsdl.org/">SDL 2.0.3</a> for its graphics and audio. This brings significant performance improvements, as the screen is now scaled using hardware acceleration when possible, as well as the following features:
  * The monitor to display the game on can now be specified.
  * Vsync can now be toggled on/off.
* <i>DOOM Retro</i> now includes a console. It may be opened at any time by pressing the <kbd>~</kbd> key.
* <i>DOOM Retro</i> now has uncapped framerates, and through use of interpolation, player, monster and sector movement is much smoother.
* Several memory leaks have been fixed, making <i>DOOM Retro</i> more stable.
* A bug has been fixed whereby if a PWAD was selected in the WAD launcher that didn’t contain any map data, <i>DOOM Retro</i> would give up and not look at the other PWADs selected.
* If none of the PWADs selected in the WAD launcher contain any map data, and no IWAD is selected, <code>DOOM2.WAD</code> will try to be loaded.
* Many performance optimizations have been made to the rendering of blood splats and shadows.
* The following changes have been made to animated liquid sectors:
  * They are all slightly higher.
  * The edge where two adjacent liquid sectors at different heights meet is now rendered correctly.
  * The player will now move smoothly when ascending from one liquid sector to a higher one.
  * No sounds are made when they move up or down.
* A bug has been fixed whereby if more than one flight of stairs was meant to be triggered at the same time, only one would be.
* The gaps around “1” digits in the HUD have been removed.
* A bug has been fixed whereby the game could crash when rendering Spectres in some instances.
* Underscores now appear under the message displayed when entering the <code>IDBEHOLD</code> cheat.
* The map title will now be displayed correctly in the automap if a PWAD is loaded using ZDL.
* Corrupt savegames will no be created if saving a game while a button is active.
* Blood splats and shadows will no longer appear on sectors without floors.
* The blood splats produced when the corpses of Barons of Hell and Hell Knights are crushed under a lowering sector are now the correct color.
* The player’s weapon is no longer off to the right by 1 pixel in some instances.
* The bottom righthand corner of the view border is now rendered correctly.
* The message that is displayed when <i>DOOM Retro</i> is run for the first time now includes a button that opens the <i><a target="_blank" href="//wiki.doomretro.com">DOOM Retro Wiki</a></i> in the default browser.
* The cursor keys will no longer make a sound when pressed on the help screen.
* A bug has been fixed whereby decorative corpses wouldn’t smear blood when sliding in some instances.
* Smoke trails are now displayed for Cyberdemon rockets as originally intended.
* The “always run” setting is now remembered between games as originally intended.
* If you load a savegame that had monsters but now the <code><span style="white-space:nowrap">&ndash;NOMONSTERS</span></code> command-line parameter is enabled, the correct percentage of monsters you actually did kill now appears in the intermission.
* A bug has been fixed whereby <i>DOOM Retro</i> would crash when trying to save a game in <i>Final DOOM</i> if the savegame description was changed to anything other than a map name.
* Additional blood splats spawned under decorative corpses when a map is started now won't be randomly shifted away from the corpse if the corpse is hanging from the ceiling.
* Several improvements have been made to <i>DOOM Retro’s</i> support of DeHackEd lumps and files.
* Now BEX files as well as DEH files, with the same name and in the same folder as the PWAD selected in the WAD launcher, will now be automatically loaded.
* DEH files are no longer automatically loaded if a PWAD is loaded from the command-line. They will need to be explicitly loaded using the <code><span style="white-space:nowrap">&ndash;DEH</span></code> command-line parameter.
* A bug has been fixed whereby some teleporters in <i>Back To Saturn X</i> were animating as if they were liquid.
* The position of the player arrow is now drawn much more accurately when in the automap and rotate mode is on.
* The automap will no longer disappear, nor the game crash, when zooming out in very large maps.
* Translucency is now applied to MegaSpheres as originally intended.

### DOOM Retro v1.6.7

* A bug has been fixed whereby <i>DOOM Retro</i> would crash at startup when trying to run in a screen resolution that wasn’t widescreen.
* <i>DOOM Retro</i> will no longer crash after successive presses of <kbd>ALT</kbd> + <kbd>ENTER</kbd> to switch between fullscreen and windowed modes.
* Ceilings that use liquid textures will now render correctly.
* An error will now be displayed at startup if a WAD file contains DeePBSP or ZDBSP nodes.
* The <code>saturation</code> setting in <code>doomretro.cfg</code> has been deprecated.
* A bug has been fixed whereby the shadows of dropped items weren’t also being mirrored when the <code>mirrorweapons</code> setting was <code>true</code> in <code>doomretro.cfg</code>.
* Weapons spawned at the start of a map are now also mirrored when the <code>mirrorweapons</code> setting was <code>true</code> in <code>doomretro.cfg</code>.
* Thanks to some <a target="_blank" href="//www.doomworld.com/vb/post/1340126">excellent coding</a> from Linguica, entryway and kb1, <i>Vanilla DOOM’s</i> <a target="_blank" href="//doomwiki.org/wiki/Long_wall_error">“long wall error”</a> has been fixed.
* Further optimizations have been made to improve the overall performance and stability of <i>DOOM Retro</i>.
* Teleporters in <i>Back to Saturn X</i> are now drawn correctly in the automap before they have been triggered.
* Whether the automap is active or not, and any automap marks, are now saved in savegames. (Note that this change breaks savegame compatibility with previous versions of <i>DOOM Retro</i>.)
* A header comment has been added to the top of <code>doomretro.cfg</code>, with a note advising to “go to http://wiki.doomretro.com for information on changing these settings”.
* The FPS counter displayed when <code><span style="white-space:nowrap">&ndash;DEVPARM</span></code> is specified on the command-line now won’t be hidden when taking a screenshot, and will continue to update when in a menu or the game is paused.
* Diminished lighting from the player has been enhanced.
* Bloodsplats are now only spawned at the same height as corpses as they slide.

### DOOM Retro v1.6.6

* The new liquid animation that was introduced in <i>DOOM Retro v1.6.5</i> has been improved upon such that the entire textures of the sectors will now also rise and fall, rather than just their edges.
* Due to this change in animation, the floating objects in liquid sectors now rise and fall in sync with each other and the sector.
* The bottom of objects in liquid that don’t float (such as monsters and barrels) will now be clipped in sync with the rise and fall of the liquid.
* All liquid sectors are now animated, avoiding issues where one liquid sector would be animated, but another liquid sector adjacent to it wouldn’t be.
* A bug has been fixed whereby the liquid animation was stopping the player and/or monsters from being able to enter certain areas of some maps.
* The brightmap for the <code>COMP2</code> wall texture has been fixed.
* A bug has been fixed whereby other monsters could infight with Arch-viles.
* The teleporter texture used in <i>Back to Saturn X</i> no longer animates like a liquid.
* Savegames will now be placed in the <code>savegames\\DOOM2.WAD\\</code> folder rather than the <code>savegames\\unknown.wad\\</code> folder when <code>NERVE.WAD</code> is loaded.
* The player’s weapon is now recentered after teleporting.

### DOOM Retro v1.6.5

* Many optimizations have been made to further improve the overall performance and stability of <i>DOOM Retro</i>.
* A bug has been fixed whereby secret sectors that lower and change their texture may not be displayed in the automap correctly.
* Decorative corpses spawned at the start of a map are now randomly mirrored as intended.
* A bug has been fixed whereby the player sometimes wouldn’t trigger a teleport when running over it in some instances.
* Changes have been made to how lighting is calculated. See Linguica’s findings on the <a target="_blank" href="//www.doomworld.com/vb/post/1336337">Doomworld Forums</a> for more information.
* Liquid sectors, and the partially submerged objects that are on them, now animate up and down. This feature may be disabled by changing the <code>animatedliquid</code> setting in <code>doomretro.cfg</code> to <code>false</code>.
* A bug has been fixed whereby objects on a sector that lowers and becomes liquid wouldn’t update immediately (that is, the bottom of their sprites wouldn’t be clipped, their shadow wouldn’t be removed, and blood splats wouldn’t be removed either).
* Blood splats that are spawned around decorative corpses at the start of a map are now spawned in a more natural-looking circular pattern, and may also be offset slightly from the corpse to give the impression that the corpse may have slid.
* Blood splats that are spawned when crushing a corpse are now also spawned in a circular pattern.
* The positions of shadows for several monsters have been improved.
* The total amount of blood splats that can be spawned when a corpse slides across the floor has been doubled.
* The total amount of blood splats that can be spawned when a corpse slides across the floor is now saved in savegames. This breaks savegame compatibility with previous versions of <i>DOOM Retro</i>.
* Blood splats that are spawned around decorative corpses at the start of a map now come from this same total.
* A bug has been fixed whereby it was possible for the screen to switch between the <code>TITLEPIC</code> and <code>CREDIT</code> lumps before wiping the screen after the player selects a skill level in the menu to start a new game.
* The top and bottom edges of Spectre shadows are now “fuzzy”.
* The top and bottom edges of Spectre shadows are now “fuzzy”, and also lighter, in <i>DOOM II’s</i> cast sequence, to match how they appear in the game.
* A savegame slot that isn’t empty is now automatically selected in the load game menu if the player previously exited the save game menu while a savegame slot that is empty was selected.
* The screen will now flash the same amount for keycards and skull keys as for other pickups.
* The window can now be resized when in widescreen mode.
* Several changes have been made so that <i>DOOM Retro</i> will behave better when custom sprites are present in a PWAD.
* <i>DOOM Retro</i> will no longer exit with an error if a flat’s texture is missing in a WAD.
* Zooming in and out of the automap using a gamepad has now been fixed.
* The textures <code>PLANET1</code>, <code>SW2BLUE</code> and <code>SW2MARB</code> now have brightmaps.
* <code>NERVE.WAD</code> will now be automatically loaded if <code>DOOM2.WAD</code> is selected in the WAD launcher and there are also one or more PWADs selected that don’t contain any map data.
* The translucency of blood splats has been reduced slightly, and their edges are no longer softened.
* Green blood splats are now slightly darker.
* A bug has been fixed whereby some shootable switches would only work once.
* The edges of shadows are now black when the <code>translucency</code> setting in <code>doomretro.cfg</code> to <code>false</code>.
* The <code>sfxvolume</code> and <code>musicvolume</code> settings in <code>doomretro.cfg</code> will no longer round down to <code>0%</code> when set to <code>6%</code>.

### DOOM Retro v1.6.4

* Several optimizations have been made in an attempt to further improve the overall performance of <i>DOOM Retro</i>.
* DeHackEd files (<code>\*.DEH</code> and <code>\*.BEX</code>) are now displayed and can be loaded in the WAD launcher. <kbd>CTRL</kbd>-click to select them along with the WAD file(s) you want to load.
* When multiple PWADs are selected in the WAD launcher, and with no IWAD, every PWAD will now be checked until the IWAD required is determined, rather than potentially failing if the first PWAD checked contains no map data.
* The FPS counter will now work correctly when <code><span style="white-space:nowrap">&ndash;DEVPARM</span></code> is specified on the command-line.
* Problems with movement of the mouse have been fixed when starting <i>DOOM Retro</i> in windowed mode.
* Panning when follow mode is off in the automap is now back to working correctly.
* Several changes have been made so that the window caption is now updated correctly.
* Smoke trails and bullet puffs are now greyer then before, and displayed using 33% translucency rather than additive translucency.
* The player’s view will no longer be lowered further due to the <code>footclip</code> setting when standing in a self-referencing sector.
* Improvements have been made to switching weapons using a gamepad.
* For non-widescreen displays, the status bar is no longer displayed when in the automap if it isn’t displayed during a game.
* A bug has been fixed whereby the <code>screensize</code> setting was being reset at startup for non-widescreen displays.
* The “dead zones” of the left and right thumbsticks of gamepads can now be adjusted using the <code>gamepad_leftdeadzone</code> and <code>gamepad_rightdeadzone</code> settings in <code>doomretro.cfg</code>. They are <code>24%</code> and <code>26.5%</code> by default.
* There are now settings for every control in the automap, for both the keyboard and gamepad, in <code>doomretro.cfg</code>.
* The <code>sfx_volume</code> and <code>music_volume</code> settings in <code>doomretro.cfg</code> have had their underscores removed, and their values are now displayed as percentages.
* Smoke trails are no longer displayed for Revenants’ non-homing rockets, as intended.
* The menu may still be opened with the gamepad’s <kbd>START</kbd> button if <code>gamepad_sensitivity</code> has been reduced to <code>0</code>.
* A bug has been fixed whereby the help screens wrongly indicated that the <kbd>A</kbd> and <kbd>D</kbd> keys were used to turn rather than strafe.
* The overall gamepad sensitivity has been increased even more, and the default of <code>gamepad_sensitivity</code> has now been doubled from <code>16</code> to <code>32</code>.
* The player will now bob as intended when <code>playerbob</code> is greater than <code>75%</code>.
* Changes have been made to the messages displayed when adding and clearing marks.

### DOOM Retro v1.6.3

* Several internal optimizations have been made in an attempt to improve the overall performance of <i>DOOM Retro</i>.
* Decorative corpses are now randomly mirrored when a map is loaded.
* A potential overflow has been fixed if there was more than 32 characters in a savegame description.
* Cheat sequences can now be overridden in DeHackEd lumps and files.
* The bobbing up and down of floating powerups can now be disabled by setting <code>floatbob</code> to <code>false</code> in <code>doomretro.cfg</code>.
* The window caption is now updated when ending a game through the options menu.
* <code>DOOM2.WAD</code> savegames are no longer placed in the <code>NERVE.WAD</code> folder.
* A bug has been fixed whereby extreme slowdown would occur if a large amount of monsters (and blood splats) were on a moving sector.
* Pressing the window’s maximize button will now switch to fullscreen mode.
* The messages displayed when the player picks up ammo are now accurate, since the amount can vary depending on the skill level chosen and/or whether a monster dropped it or not.
* A bug has been fixed whereby monsters were not always remembering their last target when attacked by something else. This would affect monster infighting in some instances. 
* Widescreen mode is now supported in windowed mode. Rather than displaying pillarboxes, the window will expand and contract horizontally as necessary when toggling widescreen mode on and off.
* The display of the “screen size” slider in the options menu is now fixed when in windowed mode.
* The width and height of the window are no longer set to be even numbers.
* A stretched cursor will no longer sometimes briefly appear in the center of the window when opening the menu in windowed mode.
* <i>DOOM Retro</i> should no longer crash when switching between fullscreen and windowed modes with <kbd>ALT</kbd> + <kbd>ENTER</kbd>, and when resizing the window.
* If <code>NERVE.WAD</code> is loaded, the window caption will now be updated to indicate the selected expansion when in the expansion and skill level menus before starting a game.
* Shadows are now saved in savegames, even when the <code>shadows</code> setting in <code>doomretro.cfg</code> is <code>false</code>.
* A bug has been fixed whereby the value of the <code>bloodsplats</code> setting in <code>doomretro.cfg</code> was always set to the default of <code>unlimited</code> regardless of what it was changed to.
* If a setting in <code>doomretro.cfg</code> that has a range of values is set out of range, it will be capped at the minimum or maximum, rather than changed back to the default.
* A <code>mapfixes</code> setting in <code>doomretro.cfg</code> has been implemented to allow the several hundred map-specific fixes that <i>DOOM Retro applies</i> to be enabled or disabled by type. The default is <code>linedefs|sectors|things|vertexes</code>.
* The weapon number keys can now be changed by altering the <code>key_weaponx</code> settings in <code>doomretro.cfg</code>.
* A bug has been fixed whereby some combinations of flags used by the <code>corpses</code> setting in <code>doomretro.cfg</code> weren’t being considered valid.
* Key settings in <code>doomretro.cfg</code> can no longer be set to function keys.
* The controls set by <code>gamepad_prevweapon</code> and <code>gamepad_nextweapon</code> can no longer be used when the game is paused.
* Faster switching to the next and previous weapons is now allowed.
* Improvements have been made to the menu in <i>HacX: Twitch ‘n Kill</i>.
* Smoke trails are no longer applied to Revenant non-homing rockets by default.
* Instead of just <code>on</code> and <code>off</code>, the <code>smoketrails</code> setting in <code>doomretro.cfg</code> has been changed to allow smoke trails to be enabled or disabled depending on who fired the projectile. The setting can be any combination of the following flags: <code>player</code> (smoke trails for player rockets), <code>revenant1</code> (smoke trails for Revenant non-homing rockets), <code>revenant2</code> (smoke trails for Revenant homing rockets) and <code>cyberdemon</code> (smoke trails for Cyberdemon rockets). The default is <code>player|revenant2|cyberdemon</code>.
* A bug has been fixed whereby pressing the <kbd>CAPSLOCK</kbd> key on the title screen will cause the use of the key to then become inverted (that is, turning <kbd>CAPSLOCK</kbd> on would turn “always run” off, and vice versa).
* The display of the asterisk character is now allowed in a savegame description.
* The player arrow in the automap is now displayed correctly when zoomed in.
* A bug has hopefully been fixed that caused objects to sometimes disappear when standing on sector boundaries.
* Several changes have been made to improve <i>DOOM Retro’s</i> DeHackEd support.
* A bug has been fixed present in <i>Vanilla DOOM</i> whereby the vertical position of an Arch-vile’s fire attack could be set incorrectly in some instances.
* There is no longer any small upward thrust when the player is receives an Arch-vile’s fire attack while “no clipping mode” is on using the <code>IDCLIP</code> cheat.

### DOOM Retro v1.6.2

* A bug, inadvertently introduced in v1.6.1, has been fixed whereby the wrong colored keys would appear in the HUD.
* The default value for the <code>saturation</code> setting in <code>doomretro.cfg</code> has been changed from <code>0.75</code> to <code>1.00</code>.
* The <code>gamma</code> setting in <code>doomretro.cfg</code> has been renamed to <code>gammacorrectionlevel</code>, and the alias <code>off</code> can now be used when it is set to <code>0</code>.
* Blood splats will now appear in maps from PWADs that use a DeHackEd file or lump.
* The clipping of the bottom of things will now be updated when the sector they’re in changes to/from liquid.
* Spectres now have slightly lighter shadows than the other monsters, with fuzzy edges.
* Minor changes have been made to the values of settings in <code>doomretro.cfg</code>. The <code>true</code>/<code>false</code> aliases now appear as <code>on</code>/<code>off</code>, and the <code>none</code> alias now appears as a hyphen. Aliases <code>yes</code>/<code>no</code> may also be used.
* Setting the mouse sensitivity to 0 will no longer disable the gamepad as well.
* The range of gamepad sensitivity has been increased.
* Several minor changes have been made to the properties of pickups in <i>HacX: Twitch ‘n Kill</i>.
* The <kbd>,</kbd> and <kbd>.</kbd> keys can now also be used to strafe left and right, as they could in <i>Vanilla DOOM</i>. They are changed using the <code>key_strafeleft2</code> and <code>key_straferight2</code> settings in <code>doomretro.cfg</code>.
* Shifted characters are now allowed when entering savegame descriptions. (In <i>Vanilla DOOM</i>, pressing <kbd>SHIFT</kbd> + <kbd>/</kbd>, for example, would still display “/” rather than “?”.)

### DOOM Retro v1.6.1

* If a DeHackEd file (with a .DEH extension) is present with the same name and in the same folder as the selected PWAD, it will be automatically opened as well.
* Improvements have been made to when the player slides against walls.
* A bug has been fixed whereby the screen would not render fully after switching from fullscreen to windowed modes when pressing <kbd>ALT</kbd> + <kbd>ENTER</kbd>.
* Several compatibility fixes have been made when using DeHacked files and lumps.
* Savegames for <i>Back To Saturn X</i> are now separated by episode.
* A bug has been fixed whereby <i>HacX: Twitch ‘n Kill</i> wouldn’t load at all. Specific support has now been added for it.
* Fake contrast is now applied to outdoor areas again.
* Thing triangles no longer appear for the invisible corpses in <i>Chex Quest</i> when using the <code>IDDT</code> cheat in the automap.
* The default value of <code>snd_maxslicetime_ms</code> has been changed from <code>120</code> to <code>28</code>. This is consistent with <i>Chocolate DOOM’s</i> default, and reduces the slight lag when playing sounds.
* The <code>centeredweapon</code> setting has been added to <code>doomretro.cfg</code>. Setting it to <code>true</code> will center the player’s weapon each time it’s fired (the default). If <code>false</code>, <i>Vanilla DOOM’s</i> behavior is used.
* A bug has been fixed whereby input would momentarily become stuck if the splash screen was skipped at startup.
* Blood splats are now green in <i>Chex Quest</i> as intended.
* A bug has been fixed whereby switching to and from the chainsaw using the number keys really quickly would cause either a crash, or the player’s weapon to disappear completely.
* The player’s weapon bob is now consistent with <i>Vanilla DOOM</i>.

### DOOM Retro v1.6

* Further enhancements have been made to <i>DOOM Retro’s</i> overall performance and stability.
* Widescreen mode is no longer stretched horizontally on displays with a greater aspect ratio than 16:10.
* Monsters, pickups, corpses and rockets now all cast dynamic shadows on the ground. These shadows are not cast when in or over liquid, nor when the player has either the light amplification visor or invulnerability power-ups. Shadows may be disabled by setting <code>shadows</code> to <code>false</code> in <code>doomretro.cfg</code>. Shadows are translucent, but if <code>translucency</code> is set to <code>false</code> in <code>doomretro.cfg</code>, they are solid black.
* All objects are now partially submerged when standing in liquid. This feature may be disabled by setting <code>footclip</code> to <code>false</code> in <code>doomretro.cfg</code>.
* The player’s view is now lowered slightly when standing in liquid. Setting <code>footclip</code> to <code>false</code> in <code>doomretro.cfg</code> will also disable this.
* Less friction is applied to corpses and dropped items when in liquid.
* Blood splats are now drawn regardless of how far they are away from the player.
* The firing animation of the Heavy Weapon Dude when facing to the right is now smoother.
* Since they all cast shadows now, the Lost Soul, Cacodemon and Pain Elemental are higher off the ground in <i>DOOM II’s</i> cast sequence.
* Deaths are now randomly flipped in the cast sequence.
* It is now possible to warp to episodes beyond episode 4 using the <code><span style="white-space:nowrap">&ndash;WARP</span></code> command-line parameter.
* <i>DOOM Retro</i> now supports <a target="_blank" href="//doomwiki.org/wiki/Dehacked">DeHackEd</a> files and lumps, including those with BOOM extensions. DeHackEd files may be opened by using the <code><span style="white-space:nowrap">&ndash;DEH</span></code> or <code><span style="white-space:nowrap">&ndash;BEX</span></code> command-line parameters. <code>DEHACKED</code> lumps will automatically be parsed unless <code><span style="white-space:nowrap">&ndash;NODEH</span></code> is specified on the command-line.
* If a map from a PWAD is loaded, and no <code>DEHACKED</code> lump is present in the PWAD specifying its name, then the PWAD’s name will also be included in the automap.
* The intermission screen is now displayed at the end of <code>E<i>x</i>M8</code>.
* The amount of kills is now correctly capped at 100% on the intermission screen in all instances.
* Walls are now drawn with even greater precision, fixing many graphic anomalies that may appear when the player stands on a line with a change in height.
* Fake contrast is no longer applied to walls in outdoor areas.
* The position of floating items has been raised off the ground slightly.
* A message is now displayed in <code>stdout.txt</code> when an Arch-vile resurrects a monster.
* The mouse controls that select next and previous weapons can now be set by changing <code>mouse_nextweapon</code> and <code>mouse_prevweapon</code> in <code>doomretro.cfg</code>. They are set to <code>wheeldown</code> and <code>wheelup</code> by default.
* The use action may now be bound to a mouse button by changing the <code>mouse_use</code> setting in <code>doomretro.cfg</code>. It is set to <code>none</code> by default.
* The maximum mouse sensitivity has been doubled from <code>64</code> to <code>128</code>.
* Mouse and gamepad sensitivity are now two separate settings.
* Both the mouse and gamepad are now completely disabled when sensitivity is set to <code>0</code> and not in a menu. Previously, only the turning movement of both devices was disabled.
* The overall gamepad sensitivity has now been reduced slightly.
* Moving sliders in the menu with the gamepad is now faster.
* When a game is loaded, including when a savegame is autoloaded after a player’s death, a message is now displayed.
* A version number is now embedded in each savegame to avoid a crash when attempting to load an older and incompatible savegame.
* Minor changes have been made to the help screen.
* The red screen tint when the player has the berserk power-up has been reduced slightly.
* All drop shadows in the menus, in the HUD and on messages are now solid when <code>translucency</code> is <code>false</code>.
* Specific support has been added for <i>Chex Quest</i>:
  * The window caption is displayed as <i>“Chex Quest”</i>.
  * <code>CHEX.DEH</code> is automatically loaded if it’s present in the same folder as <code>CHEX.WAD</code> and <code><span style="white-space:nowrap">&ndash;NODEH</span></code> isn’t specified on the command-line.
  * The episode menu is skipped.
  * No obituaries are printed to <code>stdout.txt</code>.
  * No items are dropped.
  * No additional blood is spawned around map decorations.
  * All blood splats are green.
  * Since corpses are effectively invisible, there are no blood splats or crunch sounds made if an invisible corpse happens to be under a door.
  * The screen will flash green rather than red when the player is injured.
* Specific support has also been added for both episodes of <i>Back to Saturn X</i>:
  * The window caption is displayed as <i>“Back to Saturn X E1: Get Out Of My Stations”</i> or <i>“Back to Saturn X E2: Tower in the Fountain of Sparks”</i>.
  * <i>DOOM Retro’s</i> custom sprite offsets, previously only used with the official <i>DOOM</i> IWADS, are now used. This fixes a bug that caused some level decorations to “twitch” as they animated.
  * If only <code>BTSX_E2A.WAD</code> is loaded from the WAD launcher, then <code>BTSX_E2B.WAD</code> is automatically loaded as well, and vice-versa.
  * The map number in the message displayed when using the <code>IDCLEV<i>xy</i></code> cheat is of the form “<code>E<i>x</i>M<i>yy</i></code>” to match what’s displayed in the automap.
  * To avoid a crash, you are no longer able to warp to a <i>DOOM II</i> map that is not replaced by a map in <i>Back to Saturn X’s</i> PWAD, using either the <code><span style="white-space:nowrap">&ndash;WARP</span></code> command-line parameter or the <code>IDCLEV<i>xy</i></code> cheat.
* The amount of blood splats produced when crushing corpses is now based on their width.
* The edges of blood splats have now been softened slightly.
* Decorative corpses can now be crushed.
* Changes have been made to the text on the splash screen.
* The splash screen now fades onto and off of the screen at startup.
* The splash screen may now be accelerated by pressing a key or button.
* Monsters will now try to move away from tall dropoffs.
* When spawning blood splats around decorations when a map is loaded, blood splats will no longer be spawned on floors close to but higher than the decoration itself.
* Textures <code>RROCK05</code> to <code>RROCK08</code>, and <code>SLIME09</code> to <code>SLIME12</code>, are no longer considered liquids, and therefore blood splats can now appear on them.
* Further improvements have been made in detecting Mancubus fireball collisions.
* Spawn cubes are no longer spawned when the <code><span style="white-space:nowrap">&ndash;NOMONSTERS</span></code> command-line parameter is used.
* A bug has been fixed whereby a flashing skull key in the HUD would cause other keys next to it to shift slightly.
* In the automap, lines won’t be shown as being teleport lines (that is, in dark red) unless:
  * it is part of an obvious teleport by being adjacent to a floor with a <code>GATE<i>x</i></code> texture,
  * the player has been teleported by it, or,
  * the player is using the <code>IDDT</code> cheat.
* All settings in <code>doomretro.cfg</code> are now validated at startup. If any setting is found to be invalid, it will be reset to its default.
* Improvements have been made to the smoke trails of player and Cyberdemon rockets, and Revenant projectiles. The smoke will take slightly longer to dissipate, and is better randomized.
* The heights of Revenants and Arch-viles have been restored to their lower defaults. Many maps rely on this, and so therefore this fixes instances whereby sometimes these monsters would be stuck and wouldn’t attack the player.
* A monster will no longer go fullbright when firing a projectile if they are facing away from the player.
* A bug has been fixed whereby if an action was assigned to the <kbd>I</kbd> key, it wouldn’t work since “<code>I</code>” is the first character of every cheat.
* The patch offsets for textures <code>SKY1</code> and <code>BIGDOOR7</code> have been corrected.
* Long map titles in the automap and when saving a game, as well as player messages, are now truncated and followed with an ellipsis.
* The read-only <code>xinput</code> setting has been removed from <code>doomretro.cfg</code>.
* A read-only version setting has been added to <code>doomretro.cfg</code>.
* If <code>doomretro.cfg</code> was deleted, and then regenerated by running <i>DOOM Retro</i>, the defaults for the <code>corpses</code> and <code>videodriver</code> settings are now correct.
* The <i>DOOM Retro</i> icon is now always used in the window caption.
* All trademark symbols have been removed from the window caption.
* Multiple PWADs may now be selected in the WAD launcher without an IWAD.
* Monsters will no longer go to sleep after killing a monster and not seeing the player anymore.
* All palette effects are now removed while a menu is displayed or the game is paused.
* Improvements have been made to the translucency of the blue armor in the HUD.
* The player can now still pass under solid hanging corpses if there is sufficient room.
* A bug has been fixed whereby the <kbd>,</kbd> key was being incorrectly mapped, causing it to act like the <kbd>-</kbd> key if it was bound to anything.
* Use of the <kbd>ALT</kbd> and <kbd>CTRL</kbd> keys in the menu has been disabled.
* Empty savegame slots may no longer be selected in the load game menu.
* Only update savegame descriptions of the form “<code>E<i>x</i>M<i>y</i></code>” in <i>DOOM</i> games, not <i>DOOM II</i> games.
* The text caret in the save game menu is now better positioned when using a PWAD with a custom character set.
* A bug has been fixed whereby the position of the skull cursor could be misplaced when exiting and then returning to the menu using the gamepad in some instances.
* The number of characters that can be entered in a savegame slot is now calculated correctly.
* If the <kbd>CAPSLOCK</kbd> key was on before <i>DOOM Retro</i> was run, it is now turned back on as necessary when quitting.
* Player messages and the map name in the automap are now translucent and have drop shadows when using a PWAD with a custom character set.
* A bug has been fixed whereby an Arch-vile could resurrect a monster for it to instantly become stuck in another monster.
* The player’s screen will no longer flash red if they are hit by a projectile while already dead.
* A bug has been fixed whereby the game would crash when using the <code><span style="white-space:nowrap">&ndash;NOSFX</span></code> or <code><span style="white-space:nowrap">&ndash;NOSOUND</span></code> command-line parameters.
* A bug has been fixed whereby you could switch weapons when zooming in the automap if both actions were set to the gamepad’s left and right shoulder buttons.
* Skies with heights other than 128 pixels are now rendered correctly.

### DOOM Retro v1.5.2

* Monster targets are now completely restored upon loading a game, regardless of whether they were targeting the player, or they were infighting.
* A Boss Brain no longer needs to be in MAP30 for the monsters it spawns to telefrag the player.
* A bug has been fixed whereby monsters were allowed to be above or below other monsters after teleporting, when normally they wouldn’t be.
* Improvements have been made to the position of some elements in the menu and HUD.
* Optimizations have been made to the loading of maps, and the lighting of the player’s weapon.
* A bug has been fixed whereby the game could crash when trying to draw the player’s weapon in pitch black areas in some instances.
* A bug has been fixed whereby the <kbd>CAPSLOCK</kbd> key was not being turned off when quitting the game.
* Pressing <kbd>CAPSLOCK</kbd> during a game will now display an <code>ALWAYS RUN ON/OFF</code> message. If when quitting the game the <kbd>CAPSLOCK</kbd> key is still on, it will be turned back on the next time <i>DOOM Retro</i> is started.
* In the previous version of <i>DOOM Retro</i>, the default video driver was changed from <i>Windows GDI</i> to <i>DirectX</i> to help in improving performance in fullscreen mode. If <i>DirectX</i> wasn’t installed, <i>DOOM Retro</i> would exit with an error. Now, if one video driver fails, <i>DOOM Retro</i> will try the other driver before exiting with an error.
* All in-game messages are now output to <code>stdout.txt</code>, whether messages are enabled or not.</li>
* Whenever the player or a monster is killed, a message is displayed in <code>stdout.txt</code>.
* Minor changes have been made to a few messages.
* A bug has been fixed whereby <i>DOOM Retro</i> would exit with an error if the <code>IDDQD</code> cheat was used to resurrect a dead player.

### DOOM Retro v1.5.1

* A splash screen is now displayed briefly when <i>DOOM Retro</i> is opened.
* Several optimizations have been made to improve <i>DOOM Retro’s</i> overall performance and reduce the size of its executable.
* <i>DOOM Retro</i> is now back to using the desktop resolution by default.
* If <code>doom2.wad</code> is selected by itself in the WAD launcher, <code>nerve.wad</code> will be automatically loaded if it’s in the same folder, and <i>Hell on Earth</i> will be preselected in the expansion menu. If <code>nerve.wad</code> is selected by itself, <i>No Rest for the Living</i> will be preselected instead.
* The <code>DOOMWADDIR</code> environment variable is now checked when automatically looking for IWAD files.
* No longer is anything output to the <code>stdout.txt</code> and <code>stderr.txt</code> files generated by SDL.
* A bug has been fixed whereby sometimes the press of a mouse button would register twice in a menu.
* The <kbd>PAUSE</kbd> key is now disabled on the title screen.
* The limit on the length of music in PWADs has been removed.
* A bug has been fixed whereby a crash would occur when changing to some levels.
* The sounds heard when selecting a skill level to start a new game, and using the <code>IDCLEV</code><i>xy</i> cheat, will no longer be cut off.
* In <i>Vanilla DOOM</i>, when a game is saved, all monsters would lose their targets. This has now been changed so that if a monster has a target when a game is saved, when that game is then loaded, they will target the player.
* A bug has been fixed whereby the player could still switch to their fist even though they had a chainsaw and a berserk power-up in the previous map.
* A bug present in <i>Vanilla DOOM</i> has been fixed whereby the player’s fist would be selected at the start of a map even though they had a chainsaw if they had the berserk power-up in the previous map.
* <code>doomretro.wad</code> is no longer renamed to <code>doomretro.wad.temp</code> while the WAD launcher is open.
* Each time the player switches to their fist while they have the berserk power-up, the power-up sound will now be heard.
* The red palette effect of the berserk power-up is now slightly redder.
* Palette effects will no longer be displayed while a menu is on the screen, or while the player is dead.
* The player’s view will now lower if they happen to die while on top of something else.
* Further improvements have been made to how custom menu graphics from PWADs are displayed.
* <i>DOOM Retro</i> will no longer crash if the player walks over a very deep hole.
* The intensity of the vibration effect for XInput-compatible controllers when the player fires a weapon or is injured has been increased. Previously, the effect on the <i>Logitech F710 controller</i> could barely be felt.

### DOOM Retro v1.5

* An extensive amount of enhancements have been made to <i>DOOM Retro’s</i> overall performance and stability.
* By default, <i>DOOM Retro</i> will now try running at a resolution of 1280×800. If that isn’t available, the desktop resolution will be used.
* The following changes have been made to corpses:
  * Corpses, hanging corpses, and pools of blood and/or guts that prepopulate maps are now each surrounded by an additional pool of blood.
  * Corpses (including those that prepopulate maps) will now slide in reaction to the blasts from projectile and barrel explosions.
  * The masses of some of the lighter monsters have been increased so they slide more realistically.
  * As corpses slide, blood will now be smeared on the ground. In previous versions of <i>DOOM Retro</i>, blood splats were only ever produced when monsters were injured by hitscan weapons, but now, since corpses tend to slide from the force of impact, blood splats are produced when they are injured by projectile weapons as well.
  * Corpses (and dropped items) now have torque applied to them, allowing them to slide around more realistically.
  * Arch-viles can now resurrect corpses that prepopulate maps.
  * Blood will no longer be produced as monsters are being crushed by a moving sector, but when they do finally die, instead of a small pile of blood and guts, a larger pool of blood in the correct color is produced. The same happens for corpses under closing doors.
  * These various new corpse-related changes can be toggled on and off by using any combination of flags with the new <code>corpse</code> setting in <code>doomretro.cfg</code>. It’s set to <code>mirror|slide|smearblood|moreblood</code> by default.
* A bug has been fixed whereby mouse movement wasn’t smooth for some users.
* The lower mouse sensitivity settings are now slightly slower. The default setting has been increased from <code>12</code> to <code>16</code> to accommodate for this.
* Blood splats will no longer be spawned on additional animated flats present in PWADs.
* Weapons dropped by Shotgun Guys and Heavy Weapon Dudes when they die are no longer randomly mirrored by default. This feature can be reenabled by changing the new <code>mirrorweapons</code> setting in <code>doomretro.cfg</code> to <code>true</code>.
* Blood is now randomly mirrored.
* The limit has been removed on the number of monsters a Boss Brain can spawn.
* A bug from <i>Vanilla DOOM</i> has been fixed whereby spawn cubes would miss east and west targets. See the <a target="_blank" href="//doomwiki.org/wiki/Spawn_cubes_miss_east_and_west_targets"><i>DOOM Wiki</i></a> for more information.
* A bug has been whereby shooting at a monster being raised by an Arch-vile could cause the game to crash.
* Floating monsters (Cacodemons, Pain Elementals and Lost Souls) can no longer get stuck together.
* The corpses of Cacodemons will no longer sometimes get suspended in midair.
* A bug has been fixed whereby a frame in the Cacodemon’s death sequence wasn’t displaying correctly.
* Elements on the intermission screen are now positioned better if custom graphics from PWADs are used.
* The player’s bob has been reduced by 25% to correspond with the latest official versions of <i>DOOM</i>. A <code>playerbob</code> setting has been created in <code>doomretro.cfg</code> to change this. The default value is <code>75%</code>.
* The player’s weapon bob is now slightly smoother, and is centered when a shot is fired.
* XBOX 360 controllers will now vibrate when a weapon is fired, and also when the chainsaw is idle.
* The limits on the number of moving ceilings and platforms in a level have been removed.
* The message displayed when <i>DOOM Retro</i> is run for the first time has been updated to indicate that “<code>additional PWAD files may also be selected by CTRL&ndash;clicking on them</code>” in the WAD launcher.
* PWADs can now be selected without an IWAD in the WAD launcher. <i>DOOM Retro</i> will first look for the required IWAD in the same folder as the PWAD, and if it isn’t found, will then look in the last folder an IWAD was loaded successfully.
* Similarly, PWADs can now be specified on the command-line without an IWAD. If no <code><span style="white-space:nowrap">&ndash;IWAD</span></code> command-line parameter is present, <i>DOOM Retro</i> will first look for the required IWAD in the same folder as the PWAD, and if it isn’t found, will then look in the last folder an IWAD was loaded successfully.
* A bug has been fixed whereby <i>DOOM Retro</i> may have crashed silently after selecting a WAD in the WAD launcher.
* The command-line parameter <code><span style="white-space:nowrap">&ndash;EXPANSION</span></code> may now be used. Use <span style="white-space:nowrap"><code>&ndash;EXPANSION 1</code></span> to autostart <i>DOOM II: Hell On Earth</i>, and use <span style="white-space:nowrap"><code>&ndash;EXPANSION 2</code></span> to autostart <i>DOOM II: No Rest For The Living</i> (if <code>NERVE.WAD</code> is specified using <code><span style="white-space:nowrap">&ndash;FILE</span></code> or <code><span style="white-space:nowrap">&ndash;PWAD</span></code>).
* The value that can be specified by the <code><span style="white-space:nowrap">&ndash;WARP</span></code> command-line parameter can now be of the form “<code>E<i>x</i>M<i>y</i></code>” or “<code>MAP<i>xy</i></code>”.
* If a <a target="_blank" href="//freedoom.github.io/"><i>FREEDOOM</i></a> IWAD is loaded in <i>DOOM Retro</i>, a message is displayed explaining that “<code>FREEDOOM requires a BOOM&ndash;compatible source port, and is therefore unable to be opened</code>”. This message won’t be displayed if the IWAD is loaded with an additional PWAD, so <i>FREEDOOM</i> can be used as a resource for the maps present in that PWAD.
* Further improvements have been made when using custom graphics in PWADs, particularly if <i>FREEDOOM</i> is being used.
* A bug from <i>Vanilla DOOM</i> has been fixed whereby corrupt texture names would be displayed in the error if a texture couldn’t be found. See the <a target="_blank" href="//doomwiki.org/wiki/Absurd_texture_name_in_error_message"><i>DOOM Wiki</i></a> for more information.
* A bug has been fixed whereby pressing the <kbd>ENTER</kbd> key or the left mouse button when in the help screen wouldn’t restore widescreen mode.
* The speed of turning with the gamepad’s right thumbstick when holding down the left trigger to run has been reduced slightly.
* The screen is now a constant tint of red while the player has the berserk power-up and their fist selected.
* The <code>blur</code> and <code>grayscale</code> settings that control the menu background have been removed from <code>doomretro.cfg</code>. The menu background will now always be blurred and gray.
* A bug has been fixed whereby the position of some items would be affected by moving platforms nearby.
* To better replicate the look of CRT monitors, which are/were not as bright as current LCD monitors, desaturation is now applied to <i>DOOM Retro’s</i> graphics. Changing the <code>saturation</code> setting in <code>doomretro.cfg</code> to <code>0</code> gives a grayscale effect, <code>1.0</code> is normal saturation, and <code>0.75</code> is the default.
* <code>gammalevel</code> has been changed to just <code>gamma</code> in <code>doomretro.cfg</code>.
* A bug has been fixed whereby non-solid hanging corpses would drop to the floor when above a moving sector in some instances.
* <kbd>ALT</kbd> + <kbd>F4</kbd> will now quit the game instantly without prompting.
* A bug has been fixed whereby the music on certain maps wouldn’t loop correctly.
* Monsters of the same species are now able to infight using projectiles when the player dies.
* The map names <i>Keen</i> and <i>IDKFA</i> in <i>DOOM II (BFG Edition)</i> are now correctly identified when saving a game.
* To allow neater sorting in the <code>Pictures\DOOM Retro</code> folder, determiners are now put at the end of screenshot names. For example, the file <code>The Inmost Dens.bmp</code> becomes <code>Inmost Dens, The.bmp</code>.
* <code>MUS_DDTBLU</code> is now played instead of <code>MUS_DDTBL2</code> when using the <code>IDMUS09</code> cheat in <code>NERVE.WAD</code>.
* The lighting of sprites has been changed slightly.
* The edges of the automap have been darkened slightly.
* Sounds are no longer cut off once an object has been removed. For example, rocket and barrel explosions are now slightly longer.
* When exiting a level, the exit switch will now be updated before toggling widescreen mode off.
* Brightmaps can now be toggled off using the <code>brightmaps</code> setting in <code>doomretro.cfg</code>.
* A bug has been fixed whereby the <code>IDKFA</code> cheat would still register when the player already had everything the cheat provided.
* Armor will no longer flash in the HUD when it is low.
* The clip graphic in the HUD has been shifted upwards by 1 pixel.
* Monsters are now smarter, and will avoid crushing ceilings and other damaging areas.
* The game will no longer crash if <code>bloodsplats</code> is set to <code>0</code> in <code>doomretro.cfg</code>.
* Translucency effects are now improved for white objects on a blue background.
* A <code>homindicator</code> setting has been added to <code>doomretro.cfg</code>. It is <code>false</code> by default.
* A bug has been fixed whereby the screen wouldn’t flash red if the player received only 1% damage.
* The smoke trail of Revenant projectiles has been positioned better.
* The <code>bloodsplatsvisible</code> setting has been removed.
* Widescreen mode is now retained the next time <i>DOOM Retro</i> is run if it happens to crash in a 4:3 mode.
* The chance of the super shotgun gibbing a monster at point blank range has been increased slightly.
* If a screenshot is taken with the <kbd>PRINTSCREEN</kbd> key when not in a game, the resulting file will be named accordingly: <code>Title.bmp</code>, <code>Help.bmp</code>, <code>Intermission.bmp</code> or <code>Finale.bmp</code>.
* The <kbd>CAPSLOCK</kbd> key is turned off if on when quitting the game.
* Some of the less-used mouse controls from <i>Vanilla DOOM</i> have been reimplemented. Disabled by default, they can be reenabled using the following settings in <code>doomretro.cfg</code>:
  * Set <code>novert</code> to <code>true</code> to allow vertical mouse movement to move the player forward/back.
  * Set <code>mouse_forward</code> to a mouse button to move forward.
  * Set <code>mouse_strafe</code> to a mouse button to strafe.
  * Set <code>dclick_use</code> to <code>true</code> so double-clicking the mouse buttons set by <code>mouse_forward</code> and <code>mouse_strafe</code> above will perform a use action.
* A bug has been fixed whereby the values <code>middle</code> and <code>right</code> representing mouse buttons in <code>doomretro.cfg</code> were switched.
* Now pressing a key the first time on a finale text screen will display all the text, and a second press will then advance to the next map.
* A bug has been fixed whereby pressing the <kbd>SPACEBAR</kbd> to advance a finale text screen would carry over to the following map, and cause the player to use a switch if they started directly in front of one (such as is the case for <i>MAP07: Dead Simple</i> in <i>DOOM II: Hell On Earth</i>).
* The flashing key in the HUD will now be updated if the player tries opening another locked door that requires a different key to the one currently flashing.

### DOOM Retro v1.4.3

* The screen size can now be adjusted correctly in the options menu when not in a game.
* The size of the pixels when the graphic detail is “LOW” can now be changed by editing the <code>pixelwidth</code> and <code>pixelheight</code> settings in <code>doomretro.cfg</code>. Both are set to a default of <code>2</code>.
* A bug has been fixed whereby pressing <kbd>ENTER</kbd> to save a game in an empty savegame slot wouldn’t clear that slot and replace it with the name of the current map.
* Barrels are no longer randomly mirrored when they explode.
* Some settings in <code>doomretro.cfg</code> are now sorted differently.
* A bug has been fixed whereby graphic anomalies could appear in some parts of some maps.
* The correct graphics will now be used for the titles of the load and save game menus, if custom graphics are present in a PWAD.
* If there are custom sprites present in a PWAD, they will now animate correctly.
* <code>nerve.wad</code> is now correctly recognized by the <code><span style="white-space:nowrap">&ndash;FILE</span></code> command-line parameter.
* <code><span style="white-space:nowrap">&ndash;PWAD</span></code> is now allowed as an alternate to the <code><span style="white-space:nowrap">&ndash;FILE</span></code> command-line parameter.
* Checking what things are on a floor that raises or lowers is now more accurate.
* Optimizations have been made when drawing the sky.
* The sky is now flipped vertically when repeated, as originally intended.
* A bug has been fixed whereby randomly colored pixels would appear along the very top of the screen when drawing the sky in some instances.
* There are now two settings in <code>doomretro.cfg</code> that control how many blood splats appear. <code>bloodsplats_total</code> (previously known as just <code>bloodsplats</code>) controls the total number of blood splats spawned in a level. It’s default is still <code>unlimited</code>. <code>bloodsplats_visible</code> is a new setting that specifies the number of blood splats that will appear on the screen at any one time. It’s default is <code>1024</code>.
* A bug has been fixed whereby use of the <code><span style="white-space:nowrap">&ndash;EPISODE</span></code>, <code><span style="white-space:nowrap">&ndash;SKILL</span></code> and <code><span style="white-space:nowrap">&ndash;WARP</span></code> command-line parameters would cause the game to silently crash.
* Gamepads now work correctly.

### DOOM Retro v1.4.2

* Several more limits have been removed, allowing larger and more detailed maps to be loaded without crashing.
* <i>DOOM Retro</i> will now try to fix some common map errors before loading a map.
* Optimizations have been made to the loading of large levels, and the handling of gamepads.
* The smoke trails of the player’s and Cyberdemon’s rockets can now be disabled by setting <code>smoketrails</code> to <code>false</code> in <code>doomretro.cfg</code>. It is <code>true</code> by default.
* The limit on the size of savegames has been removed, allowing the game to be saved on larger maps without exiting with an error.
* Several changes have been made to the HUD:
  * The HUD is now displayed in non-widescreen modes.
  * The drop shadows of the numbers in the HUD are now slightly lighter.
  * The HUD has been shifted downwards slightly.
  * Values in the HUD will now start flashing at 20 (rather than 10), and the speed at which they flash will increase the closer they get to 0.
  * When the player is invulnerable, only the red part of the medikit turns gold, and not the health value.
  * Keys are no longer seen to disappear from the HUD before wiping the screen when using the <code>IDCLEVxy</code> cheat to warp to a new map.
* The positions of blood splats are now randomized better.
* A memory leak has been fixed when loading a savegame.
* A bug has been fixed where the gamma correction level was being reset to 0.75 each time <i>DOOM Retro</i> was loaded, regardless of what it was previously set to.
* A bug has been fixed whereby some users were experiencing jerky mouse movement.
* A flashing HOM (“Hall of Mirrors”) indicator has been implemented. The screen will flash red in place of missing textures, but not when the player has enabled “no clipping mode” using the <code>IDCLIP</code> or <code>IDSPISPOPD</code> cheats.
* When saving a game, the savegame description will only be updated to the current map name if it hasn’t been changed to something other than map name previously.

### DOOM Retro v1.4.1

* A bug has been fixed whereby the sound would become disabled if adjusting the volume through the menu or pausing and then unpausing the game.
* The visplane limit has been removed, allowing for more detailed levels to run without crashing.
* A sound will now be heard if a wrong selection is made in the WAD launcher and it needs to reopen.
* When a Heavy Weapon Dude is killed, their corpse is no longer randomly mirrored.
* Projectiles will now pass through map decorations like they do in <i>Vanilla DOOM</i>. (Please note that this particular change means savegames from previous versions of <i>DOOM Retro</i> won’t work with <i>DOOM Retro v1.4.1</i>.)
* If music can’t be loaded for a particular map, that map will still load without music rather than the game exiting with an error.

### DOOM Retro v1.4

* Several optimizations have been made that improve the overall performance of *DOOM Retro*.
* When *DOOM Retro* is opened for the first time the following message is now displayed:
![firstuse](https://cloud.githubusercontent.com/assets/3210148/2886096/15b313ce-d4dc-11e3-8423-27ccccf939c8.png)
* There are now 2 levels of graphic detail, “HIGH” and “LOW”, adjustable through either the options menu or by pressing the `F5` key. The “HIGH” level has the same graphic detail as in previous versions of *DOOM Retro*. It has an effective resolution of 640×400 (or 640×336 in widescreen) and is the default. The “LOW” level is new, and has an effective resolution of 320×200 (the resolution used in *Vanilla DOOM*).
* When the title screen is displayed, it will now alternate with the credits screen.
* The gray text in the status bar is now twice the resolution when the graphic detail is set to “HIGH”.
* A bug has been fixed whereby the sounds of Revenant and Cyberdemon projectiles would become corrupted in some instances.
* The file **default.cfg** has been renamed to **doomretro.cfg**.
* The background when a menu is displayed or the game is paused is now in grayscale, as well as being blurred and darkened. This may be disabled by changing **grayscale** to **false** in **doomretro.cfg**.
* The menu background’s blur is now calculated differently, and should resolve the performance issues some users were experiencing.
* Minor corrections have been made to the drop shadows and kerning of text in the menus.
* Menus are now always centered vertically.
* Many changes have been made so that custom graphics in PWADs are now handled better, when previously they may have either ignored completely or misaligned.
* When changing the music using the **IDMUS** cheat, the name of the music will be displayed.
* When changing the map using the **IDCLEV<i>xy</i>** cheat, the name of the map will be displayed.
* When using the **IDBEHOLD<i>x</i>** cheats, the message displayed will indicate if the power-up is being toggled on or off.
* In **doomretro.cfg**, **menublur** has been renamed to just **blur**, **show_messages** to **messages**, **usegamma** to **gammalevel**, and **screenblocks** to **screensize**.
* When the player’s health, ammo or armor drops to 10 or less, those values will start flashing in the widescreen HUD as a warning.
* When the player is invulnerable (either by having picked up an invulnerability power-up or using the **IDDQD** cheat), their health will be displayed in gold in the widescreen HUD. This corresponds with the gold eyes of the player’s face in the status bar.
* The widescreen HUD now indicates when the player is invulnerable.
* There are now 31 different gamma correction levels between 0.50 and 2.0 inclusive, in increments of 0.05. Gamma correction level 0.75 is still the default.
* The first press of the `F11` key will now display the current gamma correction level. Further presses of the key before the message disappears will then increase the level (or decrease the level if the `SHIFT` key is held down).
* Autorepeat is now allowed for the `F11` key.
* The gamma correction level is now saved the moment it is changed.
* The weapons dropped by Shotgun Guys and Heavy Weapon Dudes when they die are now mirrored horizontally at random, independent of their corpses.
* A bug has been fixed whereby the player was unable to pick up stimpacks or medikits in the *BFG Editions* of *DOOM* or *DOOM II*.
* There is now slightly more blood.
* Because of some significant improvements to the drawing of blood splats, the number of blood splats that may be in a map is now unlimited. The **bloodsplats** setting in **doomretro.cfg** may still be changed from unlimited to a value between **0** and **32768** inclusive.
* Blood splats are now mirrored horizontally at random for some additional variation.
* The blood and blood splats from Spectres, as well as the player when they have the partial invisibility power-up, now appear with the same “fuzz effect”.
* Blood splats are now drawn first and no longer overlap other sprites, making them appear closer to the ground.
* When the mouse sensitivity is set to 0, the mouse is disabled. (And similarly, when a gamepad is in use, it is also disabled.)
* The translucency of blue and green blood has been reduced slightly.
* **doomretro.wad** no longer appears in the WAD launcher.
* Any PWADs selected with **doom1.wad** in the WAD launcher will now be ignored rather than displaying an error.
* **nerve.wad** can now be selected without **doom2.wad** in the WAD launcher, even when another PWAD is selected with it, and **doom2.wad** will be looked for automatically.
* A bug has been fixed whereby the keys the player had previously found weren’t being restored when loading a savegame.
* The location of GUS patches can now be specified using **timidity_cfg_path** in **doomretro.cfg**.
* Rotate mode is now enabled by default in the automap.
* Some minor changes have been made to the help screen.
* Screenshots are now saved as **Untitled.bmp** if taken while in the help screen.
* Messages will now timeout when the help screen is displayed.
* The lighting in maps is now calculated differently slightly.
* The small ‘3’ and ‘8’ digits used for marks in the automap, and the small ‘3’ digit in the status bar, have been altered slightly.
* There is now a chance that the super shotgun may gib a monster when fired at point blank range. (This idea has been taken from Fabian Greffrath’s *Crispy DOOM*.)

### DOOM Retro v1.3

* An extensive number of optimizations have been made that improve the overall performance of *DOOM Retro*.
* A heads-up display (HUD) is now shown in widescreen mode.
* Each element of the HUD is slightly translucent.
* An additional press of the `+` key, or move of the “SCREEN SIZE” slider to the right in the options menu, will hide the HUD.
* The HUD isn’t displayed while the player is dead.
* If the player has the berserk power-up, and has his fist selected, the berserk power-up will replace the medikit in the HUD.
* The keys that the player has picked up are displayed in the order they were found, from right to left.
* If the player attempts to open a door they don’t have the key for, that key will flash in the HUD.
* If the player has no armor, the keys are displayed along the right side of the screen.
* The type of armor the player has (either green or blue armor) is displayed.
* The background is now blurred when in a menu or the game is paused. This effect may be disabled by changing the **menublur** setting in **default.cfg** to false.
* The green blood of Hell Knights and Barons of Hell is now slightly darker.
* A bug has been fixed that existed in *Vanilla DOOM* that caused monsters to be able to see the player through walls, or not see the player when they should have, in some instances. Thank you to ???? (Nanka Kurashiki) for bringing this to my attention.
* The `PRINTSCRN` key and `ALT` + `ENTER` have been added to the help screen.
* The “HELP” title has been added to the help screen in *DOOM II* to be consistent with the other versions of *DOOM*.
* The player’s weapon is no longer displayed in the help screen’s background.
* Switching between widescreen and non-widescreen modes is now much cleaner. The status bar will no longer briefly appear at the bottom of the screen.
* Several bugs have been fixed whereby some IWADs and PWADs would sometimes fail to load from the WAD launcher.
* If the selection made in the WAD launcher is incorrect, it will reopen rather than exit with an error.
* By popular demand, **default.cfg** is now saved in the same folder as **doomretro.exe**. To make it clear where the settings are, a copy of **default.cfg** is now included in the distribution.
* The settings in **default.cfg** are now sorted alphabetically.
* *DOOM Retro* is now less likely to crash if certain settings in **default.cfg** are set incorrectly.
* The **bloodsplats** setting in **default.cfg** is now a number rather than **true** or **false**, and is **1024** by default.
* A bug has been fixed whereby over 700 different level-specific fixes (replacing missing or incorrect textures, moving stuck objects, etc.) weren’t being applied.
* Tweaks have been made to the animations of Zombiemen, Shotgun Guys and Mancubi.
* Screenshots are now saved as a 256-color Windows BMP, reducing their size in kilobytes by more than 66%.
* Pillarboxes are no longer saved in screenshots.
* The `PRINTSCRN` key now no longer saves the screen to the clipboard when taking a screenshot.
* Rotation in the automap is now more accurate.
* A bug has been fixed whereby the crosshair could still decelerate from panning while the menu was displayed.
* The red crosses in stimpacks are now darker to be consistent with medikits.
* If a sprite is replaced with a custom sprite in a PWAD, any translucency will be removed and custom offsets won’t be applied.
* The width of pickups are now calculated differently such that they are less likely to be suspended in midair when close to a change in height.
* The **TEKWALL1** texture (for example, as used on the green armor platform in *E1M1: Hangar*) is now displayed the same way as in *Vanilla DOOM*.
* Windows accessibility shortcut keys are disabled during the game.
* The `WINDOWS` key is now also disabled when in a window.
* The pistol sound is now used when toggling messages in the options menu using the `?` and `?` cursor keys.
* Changes have been made to how mouse sensitivity is calculated such that it is now exactly the same as *Chocolate DOOM*.
* The default mouse sensitivity has been increased.
* Blood splats no longer appear on **RROCK0<i>x</i>** animated flats.
* A bug has been fixed whereby if the “reject matrix” in a PWAD is empty, it will create an overflow and cause monsters to behave strangely. Thank you to jeff-d on the Doomworld forums for providing a solution to this.
* The berserk power-up may now be toggled off using **IDBEHOLDS** cheat.
* A bug has been fixed whereby the Lost Soul wouldn’t rotate correctly in *DOOM II’s* cast sequence.
* Translucency may be disabled by setting translucency setting in default.cfg to false.
* The fuzz effect is now applied to the muzzle flash of the player’s weapon when they have the partial invisibility power-up.
* A bug has been fixed whereby some floors weren’t rising or lowering when they should. Thank you to Jon Krazov for bringing this to my attention.
* **key_prevweapon** and **key_nextweapon** will no longer work when in a menu.
* Savegame descriptions are no longer updated to the current map’s name when the player saves a game if they have previously edited it.
* The correct skill level is now saved in savegames.
* The `ENTER` key on the numeric keypad can now be used wherever the main `ENTER` key can be used.
* **DMENUPIC** is now used on the intermission screen in *DOOM II (BFG Edition)*.
* There are no longer any overlapping drop shadows in the menus.
* Keys are now positioned correctly in the status bar.

### DOOM Retro v1.2.1

* *DOOM Retro* is now compiled using *Microsoft Visual Studio Express 2013 for Windows Desktop*.
* *DOOM Retro* is now distributed with version 1.2.14 of **SDL.dll** and version 1.2.12 of **SDL_mixer.dll**.
* **doomretro.exe** now has a new icon.
* If no IWAD file is specified using the **–IWAD** command-line parameter, a standard Windows dialog box entitled “Where’s All the Data?” will now appear where one IWAD, and optionally one or more PWADs, can be selected.
* *DOOM Retro* is now considerably more stable. The game will no longer crash when a Spectre is on the screen in some instances.
* Many internal optimizations have been made.
* A bug has been fixed that was present in *Vanilla DOOM* whereby bullets would pass through monsters in some instances.
* Much greater mouse sensitivity can now be selected in the options menu.
* Minor visual tweaks have been made to the status bar.
* The game will no longer switch to widescreen mode in the options menu if the screen slider is moved all the way to the right and no game is being played.
* The `+` and `–` keys, as well as moving left and right on a gamepad, can no longer be used to toggle messages on and off in the options menu.
* When **default.cfg** is created for the first time, the keyboard control variables will now be saved as their actual character values rather than their scan codes.
* Blood splats are now left on the ground wherever blood falls. (They may be disabled by setting **bloodsplats** to **false** in **default.cfg**.)
* The fuzz effect of Spectres now looks better while the game is paused or a menu is displayed.
* A bug has been fixed whereby the chainsaw could not be selected by the player unless they also had a berserk power-up.
* The screen will now be wiped at the same speed in widescreen mode.
* If a value is out of range in default.cfg, the default for that value will be used rather than the closest valid value.
* The **usegamma** value is now checked that it is in range when the game starts.
* When the player stands where there is a change in height (either on the floor or ceiling), that edge is now drawn more accurately.
* When in a confined area, Pain Elementals no longer try to spawn Lost Souls in the wrong places only for them to explode straight away.
* When Lost Souls are killed, they now explode on the spot, rather than their explosion sometimes drifting upwards.
* When Pain Elementals are killed, their explosion is now centered better.
* A bug has been fixed whereby messages weren't always being cleared before taking a screenshot.
* The state of flickering lights, active switches and moving platforms are now saved in savegames. This means that savegames from previous versions of *DOOM Retro* will no longer work.
* When a monster is killed, there is a better chance of its corpse being mirrored horizontally if the corpse of the last monster to be killed wasn't mirrored.
* When more than one monster is killed at exactly the same time, there is now a chance that they will fall randomly out of sync.
* Settings are now saved to **default.cfg** the moment they change, rather than when quitting the game, so if the game crashes or exits with an error, those settings will be restored.
* The player’s weapon now isn’t as distorted at reduced screen sizes.
* A bug has been fixed whereby the muzzle of the super shotgun was translucent in some instances.
* 33% alpha translucency rather additive translucency is now used for the SoulSphere, MegaSphere, Invincibility and Partial Invisibility power-ups.
* The `+` and `–` keys can no longer be used while the help screen is displayed.
* In those levels that require one or more monsters to be killed for a sector to move to complete the level, if the **–NOMONSTERS** command-line parameter is specified, those sectors will now automatically move.
* Replicating what happens in *Heretic* and *Hexen*, the remaining monsters in the level will turn on each other once the player has been killed. The player will face their killer when they die, but unlike those games, their view won’t continue to follow their killer around.
* A bug has been fixed whereby *DOOM’s* episode menu would be displayed when pressing the `ESC` key on *DOOM II’s* skill level menu.
* The **IDCHOPPERS** cheat will now be cancelled (by removing the invulnerability power-up and the chainsaw) when the player switches to or picks up a weapon other than the chainsaw.
* Many monsters are now positioned and animate better in *DOOM II’s* cast sequence.
* Monsters now can’t be rotated in the cast sequence until they are actually on the screen.
* Z-coordinates are now taken into account when telefragging.
* Arch-viles no longer resurrect monsters such that they become stuck in other monsters, or under doors.
* A bug has been fixed whereby the game may crash when an Arch-vile resurrects a monster after a savegame has been loaded.
* The boss in *MAP30: Icon Of Sin* at the end of *DOOM II: Hell On Earth* will now still make its alert sound when the level starts if **–NOMONSTERS** is specified on the command-line.
* A “rotate mode” is now available in the automap. Toggled on and off using the R key, it will dynamically orientate the level such that the player is always pointing towards the top of the screen. Rotate mode is off by default, and whether it is on or off is remembered between games.
* Only one instance of **doomretro.exe** can be run at a time.
* The player arrow in the automap will now be translucent while the player has a partial invisibility power-up.
* In *DOOM II (BFG Edition)*, since **TITLEPIC** isn’t present in the IWAD, the otherwise unused **DMENUPIC** is now used instead of **INTERPIC**.

### DOOM Retro v1.1

* The source code is no longer distributed with *DOOM Retro* itself, and has instead been made separately available in a *GitHub* repository. Please visit this site to follow the latest daily developments, and report any issues that you may encounter.
* The correct and complete version information is now displayed when right-clicking on **doomretro.exe** and selecting “Properties”.
* The **–FILE** command-line parameter may no longer be used with *DOOM Shareware’s* WAD file, **DOOM1.WAD**.
* If a **default.cfg** file is present in the game folder, it will now be used in preference to the **default.cfg** saved in **AppData\Local\DOOM RETRO** for the current Windows user.
* While still maintaining backwards compatibility, the values that may be specified in the default.cfg file are now much more readable, and easier to edit manually.
* Gamepad controls can now be customized by editing the default.cfg file.
* A bug has been fixed whereby the game wouldn’t be rendered correctly for displays with an aspect ratio less than 4:3 (that is, displays that aren’t widescreen).
* In fullscreen mode on a widescreen display, increasing the screen size with the `+` key to the maximum will now show a widescreen mode without the status bar, and without any of the horizontal stretching prevalent in many other source ports. *DOOM Retro* will revert to the standard 4:3 aspect ratio when on the title, intermission, finale and help screens.
* A bug has been fixed whereby parts of *MAP01: Entryway* and *MAP02: Underhalls* in *DOOM II: Hell On Earth* would become corrupted when using older versions of **DOOM2.WAD**. Two barrels and a Shotgun Guy were missing from *MAP02* as well. (Credit goes to Jon Krazov for his assistance.)
* For a majority of translucent or partially translucent objects, their translucency is now calculated using additive blending rather than alpha blending, resulting in them appearing considerably brighter.
* The blue lights in tall and short techno floor lamps (**MT_MISC29** and **MT_MISC30**) are now translucent.
* The red and green lights in all switches, as well as the exit signs and many computer terminals that appear in most levels are now consistently bright regardless of the surrounding light levels, and the distance from the player (that is, they are “fullbright”).
* Some minor cosmetic changes have been made to the status bar.
* The corpses of Cyberdemons are no longer flipped horizontally at random.
* When the player ends a level by flicking a switch, that switch will now turn on before the screen is wiped.
* If the player has both the invulnerability and the light amplification visor power-ups, and the invulnerability power-up runs out first, the screen will now flash correctly between the inverted grayscale palette and the “fullbright” palette.
* If the player has both a chainsaw and a berserk power-up, pressing the `1` key will now directly switch to either the chainsaw or the fist, depending on which weapon was selected last, rather than always switching to the chainsaw. This selection is also remembered when saving a game.
* If the `SHIFT` key is held down when the `CAPSLOCK` key is on (or vice-versa), the player will walk instead of run, as originally intended.
* Monsters can no longer pass through tall level decorations.
* A bug has been fixed whereby it took approximately twice as many rockets to kill the boss in *MAP30: Icon Of Sin* at the end of *DOOM II: Hell On Earth*.
* Like what can be done at the end of *DOOM 64*, each monster can now be rotated using the `?` and `?` cursor keys during the cast sequence in *DOOM II*.
* The Lost Soul in the cast sequence in *DOOM II* is now partially translucent.
* The explosions when the Lost Soul and the Pain Elemental die in the cast sequence in *DOOM II* are now translucent.
* A bug has been fixed whereby the cast sequence in *DOOM II* could not be advanced by pressing the `CTRL` key.
* The help screen has been updated to include the new controls for the keyboard, mouse and gamepad, and fixing several inconsistencies. (Credit goes to Robin “FrightNight” Reisinger for his assistance.)
* The text on the help screen now has drop shadows.
* The help screen’s background is now a low resolution snapshot of the game screen with a dark blue tint.
* The controls for selecting the plasma rifle and BFG 9000 are no longer present on the help screen in *DOOM Shareware*.
* When using a PWAD, screenshot filenames will now be of the format **E<i>x</i>M<i>y</i>.bmp** or **MAP<i>xx</i>.bmp** rather than incorrectly use the name of the map this map replaces in the main IWAD.
* When a screenshot is taken using the `PRINTSCREEN` key, any messages are now cleared from the top of the screen first.
* A bug has been fixed whereby if a game is saved while a platform is moving, it could potentially cause the game to crash when that savegame is loaded.
* When using the **IDCLEV<i>xy</i>** cheat, keycards and skull keys are no longer removed from the status bar before the screen is wiped.
* The correct message is now displayed when entering the **IDKFA** cheat.
