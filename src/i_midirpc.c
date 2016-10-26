/*
========================================================================

                           D O O M  R e t r o
         The classic, refined DOOM source port. For Windows PC.

========================================================================

  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
  Copyright © 2013-2016 Brad Harding.

  DOOM Retro is a fork of Chocolate DOOM.
  For a list of credits, see <http://credits.doomretro.com>.

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
  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.

  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
  company, in the US and/or other countries and is used without
  permission. All other trademarks are the property of their respective
  holders. DOOM Retro is in no way affiliated with nor endorsed by
  id Software.

========================================================================
*/

// Eternity Engine's Client Interface to RPC Midi Server by James Haley

#if defined(WIN32)

#include <windows.h>

#include "../midiproc/midiproc.h"
#include "c_console.h"
#include "doomtype.h"
#include "i_timer.h"
#include "m_misc.h"

//
// Data
//
static unsigned char    *szStringBinding;       // RPC client binding string
static dboolean         serverInit = false;     // if true, server was started
static dboolean         clientInit = false;     // if true, client was bound

// server process information
static STARTUPINFO              si;
static PROCESS_INFORMATION      pi;

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t size)
{
   return malloc(size);
}

void __RPC_USER midl_user_free(void __RPC_FAR *p)
{
   free(p);
}

//
// RPC Wrappers
//

//
// CHECK_RPC_STATUS
//
// If either server or client initialization failed, we don't try to make any
// RPC calls.
//
#define CHECK_RPC_STATUS()         \
    if (!serverInit || !clientInit) \
        return false

// This number * 10 is the amount of time you can try to wait for.
#define MIDIRPC_MAXTRIES        50

static dboolean I_MidiRPCWaitForServer()
{
   int tries = 0;

   while (RpcMgmtIsServerListening(hMidiRPCBinding) != RPC_S_OK)
   {
      I_Sleep(10);
      if (++tries >= MIDIRPC_MAXTRIES)
         return false;
   }
   return true;
}

//
// I_MidiRPCRegisterSong
//
// Prepare the RPC MIDI engine to receive new song data, and transmit the song
// data to the server process.
//
dboolean I_MidiRPCRegisterSong(void *data, int size)
{
   unsigned int rpcSize = (unsigned int)size;

   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_PrepareNewSong();
      MidiRPC_AddChunk(rpcSize, (byte *)data);
   }
   RpcExcept(1)
   {
      return false;
   }
   RpcEndExcept

   return true;
}

//
// I_MidiRPCPlaySong
//
// Tell the RPC server to start playing a song.
//
dboolean I_MidiRPCPlaySong(dboolean looping)
{
   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_PlaySong(looping ? TRUE : FALSE);
   }
   RpcExcept(1)
   {
      return false;
   }
   RpcEndExcept

   return true;
}

// 
// I_MidiRPCStopSong
//
// Tell the RPC server to stop any currently playing song.
//
dboolean I_MidiRPCStopSong()
{
   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_StopSong();
   }
   RpcExcept(1)
   {
      return false;
   }
   RpcEndExcept

   return true;
}

//
// I_MidiRPCSetVolume
//
// Change the volume level of music played by the RPC midi server.
//
dboolean I_MidiRPCSetVolume(int volume)
{
   CHECK_RPC_STATUS();
   
   RpcTryExcept
   {
      MidiRPC_ChangeVolume(volume);
   }
   RpcExcept(1)
   {
      return false;
   }
   RpcEndExcept

   return true;
}

//
// I_MidiRPCPauseSong
//
// Pause the music being played by the server. In actuality, due to SDL_mixer
// limitations, this just temporarily sets the volume to zero.
//
dboolean I_MidiRPCPauseSong()
{
   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_PauseSong();
   }
   RpcExcept(1)
   {
      return false;
   }
   RpcEndExcept

   return true;
}

//
// I_MidiRPCResumeSong
//
// Resume a song after having paused it.
//
dboolean I_MidiRPCResumeSong()
{
   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_ResumeSong();
   }
   RpcExcept(1)
   {
      return false;
   }
   RpcEndExcept

   return true;
}

//
// Public Interface
//

//
// I_MidiRPCInitServer
//
// Start up the RPC MIDI server.
//
dboolean I_MidiRPCInitServer()
{
   char         module[MAX_PATH+1];

   M_snprintf(module, sizeof(module), "%s"DIR_SEPARATOR_S"midiproc.exe", M_GetExecutableFolder());

   // Look for executable file
   if(!M_FileExists(module))
      return false;

   si.cb = sizeof(si);

   return CreateProcess(module, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}

//
// I_MidiRPCInitClient
//
// Initialize client RPC bindings and bind to the server.
//
dboolean I_MidiRPCInitClient()
{
   // If server didn't start, client cannot be bound.
   if (!serverInit)
      return false;

   // Compose binding string
   if (RpcStringBindingCompose(NULL, (RPC_CSTR)"ncalrpc", NULL,
      (RPC_CSTR)"2d4dc2f9-ce90-4080-8a00-1cb819086970", NULL, &szStringBinding))
      return false;

   // Create binding handle
   if (RpcBindingFromStringBinding(szStringBinding, &hMidiRPCBinding))
      return false;

   clientInit = true;

   return I_MidiRPCWaitForServer();
}

//
// I_MidiRPCClientShutDown
//
// Shutdown the RPC Client
//
void I_MidiRPCClientShutDown()
{
   // stop the server
   if (serverInit)
   {
      RpcTryExcept
      {
         MidiRPC_StopServer();
      }
      RpcExcept(1)
      {
      }
      RpcEndExcept

      serverInit = false;
   }

   if (szStringBinding)
   {
      RpcStringFree(&szStringBinding);
      szStringBinding = NULL;
   }

   if (hMidiRPCBinding)
   {
      RpcBindingFree(&hMidiRPCBinding);
      hMidiRPCBinding = NULL;
   }

   clientInit = false;
}

//
// I_MidiRPCReady
//
// Returns true if both server and client initialized successfully.
//
dboolean I_MidiRPCReady()
{
   CHECK_RPC_STATUS();
   return true;
}

#endif
