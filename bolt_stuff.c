/*
 *  _           _ _   
 * | |         | | |  
 * | |__   ___ | | |_ 
 * | '_ \ / _ \| | __|
 * | |_) | (_) | | |_ 
 * |_.__/ \___/|_|\__|
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#include "bolt_addons.h"
#include "oassert.h"
#include "stuff.h"
#include "bolt_stuff.h"
#include "porg_utils.h"

// NtQueryInformationThread retrieves information about a thread object.
/*
NTSYSAPI
    NTSTATUS
    NTAPI
    NtQueryInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
*/

PNT_TIB get_TIB (HANDLE THDL)
{
    THREAD_BASIC_INFORMATION ThreadInfo;
    NTSTATUS (NTAPI * NtQueryInformationThread_ptr)(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, OUT PVOID ThreadInformation, IN ULONG ThreadInformationLength, OUT PULONG ReturnLength OPTIONAL);

    NtQueryInformationThread_ptr=(NTSTATUS (__stdcall *)(HANDLE,THREADINFOCLASS,PVOID,ULONG,PULONG))(GetProcAddress (LoadLibrary ("ntdll.dll"), "NtQueryInformationThread"));

    if (NtQueryInformationThread_ptr==NULL)
        die ("ntdll.dll!NtQueryInformationThread() was not found\n");

    // FIXME: untested
    //if (!NT_SUCCESS((*NtQueryInformationThread_ptr) (THDL, ThreadBasicInformation, &ThreadInfo, sizeof( THREAD_BASIC_INFORMATION ), 0 )))
    if ((*NtQueryInformationThread_ptr) (THDL, ThreadBasicInformation, &ThreadInfo, sizeof( THREAD_BASIC_INFORMATION ), 0 )<0)
        die ("NtQueryInformationThread() failed\n");

    return ThreadInfo.TebBaseAddress;
};

address TIB_get_stack_top (HANDLE THDL, struct MemoryCache *mem)
{
    PNT_TIB TEB;
    NT_TIB TIB;

    TEB=get_TIB (THDL);
    oassert (TEB);

    bool b=MC_ReadBuffer(mem, (REG)TEB, sizeof (TIB), (BYTE*)&TIB);
    oassert (b);

    return (address)TIB.StackBase;
};

address TIB_get_stack_bottom (HANDLE THDL, struct MemoryCache *mem)
{
    PNT_TIB TEB;
    NT_TIB TIB;

    TEB=get_TIB (THDL);
    oassert (TEB);

    bool b=MC_ReadBuffer(mem, (REG)TEB, sizeof (TIB), (BYTE*)&TIB);
    oassert (b);

    return (address)TIB.StackLimit;
};

address TIB_get_exceptionlist (HANDLE THDL, struct MemoryCache *mem)
{
    PNT_TIB TEB;
    NT_TIB TIB;

    TEB=get_TIB (THDL);
    oassert (TEB);

    bool b=MC_ReadBuffer(mem, (REG)TEB, sizeof (TIB), (BYTE*)&TIB);
    oassert (b);

    return (address)TIB.ExceptionList;
};

bool TIB_is_ptr_in_stack_limits (HANDLE THDL, address p, struct MemoryCache *mem)
{
    return p<=TIB_get_stack_top (THDL, mem) && p>=TIB_get_stack_bottom (THDL, mem);
};

address TIB_get_current_SEH_frame (struct MemoryCache *mc, HANDLE THDL)
{
    PNT_TIB TEB;
    NT_TIB TIB;

    TEB=get_TIB (THDL);
    oassert (TEB);

    bool b=MC_ReadBuffer(mc, (REG)TEB, sizeof (TIB), (BYTE*)&TIB);
    oassert (b);

    return (address)TIB.ExceptionList;
};

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    /*PPEB_LDR_DATA*/ void* Ldr;
    /*PRTL_USER_PROCESS_PARAMETERS*/ void* ProcessParameters;
    BYTE Reserved4[104];
    PVOID Reserved5[52];
    /*PPS_POST_PROCESS_INIT_ROUTINE*/ void* PostProcessInitRoutine;
    BYTE Reserved6[128];
    PVOID Reserved7[1];
    ULONG SessionId;
} PEB, *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0,
    ProcessWow64Information = 26
} PROCESSINFOCLASS;

address get_PEB(HANDLE PHDL)
{
	ULONG dwLength;
	PROCESS_BASIC_INFORMATION pbi;

	NTSTATUS (NTAPI * NtQueryInformationProcess_ptr)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation,
		ULONG ProcessInformationLength, PULONG ReturnLength);

	NtQueryInformationProcess_ptr=(NTSTATUS (__stdcall *)(HANDLE,PROCESSINFOCLASS,PVOID,ULONG,PULONG))(GetProcAddress (LoadLibrary ("ntdll.dll"), "NtQueryInformationProcess"));

	if (NtQueryInformationProcess_ptr==NULL)
        	die ("ntdll.dll!NtQueryInformationProcess() was not found\n");

	DWORD status=NtQueryInformationProcess_ptr(PHDL, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), &dwLength);

	if(status!=0x0)
		die ("NtQueryInformationProcess() failed\n");

	return pbi.PebBaseAddress;
};
