/*
========================================================================

  DOOM RETRO
  The classic, refined DOOM source port. For Windows PC.
  Copyright (C) 2013-2014 Brad Harding.

  This file is part of DOOM RETRO.

  DOOM RETRO is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  DOOM RETRO is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with DOOM RETRO. If not, see <http://www.gnu.org/licenses/>.

========================================================================
*/

#ifndef __D_ENGLSH__
#define __D_ENGLSH__

//
//  Printed strings for translation
//

//
// m_menu.c
//
#define PRESSKEY                "(Press any key to continue.)"
#define PRESSYN                 "(Press 'Y' or 'N' to continue.)"
#define PRESSA                  "(Press button 'A' to continue.)"
#define QLPROMPT                "Do you want to quickload the savegame \"%s\"?"
#define NIGHTMARE               "Are you sure? This skill level\n"\
                                "isn't even remotely fair!"
#define SWSTRING                "This is the shareware version of DOOM.\n"\
                                "You need to purchase the full version."

#define DETAILHI                "High graphic detail"
#define DETAILLO                "Low graphic detail"

#define MSGOFF                  "Messages OFF"
#define MSGON                   "Messages ON"
#define ENDGAME                 "Are you sure you want\n"\
                                "to end this game?"

#if defined(WIN32)
#define QUITY                   "(Press 'Y' to quit to Windows.)"
#define QUITA                   "(Press button 'A' to quit to Windows.)"
#elif defined(__MACOSX__)
#define QUITY                   "(Press 'Y' to quit to OS X.)"
#define QUITA                   "(Press button 'A' to quit to OS X.)"
#else
#define QUITY                   "(Press 'Y' to quit to Linux.)"
#define QUITA                   "(Press button 'A' to quit to Linux.)"
#endif

#define GAMMALVL                "Gamma correction level %.2f"
#define GAMMAOFF                "Gamma correction off"

#define EMPTYSTRING             "  -"

//
// p_inter.c
//
#define GOTARMOR                "You picked up some armor."
#define GOTMEGA                 "You picked up some MegaArmor!"
#define GOTHTHBONUS             "You picked up a health bonus."
#define GOTARMBONUS             "You picked up an armor bonus."
#define GOTSTIM                 "You picked up a stimpack."
#define GOTMEDINEED             "You picked up a medikit that you REALLY need!"
#define GOTMEDIKIT              "You picked up a medikit."
#define GOTSUPER                "SoulSphere!"

#define GOTBLUECARD             "You picked up a blue keycard."
#define GOTYELWCARD             "You picked up a yellow keycard."
#define GOTREDCARD              "You picked up a red keycard."
#define GOTBLUESKUL             "You picked up a blue skull key."
#define GOTYELWSKUL             "You picked up a yellow skull key."
#define GOTREDSKULL             "You picked up a red skull key."

#define GOTINVUL                "Invulnerability!"
#define GOTBERSERK              "Berserk!"
#define GOTINVIS                "Partial Invisibility!"
#define GOTSUIT                 "Radiation Shielding Suit!"
#define GOTMAP                  "Computer Area Map!"
#define GOTVISOR                "Light Amplification Visor!"
#define GOTMSPHERE              "MegaSphere!"

#define GOTCLIP                 "You picked up a clip."
#define GOTCLIPBOX              "You picked up a box of bullets."
#define GOTROCKET               "You picked up a rocket."
#define GOTROCKBOX              "You picked up a box of rockets."
#define GOTCELL                 "You picked up a cell."
#define GOTCELLBOX              "You picked up a cell pack."
#define GOTSHELLS               "You picked up some shotgun shells."
#define GOTSHELLBOX             "You picked up a box of shotgun shells."
#define GOTBACKPACK             "You picked up a backpack full of ammo!"

#define GOTBFG9000              "You got a BFG 9000! Oh, yes..."
#define GOTCHAINGUN             "You got a chaingun!"
#define GOTCHAINSAW             "You got a chainsaw! Find some meat!"
#define GOTLAUNCHER             "You got a rocket launcher!"
#define GOTPLASMA               "You got a plasma rifle!"
#define GOTSHOTGUN              "You got a shotgun!"
#define GOTSHOTGUN2             "You got a super shotgun!"

