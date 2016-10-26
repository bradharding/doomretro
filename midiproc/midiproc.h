

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Fri Nov 23 13:34:56 2012
 */
/* Compiler settings for midiproc.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, app_config, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __midiproc_h__
#define __midiproc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __MidiRPC_INTERFACE_DEFINED__
#define __MidiRPC_INTERFACE_DEFINED__

/* interface MidiRPC */
/* [implicit_handle][implicit_handle][version][uuid] */ 

void MidiRPC_PrepareNewSong( void);

void MidiRPC_AddChunk( 
    /* [in] */ unsigned int count,
    /* [size_is][in] */ byte *pBuf);

void MidiRPC_PlaySong( 
    /* [in] */ boolean looping);

void MidiRPC_StopSong( void);

void MidiRPC_ChangeVolume( 
    /* [in] */ int volume);

void MidiRPC_PauseSong( void);

void MidiRPC_ResumeSong( void);

void MidiRPC_StopServer( void);


extern handle_t hMidiRPCBinding;


extern RPC_IF_HANDLE MidiRPC_v1_0_c_ifspec;
extern RPC_IF_HANDLE MidiRPC_v1_0_s_ifspec;
#endif /* __MidiRPC_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


