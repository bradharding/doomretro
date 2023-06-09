/*
==============================================================================

                                 DOOM Retro
           The classic, refined DOOM source port. For Windows PC.

==============================================================================

    Copyright © 1993-2023 by id Software LLC, a ZeniMax Media company.
    Copyright © 2013-2023 by Brad Harding <mailto:brad@doomretro.com>.

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

#include <ctype.h>
#include <string.h>

#include "c_console.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "m_cheat.h"
#include "m_misc.h"

//
// CHEAT SEQUENCE PACKAGE
//

//
// Called in st_stuff.c, which handles the input.
// Returns true if the cheat was successful, false if it failed.
//
char    cheatkey = '\0';

bool cht_CheckCheat(cheatseq_t *cht, unsigned char key)
{
    if (*consolecheat && M_StringCompare(consolecheat, cht->sequence))
    {
        consolecheat[0] = '\0';

        if (*consolecheatparm)
        {
            cht->parameter_buf[0] = consolecheatparm[0];
            cht->parameter_buf[1] = consolecheatparm[1];
            cht->param_chars_read = 2;
            consolecheatparm[0] = '\0';
        }

        return true;
    }

    // [BH] you have 2 or 4 seconds to enter all characters of a cheat sequence correctly
    if (cht->timeout && maptime - cht->timeout > (cht->longtimeout ? HU_MSGTIMEOUT : HU_MSGTIMEOUT / 2))
    {
        cht->chars_read = 0;
        cht->param_chars_read = 0;
    }

    cht->timeout = maptime;

    if (cht->chars_read < strlen(cht->sequence))
    {
        // still reading characters from the cheat code
        // and verifying. reset back to the beginning
        // if a key is wrong
        if (toupper(key) == toupper(cht->sequence[cht->chars_read]))
            cht->chars_read++;
        else
            // [BH] recognize key as first in sequence if it matches, rather than resetting
            cht->chars_read = (toupper(key) == toupper(cht->sequence[0]));

        cht->param_chars_read = 0;

        if (cht->chars_read > 1 && !cht->movekey)
            cheatkey = key;
    }
    else if (cht->param_chars_read < cht->parameter_chars)
    {
        // we have passed the end of the cheat sequence and are
        // entering parameters now
        if (!isdigit(key))
        {
            cht->chars_read = 0;
            cht->param_chars_read = 0;
            cht->timeout = 0;
            return false;
        }
        else
        {
            cht->parameter_buf[cht->param_chars_read] = key;
            cht->param_chars_read++;
        }
    }

    if (cht->chars_read >= strlen(cht->sequence) && cht->param_chars_read >= cht->parameter_chars)
    {
        cht->chars_read = 0;
        cht->param_chars_read = 0;
        cht->timeout = 0;
        return true;
    }

    // cheat not matched yet
    return false;
}

void cht_GetParam(cheatseq_t *cht, char *buffer)
{
    memcpy(buffer, cht->parameter_buf, cht->parameter_chars);
}