//
// p_doors.c
//
#define PD_BLUEO                "You need a blue keycard to use this."
#define PD_BLUEO2               "You need a blue skull key to use this."
#define PD_REDO                 "You need a red keycard to use this."
#define PD_REDO2                "You need a red skull key to use this."
#define PD_YELLOWO              "You need a yellow keycard to use this."
#define PD_YELLOWO2             "You need a yellow skull key to use this."
#define PD_BLUEK                "You need a blue keycard to open this door."
#define PD_BLUEK2               "You need a blue skull key to open this door."
#define PD_REDK                 "You need a red keycard to open this door."
#define PD_REDK2                "You need a red skull key to open this door."
#define PD_YELLOWK              "You need a yellow keycard to open this door."
#define PD_YELLOWK2             "You need a yellow skull key to open this door."

//
// g_game.c
//
#define GGSAVED                 "\"%s\" saved"
#define GSCREENSHOT             "\"%s\" saved"

#define ALWAYSRUNOFF            "Always run OFF"
#define ALWAYSRUNON             "Always run ON"

//
//  hu_stuff.c
//
#define HUSTR_E1M1              "Hangar"
#define HUSTR_E1M2              "Nuclear Plant"
#define HUSTR_E1M3              "Toxin Refinery"
#define HUSTR_E1M4              "Command Control"
#define HUSTR_E1M5              "Phobos Lab"
#define HUSTR_E1M6              "Central Processing"
#define HUSTR_E1M7              "Computer Station"
#define HUSTR_E1M8              "Phobos Anomaly"
#define HUSTR_E1M9              "Military Base"

#define HUSTR_E2M1              "Deimos Anomaly"
#define HUSTR_E2M2              "Containment Area"
#define HUSTR_E2M3              "Refinery"
#define HUSTR_E2M4              "Deimos Lab"
#define HUSTR_E2M5              "Command Center"
#define HUSTR_E2M6              "Halls of the Damned"
#define HUSTR_E2M7              "Spawning Vats"
#define HUSTR_E2M8              "Tower of Babel"
#define HUSTR_E2M9              "Fortress of Mystery"

#define HUSTR_E3M1              "Hell Keep"
#define HUSTR_E3M2              "Slough of Despair"
#define HUSTR_E3M3              "Pandemonium"
#define HUSTR_E3M4              "House of Pain"
#define HUSTR_E3M5              "Unholy Cathedral"
#define HUSTR_E3M6              "Mt. Erebus"
#define HUSTR_E3M7              "Gate to Limbo"
#define HUSTR_E3M8              "Dis"
#define HUSTR_E3M9              "Warrens"

#define HUSTR_E4M1              "Hell Beneath"
#define HUSTR_E4M2              "Perfect Hatred"
#define HUSTR_E4M3              "Sever the Wicked"
#define HUSTR_E4M4              "Unruly Evil"
#define HUSTR_E4M5              "They Will Repent"
#define HUSTR_E4M6              "Against Thee Wickedly"
#define HUSTR_E4M7              "And Hell Followed"
#define HUSTR_E4M8              "Unto the Cruel"
#define HUSTR_E4M9              "Fear"

#define HUSTR_1                 "Entryway"
#define HUSTR_2                 "Underhalls"
#define HUSTR_3                 "The Gantlet"
#define HUSTR_4                 "The Focus"
#define HUSTR_5                 "The Waste Tunnels"
#define HUSTR_6                 "The Crusher"
#define HUSTR_7                 "Dead Simple"
#define HUSTR_8                 "Tricks and Traps"
#define HUSTR_9                 "The Pit"
#define HUSTR_10                "Refueling Base"
#define HUSTR_11                "'O' of Destruction!"

#define HUSTR_12                "The Factory"
#define HUSTR_13                "Downtown"
#define HUSTR_14                "The Inmost Dens"
#define HUSTR_15                "Industrial Zone"
#define HUSTR_16                "Suburbs"
#define HUSTR_17                "Tenements"
#define HUSTR_18                "The Courtyard"
#define HUSTR_19                "The Citadel"
#define HUSTR_20                "Gotcha!"

