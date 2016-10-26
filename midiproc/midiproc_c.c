

/* this ALWAYS GENERATED file contains the RPC client stubs */


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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#include <string.h>

#include "midiproc.h"

#define TYPE_FORMAT_STRING_SIZE   19                                
#define PROC_FORMAT_STRING_SIZE   217                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   0            

typedef struct _midiproc_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } midiproc_MIDL_TYPE_FORMAT_STRING;

typedef struct _midiproc_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } midiproc_MIDL_PROC_FORMAT_STRING;

typedef struct _midiproc_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } midiproc_MIDL_EXPR_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const midiproc_MIDL_TYPE_FORMAT_STRING midiproc__MIDL_TypeFormatString;
extern const midiproc_MIDL_PROC_FORMAT_STRING midiproc__MIDL_ProcFormatString;
extern const midiproc_MIDL_EXPR_FORMAT_STRING midiproc__MIDL_ExprFormatString;

#define GENERIC_BINDING_TABLE_SIZE   0            


/* Standard interface: MidiRPC, ver. 1.0,
   GUID={0x2d4dc2f9,0xce90,0x4080,{0x8a,0x00,0x1c,0xb8,0x19,0x08,0x69,0x70}} */

handle_t hMidiRPCBinding;


static const RPC_CLIENT_INTERFACE MidiRPC___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x2d4dc2f9,0xce90,0x4080,{0x8a,0x00,0x1c,0xb8,0x19,0x08,0x69,0x70}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE MidiRPC_v1_0_c_ifspec = (RPC_IF_HANDLE)& MidiRPC___RpcClientInterface;

extern const MIDL_STUB_DESC MidiRPC_StubDesc;

static RPC_BINDING_HANDLE MidiRPC__MIDL_AutoBindHandle;


void MidiRPC_PrepareNewSong( void)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&MidiRPC_StubDesc,
                  (PFORMAT_STRING) &midiproc__MIDL_ProcFormatString.Format[0],
                  ( unsigned char * )0);
    
}


void MidiRPC_AddChunk( 
    /* [in] */ unsigned int count,
    /* [size_is][in] */ byte *pBuf)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&MidiRPC_StubDesc,
                  (PFORMAT_STRING) &midiproc__MIDL_ProcFormatString.Format[24],
                  ( unsigned char * )&count);
    
}


void MidiRPC_PlaySong( 
    /* [in] */ boolean looping)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&MidiRPC_StubDesc,
                  (PFORMAT_STRING) &midiproc__MIDL_ProcFormatString.Format[60],
                  ( unsigned char * )&looping);
    
}


void MidiRPC_StopSong( void)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&MidiRPC_StubDesc,
                  (PFORMAT_STRING) &midiproc__MIDL_ProcFormatString.Format[90],
                  ( unsigned char * )0);
    
}


void MidiRPC_ChangeVolume( 
    /* [in] */ int volume)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&MidiRPC_StubDesc,
                  (PFORMAT_STRING) &midiproc__MIDL_ProcFormatString.Format[114],
                  ( unsigned char * )&volume);
    
}


void MidiRPC_PauseSong( void)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&MidiRPC_StubDesc,
                  (PFORMAT_STRING) &midiproc__MIDL_ProcFormatString.Format[144],
                  ( unsigned char * )0);
    
}


void MidiRPC_ResumeSong( void)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&MidiRPC_StubDesc,
                  (PFORMAT_STRING) &midiproc__MIDL_ProcFormatString.Format[168],
                  ( unsigned char * )0);
    
}


void MidiRPC_StopServer( void)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&MidiRPC_StubDesc,
                  (PFORMAT_STRING) &midiproc__MIDL_ProcFormatString.Format[192],
                  ( unsigned char * )0);
    
}


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const midiproc_MIDL_PROC_FORMAT_STRING midiproc__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure MidiRPC_PrepareNewSong */

			0x32,		/* FC_BIND_PRIMITIVE */
			0x48,		/* Old Flags:  */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x0 ),	/* 0 */
/*  8 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* 0 */
/* 14 */	0x40,		/* Oi2 Flags:  has ext, */
			0x0,		/* 0 */
/* 16 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 18 */	NdrFcShort( 0x0 ),	/* 0 */
/* 20 */	NdrFcShort( 0x0 ),	/* 0 */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Procedure MidiRPC_AddChunk */

/* 24 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x48,		/* Old Flags:  */
/* 26 */	NdrFcLong( 0x0 ),	/* 0 */
/* 30 */	NdrFcShort( 0x1 ),	/* 1 */
/* 32 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 34 */	NdrFcShort( 0x8 ),	/* 8 */
/* 36 */	NdrFcShort( 0x0 ),	/* 0 */
/* 38 */	0x42,		/* Oi2 Flags:  clt must size, has ext, */
			0x2,		/* 2 */
/* 40 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 42 */	NdrFcShort( 0x0 ),	/* 0 */
/* 44 */	NdrFcShort( 0x1 ),	/* 1 */
/* 46 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter count */

/* 48 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 50 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 52 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter pBuf */

/* 54 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 56 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 58 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */

	/* Procedure MidiRPC_PlaySong */

