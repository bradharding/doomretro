/*
========================================================================

DOOM Retro
The classic, refined DOOM source port. For Windows PC.

========================================================================

Copyright (c) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (c) 2013-2016 Brad Harding.

DOOM Retro is a fork of Chocolate DOOM by Simon Howard.
For a complete list of credits, see the accompanying AUTHORS file.

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

#include "doomtype.h"
#include "i_system.h"
#include "m_misc.h"
#include "sc_man.h"
#include "w_wad.h"
#include "z_zone.h"

#define MAX_STRING_SIZE         64
#define ASCII_COMMENT           ';'
#define ASCII_QUOTE             34
#define LUMP_SCRIPT             1
#define FILE_ZONE_SCRIPT        2

static void CheckOpen(void);
static void OpenScript(char *name, int type);

char            *sc_String;
int             sc_Number;
int             sc_Line;
dboolean        sc_End;
dboolean        sc_Crossed;
dboolean        sc_FileScripts = false;
char            *sc_ScriptsDir = "";

static char     ScriptName[16];
static char     *ScriptBuffer;
static char     *ScriptPtr;
static char     *ScriptEndPtr;
static char     StringBuffer[MAX_STRING_SIZE];
static int      ScriptLumpNum;
static dboolean ScriptOpen = false;
static int      ScriptSize;
static dboolean AlreadyGot = false;

void SC_Open(char *name)
{
    char        fileName[128];

    if (sc_FileScripts == true)
    {
        M_snprintf(fileName, sizeof(fileName), "%s%s.txt", sc_ScriptsDir, name);
        SC_OpenFile(fileName);
    }
    else
        SC_OpenLump(name);
}

void SC_OpenLump(char *name)
{
    OpenScript(name, LUMP_SCRIPT);
}

void SC_OpenFile(char *name)
{
    OpenScript(name, FILE_ZONE_SCRIPT);
}

static void OpenScript(char *name, int type)
{
    SC_Close();
    if (type == LUMP_SCRIPT)
    {
        ScriptLumpNum = W_GetNumForName(name);
        ScriptBuffer = W_CacheLumpNum(ScriptLumpNum, PU_STATIC);
        ScriptSize = W_LumpLength(ScriptLumpNum);
        M_StringCopy(ScriptName, name, sizeof(ScriptName));
    }
    //else if (type == FILE_ZONE_SCRIPT)
    //{
    //    ScriptLumpNum = -1;
    //    ScriptSize = M_ReadFile(name, (byte **)&ScriptBuffer);
    //    M_ExtractFileBase(name, ScriptName);
    //}
    ScriptPtr = ScriptBuffer;
    ScriptEndPtr = ScriptPtr + ScriptSize;
    sc_Line = 1;
    sc_End = false;
    ScriptOpen = true;
    sc_String = StringBuffer;
    AlreadyGot = false;
}

void SC_Close(void)
{
    if (ScriptOpen)
    {
        if (ScriptLumpNum >= 0)
            W_ReleaseLumpNum(ScriptLumpNum);
        else
            Z_Free(ScriptBuffer);
        ScriptOpen = false;
    }
}

dboolean SC_GetString(void)
{
    char        *text;
    dboolean    foundToken;

    CheckOpen();
    if (AlreadyGot)
    {
        AlreadyGot = false;
        return true;
    }
    foundToken = false;
    sc_Crossed = false;
    if (ScriptPtr >= ScriptEndPtr)
    {
        sc_End = true;
        return false;
    }
    while (foundToken == false)
    {
        while (*ScriptPtr <= 32)
        {
            if (ScriptPtr >= ScriptEndPtr)
            {
                sc_End = true;
                return false;
            }
            if (*ScriptPtr++ == '\n')
            {
                sc_Line++;
                sc_Crossed = true;
            }
        }
        if (ScriptPtr >= ScriptEndPtr)
        {
            sc_End = true;
            return false;
        }
        if (*ScriptPtr != ASCII_COMMENT)
            foundToken = true;
        else
        {
            while (*ScriptPtr++ != '\n')
                if (ScriptPtr >= ScriptEndPtr)
                {
                    sc_End = true;
                    return false;
                }
            sc_Line++;
            sc_Crossed = true;
        }
    }
    text = sc_String;
    if (*ScriptPtr == ASCII_QUOTE)
    {
        ScriptPtr++;
        while (*ScriptPtr != ASCII_QUOTE)
        {
            *text++ = *ScriptPtr++;
            if (ScriptPtr == ScriptEndPtr || text == &sc_String[MAX_STRING_SIZE - 1])
                break;
        }
        ScriptPtr++;
    }
    else
    {
        while (*ScriptPtr > 32 && *ScriptPtr != ASCII_COMMENT)
        {
            *text++ = *ScriptPtr++;
            if (ScriptPtr == ScriptEndPtr || text == &sc_String[MAX_STRING_SIZE - 1])
                break;
        }
    }
    *text = 0;
    return true;
}

void SC_MustGetString(void)
{
    if (SC_GetString() == false)
        SC_ScriptError("Missing string.");
}

void SC_MustGetStringName(char *name)
{
    SC_MustGetString();
    if (SC_Compare(name) == false)
        SC_ScriptError(NULL);
}

dboolean SC_GetNumber(void)
{
    char        *stopper;

    CheckOpen();
    if (SC_GetString())
    {
        sc_Number = strtol(sc_String, &stopper, 0);
        //if (*stopper != 0)
        //    I_Error("SC_GetNumber: Bad numeric constant \"%s\".\nScript %s, Line %d",
        //        sc_String, ScriptName, sc_Line);
        return true;
    }
    else
        return false;
}

void SC_MustGetNumber(void)
{
    if (!SC_GetNumber())
        SC_ScriptError("Missing integer.");
}

void SC_UnGet(void)
{
    AlreadyGot = true;
}

int SC_MatchString(char **strings)
{
    int i;

    for (i = 0; *strings; i++)
        if (SC_Compare(*strings++))
            return i;
    return -1;
}

int SC_MustMatchString(char **strings)
{
    int i = SC_MatchString(strings);

    if (i == -1)
        SC_ScriptError(NULL);
    return i;
}

dboolean SC_Compare(char *text)
{
    if (strcasecmp(text, sc_String) == 0)
        return true;
    return false;
}

void SC_ScriptError(char *message)
{
    if (!message)
        message = "Bad syntax.";
    I_Error("Script error, \"%s\" line %d: %s", ScriptName, sc_Line, message);
}

static void CheckOpen(void)
{
    if (ScriptOpen == false)
        I_Error("SC_ call before SC_Open().");
}