#define HUSTR_21                "Nirvana"
#define HUSTR_22                "The Catacombs"
#define HUSTR_23                "Barrels o' Fun"
#define HUSTR_24                "The Chasm"
#define HUSTR_25                "Bloodfalls"
#define HUSTR_26                "The Abandoned Mines"
#define HUSTR_27                "Monster Condo"
#define HUSTR_28                "The Spirit World"
#define HUSTR_29                "The Living End"
#define HUSTR_30                "Icon of Sin"

#define HUSTR_31                "Wolfenstein"
#define HUSTR_32                "Grosse"

#define HUSTR_31_BFG            "IDKFA"
#define HUSTR_32_BFG            "Keen"
#define HUSTR_33_BFG            "Betray"

#define PHUSTR_1                "Congo"
#define PHUSTR_2                "Well of Souls"
#define PHUSTR_3                "Aztec"
#define PHUSTR_4                "Caged"
#define PHUSTR_5                "Ghost Town"
#define PHUSTR_6                "Baron's Lair"
#define PHUSTR_7                "Caughtyard"
#define PHUSTR_8                "Realm"
#define PHUSTR_9                "Abattoire"
#define PHUSTR_10               "Onslaught"
#define PHUSTR_11               "Hunted"

#define PHUSTR_12               "Speed"
#define PHUSTR_13               "The Crypt"
#define PHUSTR_14               "Genesis"
#define PHUSTR_15               "The Twilight"
#define PHUSTR_16               "The Omen"
#define PHUSTR_17               "Compound"
#define PHUSTR_18               "Neurosphere"
#define PHUSTR_19               "NME"
#define PHUSTR_20               "The Death Domain"

#define PHUSTR_21               "Slayer"
#define PHUSTR_22               "Impossible Mission"
#define PHUSTR_23               "Tombstone"
#define PHUSTR_24               "The Final Frontier"
#define PHUSTR_25               "The Temple of Darkness"
#define PHUSTR_26               "Bunker"
#define PHUSTR_27               "Anti-Christ"
#define PHUSTR_28               "The Sewers"
#define PHUSTR_29               "Odyssey of Noises"
#define PHUSTR_30               "The Gateway of Hell"

#define PHUSTR_31               "Cyberden"
#define PHUSTR_32               "Go 2 It"

#define THUSTR_1                "System Control"
#define THUSTR_2                "Human BBQ"
#define THUSTR_3                "Power Control"
#define THUSTR_4                "Wormhole"
#define THUSTR_5                "Hanger"
#define THUSTR_6                "Open Season"
#define THUSTR_7                "Prison"
#define THUSTR_8                "Metal"
#define THUSTR_9                "Stronghold"
#define THUSTR_10               "Redemption"
#define THUSTR_11               "Storage Facility"

#define THUSTR_12               "Crater"
#define THUSTR_13               "Nukage Processing"
#define THUSTR_14               "Steel Works"
#define THUSTR_15               "Dead Zone"
#define THUSTR_16               "Deepest Reaches"
#define THUSTR_17               "Processing Area"
#define THUSTR_18               "Mill"
#define THUSTR_19               "Shipping/Respawning"
#define THUSTR_20               "Central Processing"

#define THUSTR_21               "Administration Center"
#define THUSTR_22               "Habitat"
#define THUSTR_23               "Lunar Mining Project"
#define THUSTR_24               "Quarry"
#define THUSTR_25               "Baron's Den"
#define THUSTR_26               "Ballistyx"
#define THUSTR_27               "Mount Pain"
#define THUSTR_28               "Heck"
#define THUSTR_29               "River Styx"
#define THUSTR_30               "Last Call"

#define THUSTR_31               "Pharaoh"
#define THUSTR_32               "Caribbean"

#define NHUSTR_1                "The Earth Base"
#define NHUSTR_2                "The Pain Labs"
#define NHUSTR_3                "Canyon of the Dead"
#define NHUSTR_4                "Hell Mountain"
#define NHUSTR_5                "Vivisection"
#define NHUSTR_6                "Inferno of Blood"
#define NHUSTR_7                "Baron's Banquet"
#define NHUSTR_8                "Tomb of Malevolence"
#define NHUSTR_9                "March of the Demons"

//
// am_map.c
//
#define AMSTR_FOLLOWON          "Follow mode ON"
#define AMSTR_FOLLOWOFF         "Follow mode OFF"

#define AMSTR_GRIDON            "Grid ON"
#define AMSTR_GRIDOFF           "Grid OFF"

