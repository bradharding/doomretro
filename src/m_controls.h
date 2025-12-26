/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.

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

==============================================================================
*/

#pragma once

#include "doomkeys.h"

extern int  keyboardalwaysrun;
extern int  keyboardalwaysrun2;
extern int  keyboardautomap;
extern int  keyboardautomap2;
extern int  keyboardback;
extern int  keyboardback2;
extern int  keyboardbfg9000;
extern int  keyboardbfg90002;
extern int  keyboardchaingun;
extern int  keyboardchaingun2;
extern int  keyboardchainsaw;
extern int  keyboardchainsaw2;
extern int  keyboardclearmark;
extern int  keyboardclearmark2;
extern int  keyboardconsole;
extern int  keyboardconsole2;
extern int  keyboardfire;
extern int  keyboardfire2;
extern int  keyboardfists;
extern int  keyboardfists2;
extern int  keyboardfollowmode;
extern int  keyboardfollowmode2;
extern int  keyboardforward;
extern int  keyboardforward2;
extern int  keyboardfreelook;
extern int  keyboardfreelook2;
extern int  keyboardgrid;
extern int  keyboardgrid2;
extern int  keyboardjump;
extern int  keyboardjump2;
extern int  keyboardleft;
extern int  keyboardleft2;
extern int  keyboardmark;
extern int  keyboardmark2;
extern int  keyboardmaxzoom;
extern int  keyboardmaxzoom2;
extern int  keyboardmenu;
extern int  keyboardmenu2;
extern int  keyboardnextweapon;
extern int  keyboardnextweapon2;
extern int  keyboardpath;
extern int  keyboardpath2;
extern int  keyboardpistol;
extern int  keyboardpistol2;
extern int  keyboardplasmarifle;
extern int  keyboardplasmarifle2;
extern int  keyboardprevweapon;
extern int  keyboardprevweapon2;
extern int  keyboardright;
extern int  keyboardright2;
extern int  keyboardrocketlauncher;
extern int  keyboardrocketlauncher2;
extern int  keyboardrotatemode;
extern int  keyboardrotatemode2;
extern int  keyboardrun;
extern int  keyboardrun2;
extern int  keyboardscreenshot;
extern int  keyboardscreenshot2;
extern int  keyboardshotgun;
extern int  keyboardshotgun2;
extern int  keyboardstrafe;
extern int  keyboardstrafe2;
extern int  keyboardstrafeleft;
extern int  keyboardstrafeleft2;
extern int  keyboardstraferight;
extern int  keyboardstraferight2;
extern int  keyboardsupershotgun;
extern int  keyboardsupershotgun2;
extern int  keyboarduse;
extern int  keyboarduse2;
extern int  keyboardweapon1A;
extern int  keyboardweapon1B;
extern int  keyboardweapon2A;
extern int  keyboardweapon2B;
extern int  keyboardweapon3A;
extern int  keyboardweapon3B;
extern int  keyboardweapon4A;
extern int  keyboardweapon4B;
extern int  keyboardweapon5A;
extern int  keyboardweapon5B;
extern int  keyboardweapon6A;
extern int  keyboardweapon6B;
extern int  keyboardweapon7A;
extern int  keyboardweapon7B;
extern int  keyboardzoomin;
extern int  keyboardzoomin2;
extern int  keyboardzoomout;
extern int  keyboardzoomout2;

extern int  mousealwaysrun;
extern int  mouseautomap;
extern int  mouseback;
extern int  mousebfg9000;
extern int  mousechaingun;
extern int  mousechainsaw;
extern int  mouseclearmark;
extern int  mouseconsole;
extern int  mousefire;
extern int  mousefists;
extern int  mousefollowmode;
extern int  mouseforward;
extern int  mousefreelook;
extern int  mousegrid;
extern int  mousejump;
extern int  mouseleft;
extern int  mousemark;
extern int  mousemaxzoom;
extern int  mousemenu;
extern int  mousenextweapon;
extern int  mousepath;
extern int  mousepistol;
extern int  mouseplasmarifle;
extern int  mouseprevweapon;
extern int  mouseright;
extern int  mouserocketlauncher;
extern int  mouserotatemode;
extern int  mouserun;
extern int  mousescreenshot;
extern int  mouseshotgun;
extern int  mousestrafe;
extern int  mousestrafeleft;
extern int  mousestraferight;
extern int  mousesupershotgun;
extern int  mouseuse;
extern int  mouseweapon1;
extern int  mouseweapon2;
extern int  mouseweapon3;
extern int  mouseweapon4;
extern int  mouseweapon5;
extern int  mouseweapon6;
extern int  mouseweapon7;
extern int  mousezoomin;
extern int  mousezoomout;

extern int  controlleralwaysrun;
extern int  controllerautomap;
extern int  controllerback;
extern int  controllerbfg9000;
extern int  controllerchaingun;
extern int  controllerchainsaw;
extern int  controllerclearmark;
extern int  controllerconsole;
extern int  controllerfire;
extern int  controllerfists;
extern int  controllerfollowmode;
extern int  controllerforward;
extern int  controllerfreelook;
extern int  controllergrid;
extern int  controllerjump;
extern int  controllerleft;
extern int  controllermark;
extern int  controllermaxzoom;
extern int  controllermenu;
extern int  controllernextweapon;
extern int  controllerpath;
extern int  controllerpistol;
extern int  controllerplasmarifle;
extern int  controllerprevweapon;
extern int  controllerright;
extern int  controllerrocketlauncher;
extern int  controllerrotatemode;
extern int  controllerrun;
extern int  controllerscreenshot;
extern int  controllershotgun;
extern int  controllerstrafe;
extern int  controllerstrafeleft;
extern int  controllerstraferight;
extern int  controllersupershotgun;
extern int  controlleruse;
extern int  controlleruse2;
extern int  controllerweapon1;
extern int  controllerweapon2;
extern int  controllerweapon3;
extern int  controllerweapon4;
extern int  controllerweapon5;
extern int  controllerweapon6;
extern int  controllerweapon7;
extern int  controllerzoomin;
extern int  controllerzoomout;
