#pragma once

#include <windows.h>
#include <ntdef.h>

// absent in MinGW
#ifndef _MSC_VER

typedef ULONG_PTR KAFFINITY, *PKAFFINITY;

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

#ifndef _WIN64
typedef struct _M128A {
    ULONGLONG Low;
    LONGLONG High;
} M128A, *PM128A;
#endif

typedef struct _XSAVE_FORMAT {
    WORD   ControlWord;
    WORD   StatusWord;
    BYTE  TagWord;
    BYTE  Reserved1;
    WORD   ErrorOpcode;
    DWORD ErrorOffset;
    WORD   ErrorSelector;
    WORD   Reserved2;
    DWORD DataOffset;
    WORD   DataSelector;
    WORD   Reserved3;
    DWORD MxCsr;
    DWORD MxCsr_Mask;
    M128A FloatRegisters[8];

#if defined(_WIN64)

    M128A XmmRegisters[16];
    BYTE  Reserved4[96];

#else

    M128A XmmRegisters[8];
    BYTE  Reserved4[192];

    //
    // The fields below are not part of XSAVE/XRSTOR format.
    // They are written by the OS which is relying on a fact that
    // neither (FX)SAVE nor (F)XSTOR used this area.
    //

    DWORD   StackControl[7];    // KERNEL_STACK_CONTROL structure actualy
    DWORD   Cr0NpxState;

#endif
} XSAVE_FORMAT, *PXSAVE_FORMAT;

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID,*PCLIENT_ID;

typedef LONG KPRIORITY;

typedef struct _THREAD_BASIC_INFORMATION { // Information Class 0
    NTSTATUS ExitStatus;
    PNT_TIB TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

typedef enum _THREADINFOCLASS {        //        Query    Set        Notes
    ThreadBasicInformation,            // 0     Y        N
    ThreadTimes,                       // 1     Y        N
    ThreadPriority,                    // 2     N        Y        KAFFINITY
    ThreadBasePriority,                // 3     N        Y        LONG
    ThreadAffinityMask,                // 4     N        Y        KAFFINITY
    ThreadImpersonationToken,          // 5     N        Y        HANDLE
    ThreadDescriptorTableEntry,        // 6     Y        N
    ThreadEnableAlignmentFaultFixup,   // 7     N        Y        BOOLEAN
    ThreadEventPair,                   // 8     N        Y        HANDLE, removed in Windows 2000 and later (invalid infoclass)
    ThreadQuerySetWin32StartAddress,   // 9     Y        Y        PVOID, x86: Initially eax, also LpcReceivedMessageId
    ThreadZeroTlsCell,                 // 10    N        Y        ULONG, Zeroes TLS cell at specified index
    ThreadPerformanceCount,            // 11    Y        N        LARGE_INTEGER, Always 0
    ThreadAmILastThread,               // 12    Y        N        ULONG
    ThreadIdealProcessor,              // 13    N        Y        ULONG, MAXIMUM_PROCESSORS sets none
    ThreadPriorityBoost,               // 14    Y        Y        ULONG, boolean: enabled
    ThreadSetTlsArrayAddress,          // 15    N        Y        PVOID, address of TLS array
    ThreadIsIoPending,                 // 16    Y        N        ULONG, boolean: outstanding IRPs
    ThreadHideFromDebugger,            // 17    N        Y        Use NULL pointer and zero disables debug events
    ThreadBreakOnTermination,          // 18    Y        Y        Terminate system on exit, ULONG/BOOLEAN interpretation, SeTcbPrivilege,
    ThreadSwitchLegacyState,           // 19    N        ?        x64 only, Thread->Tcb.NpxState set to LEGACY_STATE_SWITCH, no data supplied
    ThreadIsTerminated,                // 20    Y        N        ULONG, Terminated ? TRUE : FALSE
    MaxThreadInfoClass
} THREADINFOCLASS;

#define _NTNATIVE_SELECT_NT(exp) exp

#if 0
#define NT_SUCCESS(Status) ((_NTNATIVE_SELECT_NT(NTSTATUS))(Status) >= 0)

#if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define NTAPI __stdcall
#else
#define _cdecl
#define NTAPI
#endif

#if !defined(_NTSYSTEM_)
#define NTSYSAPI DECLSPEC_IMPORT
#else
#define NTSYSAPI DECLSPEC_EXPORT
#endif
#endif

#endif // _MSC_VER