#define AMSTR_MARKEDSPOT        "Marked spot %i"
#define AMSTR_MARKCLEARED       "Mark cleared"
#define AMSTR_MARKSCLEARED      "Marks cleared"

#define AMSTR_ROTATEON          "Rotate Mode ON"
#define AMSTR_ROTATEOFF         "Rotate Mode OFF"

//
//  st_stuff.c
//
#define STSTR_MUS               "Music changed to \"d_%s.mus\""

#define STSTR_DQDON             "Degreelessness mode ON"
#define STSTR_DQDOFF            "Degreelessness mode OFF"

#define STSTR_KFAADDED          "Very happy ammo added"
#define STSTR_FAADDED           "Ammo (no keys) added"

#define STSTR_NCON              "No clipping mode ON"
#define STSTR_NCOFF             "No clipping mode OFF"

#define STSTR_BEHOLD            "inVuln, bSrk, Inviso, Rad, Allmap or Lite-amp?"
#define STSTR_BEHOLDON          "Power-up toggled ON"
#define STSTR_BEHOLDOFF         "Power-up toggled OFF"

#define STSTR_CHOPPERS          "\"...doesn't suck.\" - GM"

#define STSTR_CLEV              "Warping to %s..."
#define STSTR_CLEVSAME          "Restarting %s..."

#define STSTR_MYPOS             "angle = %i%c  x,y,z = (%i,%i,%i)"

//
//  f_finale.c
//
#define E1TEXT \
"Once you beat the big badasses and\n"\
"clean out the moon base, you're supposed\n"\
"to win, aren't you? Aren't you? Where's\n"\
"your fat reward and ticket home? What\n"\
"the Hell is this? It's not supposed to\n"\
"end this way!\n"\
"\n" \
"It stinks like rotten meat, but looks\n"\
"like the lost Deimos base. It looks like\n"\
"you're stuck on The Shores of Hell.\n"\
"The only way out is through.\n"\
"\n"\
"To continue the DOOM experience, play\n"\
"The Shores of Hell and its amazing\n"\
"sequel, Inferno!\n"

#define E2TEXT \
"You've done it! The hideous Cyber-\n"\
"demon lord that ruled the lost Deimos\n"\
"moon base has been slain and you\n"\
"are triumphant! But... where are\n"\
"you? You clamber to the edge of the\n"\
"moon and look down to see the awful\n"\
"truth.\n" \
"\n"\
"Deimos floats above Hell itself!\n"\
"You've never heard of anyone escaping\n"\
"from Hell, but you'll make the bastards\n"\
"sorry they ever heard of you! Quickly,\n"\
"you rappel down to the surface of\n"\
"Hell.\n"\
"\n" \
"Now, it's onto the final chapter of\n"\
"DOOM - Inferno!"

#define E3TEXT \
"The loathsome spiderdemon that\n"\
"masterminded the invasion of the moon\n"\
"bases and caused so much death has had\n"\
"its ass kicked for all time.\n"\
"\n"\
"A hidden doorway opens and you enter.\n"\
"You've proven too tough for Hell to\n"\
"contain, and now Hell at last plays\n"\
"fair - for you emerge from the door\n"\
"to see the green fields of Earth!\n"\
"Home at last.\n" \
"\n"\
"You wonder what's been happening on\n"\
"Earth while you were battling evil\n"\
"unleashed. It's good that no Hell-\n"\
"spawn could have come through that\n"\
"door with you..."

#define E4TEXT \
"The Spider Mastermind must have sent forth\n"\
"its legions of Hellspawn before your\n"\
"final confrontation with that terrible\n"\
"beast from Hell. But you stepped forward\n"\
"and brought forth eternal damnation and\n"\
"suffering upon the horde as a true hero\n"\
"would in the face of something so evil.\n"\
"\n"\
"Besides, someone was gonna pay for what\n"\
"happened to Daisy, your pet rabbit.\n"\
"\n"\
"But now, you see spread before you more\n"\
"potential pain and gibbitude as a nation\n"\
"of demons run amok among our cities.\n"\
"\n"\
"Next stop, Hell on Earth!"

