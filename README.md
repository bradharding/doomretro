![](http://1.bp.blogspot.com/-XcEjTtLugTQ/Uwey9Mms14I/AAAAAAAAEno/Z9_8h5mzHH4/s1600/title.png)

#### A classic, refined DOOM source port. For Windows PC.

---

*DOOM RETRO* is a classic, refined *DOOM* source port. It represents what I want *DOOM* to be today, in all its dark, gritty, unapologetically pixelated glory. It’s a meticulously crafted expression in restrained design. I’ve strived to implement a set of features and a certain level of attention to detail that isn’t necessarily present in other source ports, but still upholding a deference for that classic, nostalgic *DOOM* experience we all hold dear.

##### How do I play a game?

As with all *DOOM* source ports, *DOOM RETRO* doesn’t come with any actual level data. To play *DOOM RETRO*, this level data first needs to be aquired by purchasing an official release of *DOOM* separately, such as through Steam. Once you have that on your PC, you then need to tell *DOOM RETRO* where this data is. There are 2 ways to go about this: either with or without command-line parameters.

The easiest way is to just double-click on **doomretro.exe** directly. A dialog box will open asking you where the data is. Navigate to where the data was installed and select one of the following WAD files that *DOOM RETRO* supports:

WAD File    |Game
------------|-------------------------------------
DOOM1.WAD   |*DOOM Shareware*
DOOM.WAD    |*The Ultimate DOOM*
DOOM2.WAD   |*DOOM II: Hell On Earth*
NERVE.WAD   |*DOOM II: No Rest For The Living*
PLUTONIA.WAD|*Final DOOM: The Plutonia Experiment*
TNT.WAD     |*Final DOOM: TNT - Evilution*

The next time *DOOM RETRO* is opened, the folder that you navigated to before will open again.

Alternatively, you may want to create shortcuts for each WAD that you own. To do this, right-click on **doomretro.exe** and click “Create shortcut”. Right-click on the new shortcut that was created and click “Properties”. At the end of the text in the “Target” field, add the **-IWAD** command-line parameter followed by the full path of your chosen WAD file. For example:
```
doomretro.exe -IWAD C:\DOOM2\DOOM2.WAD
```

##### What are the controls?

You can play *DOOM RETRO* using the keyboard, mouse or gamepad. For a list of all the controls, start a game and press the F1 key.

##### How do I configure the controls?

*DOOM RETRO’s* settings are saved in a text file called **default.cfg**. This file is stored in **AppData\Local\DOOM RETRO\** in the current Windows user’s personal folder.

---

*DOOM RETRO* is Copyright © 2013-2014 by [Brad Harding](mailto:brad@doomretro.com). All rights reserved. Made in Sydney, Australia. ABN 37 234 369 257. *DOOM RETRO* is free software: you can redistribute it and/or modify it under the terms of the [*GNU General Public License*](http://www.gnu.org/licenses/gpl.html) as published by the [Free Software Foundation](http://www.fsf.org/), either version 3 of the License, or (at your option) any later version. *DOOM RETRO* is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the [*GNU General Public License*](http://www.gnu.org/licenses/gpl.html) for more details. *DOOM* is a registered trademark of [id Software LLC](http://www.idsoftware.com), a [ZeniMax Media](http://www.zenimax.com/) company, in the US and/or other countries, and is used without permission. All other trademarks are the property of their respective holder. *DOOM RETRO* is in no way affiliated with nor endorsed by [id Software LLC](http://www.idsoftware.com).
