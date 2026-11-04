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

// Built-in OBS link. Connects to the local obs-websocket server using the
// settings OBS saved, and reconnects whenever OBS is opened or restarted.
void I_InitOBSLink(void);
void I_ShutdownOBSLink(void);
void I_PollOBSLink(void);
void I_OBSLinkEscape(const char *in, char *out, const size_t size);

// mapnum and author must already be escaped. title must not be.
void I_OBSLinkMap(const char *mapnum, const char *title, const char *author);

// Broadcasts a game event through OBS as a CustomEvent. name may be NULL.
// format adds extra JSON fields, for example "\"amount\":%i", and may be NULL.
void I_OBSLinkEvent(const char *event, const char *name, const char *format, ...);