// After level 6, put this:
#define C1TEXT \
"You have entered deeply into the infested\n" \
"starport. But something is wrong. The\n" \
"monsters have brought their own reality\n" \
"with them, and the starport's technology\n" \
"is being subverted by their presence.\n" \
"\n"\
"Ahead, you see an outpost of Hell, a\n" \
"fortified zone. If you can get past it,\n" \
"you can penetrate into the haunted heart\n" \
"of the starbase and find the controlling\n" \
"switch which holds Earth's population\n" \
"hostage."

// After level 11, put this:
#define C2TEXT \
"You have won! Your victory has enabled\n" \
"humankind to evacuate Earth and escape\n"\
"the nightmare. Now you are the only\n"\
"human left on the face of the planet.\n"\
"Cannibal mutations, carnivorous aliens,\n"\
"and evil spirits are your only neighbors.\n"\
"You sit back and wait for death, content\n"\
"that you have saved your species.\n"\
"\n"\
"But then, Earth Control beams down a\n"\
"message from space: \"Sensors have located\n"\
"the source of the alien invasion. If you\n"\
"go there, you may be able to block their\n"\
"entry. The alien base is in the heart of\n"\
"your own home city, not far from the\n"\
"starport.\" Slowly and painfully you get\n"\
"up and return to the fray."

// After level 20, put this:
#define C3TEXT \
"You are at the corrupt heart of the city,\n"\
"surrounded by the corpses of your enemies.\n"\
"You see no way to destroy the creatures'\n"\
"entryway on this side, so you clench your\n"\
"teeth and plunge through it.\n"\
"\n"\
"There must be a way to close it on the\n"\
"other side. What do you care if you've\n"\
"got to go through Hell to get to it?"

// After level 29, put this:
#define C4TEXT \
"The horrendous visage of the biggest\n"\
"demon you've ever seen crumbles before\n"\
"you, after you pump your rockets into\n"\
"its exposed brain. The monster shrivels\n"\
"up and dies, its thrashing limbs\n"\
"devastating untold miles of Hell's\n"\
"surface.\n"\
"\n"\
"You've done it. The invasion is over.\n"\
"Earth is saved. Hell is a wreck. You\n"\
"wonder where bad folks will go when they\n"\
"die, now. Wiping the sweat from your\n"\
"forehead you begin the long trek back\n"\
"home. Rebuilding Earth ought to be a\n"\
"lot more fun than ruining it was.\n"

// Before level 31, put this:
#define C5TEXT \
"Congratulations, you've found the secret\n"\
"level! Looks like it's been built by\n"\
"humans, rather than demons. You wonder\n"\
"who the inmates of this corner of Hell\n"\
"will be."

// Before level 32, put this:
#define C6TEXT \
"Congratulations, you've found the\n"\
"super secret level! You'd better\n"\
"blaze through this one!\n"

// After level 8, put this:
#define N1TEXT \
"Trouble was brewing again in your favorite\n"\
"vacation spot... Hell. Some Cyberdemon\n"\
"punk thought he could turn hell into a\n"\
"personal amusement park, and make Earth\n"\
"the ticket booth.\n"\
"\n"\
"Well, that half-robot freak show didn't\n"\
"know who was coming to the fair. There's\n"\
"nothing like a shooting gallery full of\n"\
"hellspawn to get the blood pumping...\n"\
"\n"\
"Now the walls of the demon's labyrinth\n"\
"echo with the sound of his metallic limbs\n"\
"hitting the floor. His death moan gurgles\n"\
"out through the mess you left of his face.\n"\
"\n"\
"This ride is closed."

// After map 06, put this:
#define P1TEXT \
"You gloat over the steaming carcass of the\n"\
"Guardian. With its death, you've wrested\n"\
"the Accelerator from the stinking claws\n"\
"of Hell. You relax and glance around the\n"\
"room. Damn! There was supposed to be at\n"\
"least one working prototype, but you can't\n"\
"see it. The demons must have taken it.\n"\
"\n"\
"You must find the prototype, or all your\n"\
"struggles will have been wasted. Keep\n"\
"moving, keep fighting, keep killing.\n"\
"Oh yes, keep living, too."

// After map 11, put this:
#define P2TEXT \
"Even the deadly Arch-Vile labyrinth could\n"\
"not stop you, and you've gotten to the\n"\
"prototype Accelerator which is soon\n"\
"efficiently and permanently deactivated.\n"\
"\n"\
"You're good at that kind of thing."