/* 60 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x48,		/* Old Flags:  */
/* 62 */	NdrFcLong( 0x0 ),	/* 0 */
/* 66 */	NdrFcShort( 0x2 ),	/* 2 */
/* 68 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 70 */	NdrFcShort( 0x5 ),	/* 5 */
/* 72 */	NdrFcShort( 0x0 ),	/* 0 */
/* 74 */	0x40,		/* Oi2 Flags:  has ext, */
			0x1,		/* 1 */
/* 76 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */
/* 80 */	NdrFcShort( 0x0 ),	/* 0 */
/* 82 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter looping */

/* 84 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 86 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 88 */	0x3,		/* FC_SMALL */
			0x0,		/* 0 */

	/* Procedure MidiRPC_StopSong */

/* 90 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x48,		/* Old Flags:  */
/* 92 */	NdrFcLong( 0x0 ),	/* 0 */
/* 96 */	NdrFcShort( 0x3 ),	/* 3 */
/* 98 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 100 */	NdrFcShort( 0x0 ),	/* 0 */
/* 102 */	NdrFcShort( 0x0 ),	/* 0 */
/* 104 */	0x40,		/* Oi2 Flags:  has ext, */
			0x0,		/* 0 */
/* 106 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 108 */	NdrFcShort( 0x0 ),	/* 0 */
/* 110 */	NdrFcShort( 0x0 ),	/* 0 */
/* 112 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Procedure MidiRPC_ChangeVolume */

/* 114 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x48,		/* Old Flags:  */
/* 116 */	NdrFcLong( 0x0 ),	/* 0 */
/* 120 */	NdrFcShort( 0x4 ),	/* 4 */
/* 122 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 124 */	NdrFcShort( 0x8 ),	/* 8 */
/* 126 */	NdrFcShort( 0x0 ),	/* 0 */
/* 128 */	0x40,		/* Oi2 Flags:  has ext, */
			0x1,		/* 1 */
/* 130 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 132 */	NdrFcShort( 0x0 ),	/* 0 */
/* 134 */	NdrFcShort( 0x0 ),	/* 0 */
/* 136 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter volume */

/* 138 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 140 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 142 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure MidiRPC_PauseSong */

/* 144 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x48,		/* Old Flags:  */
/* 146 */	NdrFcLong( 0x0 ),	/* 0 */
/* 150 */	NdrFcShort( 0x5 ),	/* 5 */
/* 152 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 154 */	NdrFcShort( 0x0 ),	/* 0 */
/* 156 */	NdrFcShort( 0x0 ),	/* 0 */
/* 158 */	0x40,		/* Oi2 Flags:  has ext, */
			0x0,		/* 0 */
/* 160 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 162 */	NdrFcShort( 0x0 ),	/* 0 */
/* 164 */	NdrFcShort( 0x0 ),	/* 0 */
/* 166 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Procedure MidiRPC_ResumeSong */

/* 168 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x48,		/* Old Flags:  */
/* 170 */	NdrFcLong( 0x0 ),	/* 0 */
/* 174 */	NdrFcShort( 0x6 ),	/* 6 */
/* 176 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 178 */	NdrFcShort( 0x0 ),	/* 0 */
/* 180 */	NdrFcShort( 0x0 ),	/* 0 */
/* 182 */	0x40,		/* Oi2 Flags:  has ext, */
			0x0,		/* 0 */
/* 184 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 186 */	NdrFcShort( 0x0 ),	/* 0 */
/* 188 */	NdrFcShort( 0x0 ),	/* 0 */
/* 190 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Procedure MidiRPC_StopServer */

/* 192 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x48,		/* Old Flags:  */
/* 194 */	NdrFcLong( 0x0 ),	/* 0 */
/* 198 */	NdrFcShort( 0x7 ),	/* 7 */
/* 200 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 202 */	NdrFcShort( 0x0 ),	/* 0 */
/* 204 */	NdrFcShort( 0x0 ),	/* 0 */
/* 206 */	0x40,		/* Oi2 Flags:  has ext, */
			0x0,		/* 0 */
/* 208 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 210 */	NdrFcShort( 0x0 ),	/* 0 */
/* 212 */	NdrFcShort( 0x0 ),	/* 0 */
/* 214 */	NdrFcShort( 0x0 ),	/* 0 */

			0x0
        }
    };

static const midiproc_MIDL_TYPE_FORMAT_STRING midiproc__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x0,	/* FC_RP */
/*  4 */	NdrFcShort( 0x2 ),	/* Offset= 2 (6) */
/*  6 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/*  8 */	NdrFcShort( 0x1 ),	/* 1 */
/* 10 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 16 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */

			0x0
        }
    };

static const unsigned short MidiRPC_FormatStringOffsetTable[] =
    {
    0,
    24,
    60,
    90,
    114,
    144,
    168,
    192
    };


static const MIDL_STUB_DESC MidiRPC_StubDesc = 
    {
    (void *)& MidiRPC___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &hMidiRPCBinding,
    0,
    0,
    0,
    0,
    midiproc__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x70001f4, /* MIDL Version 7.0.500 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