// After map 20, put this:
#define P3TEXT \
"You've bashed and battered your way into\n"\
"the heart of the devil-hive. Time for a\n"\
"Search-and-Destroy mission, aimed at the\n"\
"Gatekeeper, whose foul offspring is\n"\
"cascading to Earth. Yeah, he's bad. But\n"\
"you know who's worse!\n"\
"\n"\
"Grinning evilly, you check your gear, and\n"\
"get ready to give the bastard a little Hell\n"\
"of your own making!"

// After map 30, put this:
#define P4TEXT \
"The Gatekeeper's evil face is splattered\n"\
"all over the place. As its tattered corpse\n"\
"collapses, an inverted Gate forms and\n"\
"sucks down the shards of the last\n"\
"prototype Accelerator, not to mention the\n"\
"few remaining demons. You're done. Hell\n"\
"has gone back to pounding bad dead folks\n"\
"instead of good live ones. Remember to\n"\
"tell your grandkids to put a rocket\n"\
"launcher in your coffin. If you go to Hell\n"\
"when you die, you'll need it for some\n"\
"final cleaning-up..."

// Before map 31, put this:

#define P5TEXT \
"You've found the second-hardest level we\n"\
"got. Hope you have a saved game a level or\n"\
"two previous. If not, be prepared to die\n"\
"aplenty. For master marines only."

// Before map 32, put this:
#define P6TEXT \
"Betcha wondered just what WAS the hardest\n"\
"level we had ready for ya? Now you know.\n"\
"No one gets out alive."

#define T1TEXT \
"You've fought your way out of the infested\n"\
"experimental labs. It seems that the UAC\n"\
"has once again gulped it down. With their\n"\
"high turnover, it must be hard for the poor\n"\
"old UAC to buy corporate health insurance\n"\
"nowadays...\n"\
"\n"\
"Ahead lies the military complex, now\n"\
"swarming with diseased horrors hot to get\n"\
"their teeth into you. With luck, the\n"\
"complex still has some warlike ordnance\n"\
"laying around."

#define T2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead. You sure hope they're not stamping\n"\
"out new Hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."

#define T3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too - like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as Hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."

#define T4TEXT \
"Suddenly, all is silent, from one horizon\n"\
"to the other. The agonizing echo of Hell\n"\
"fades away, the nightmare sky turns to\n"\
"blue, the heaps of monster corpses start \n"\
"to evaporate along with the evil stench \n"\
"that filled the air. Jeez, maybe you've\n"\
"done it. Have you really won?\n"\
"\n"\
"Something rumbles in the distance.\n"\
"A blue light begins to glow inside the\n"\
"ruined skull of the demon-spitter."

#define T5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it? Or maybe it's best\n"\
"to let sleeping Gods lie..."

#define T6TEXT \
"Time for a vacation. You've burst the\n"\
"bowels of Hell and by golly you're ready\n"\
"for a break. You mutter to yourself,\n"\
"\"Maybe someone else can kick Hell's ass\n"\
"next time around.\" Ahead lies a quiet town,\n"\
"with peaceful flowing water, quaint\n"\
"buildings, and presumably no Hellspawn.\n"\
"\n"\
"As you step off the transport, you hear\n"\
"the stomp of a Cyberdemon's iron shoe."

//
// Character cast strings for f_finale.c
//
#define CC_ZOMBIE       "ZOMBIEMAN"
#define CC_SHOTGUN      "SHOTGUN GUY"
#define CC_HEAVY        "HEAVY WEAPON DUDE"
#define CC_IMP          "IMP"
#define CC_DEMON        "DEMON"
#define CC_SPECTRE      "SPECTRE"
#define CC_LOST         "LOST SOUL"
#define CC_CACO         "CACODEMON"
#define CC_HELL         "HELL KNIGHT"
#define CC_BARON        "BARON OF HELL"
#define CC_ARACH        "ARACHNOTRON"
#define CC_PAIN         "PAIN ELEMENTAL"
#define CC_REVEN        "REVENANT"
#define CC_MANCU        "MANCUBUS"
#define CC_ARCH         "ARCH-VILE"
#define CC_SPIDER       "THE SPIDER MASTERMIND"
#define CC_CYBER        "THE CYBERDEMON"
#define CC_HERO         "OUR HERO"

#endif
