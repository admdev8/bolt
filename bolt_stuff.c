#include <assert.h>
#include "stuff.h"
#include "bolt_stuff.h"

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

    if (!NT_SUCCESS((*NtQueryInformationThread_ptr) (THDL, ThreadBasicInformation, &ThreadInfo, sizeof( THREAD_BASIC_INFORMATION ), 0 )))
        die ("NtQueryInformationThread() failed\n");

    return ThreadInfo.TebBaseAddress;
};

address TIB_get_stack_top (HANDLE THDL, MemoryCache *mem)
{
    PNT_TIB TEB;
    NT_TIB TIB;

    TEB=get_TIB (THDL);
    assert (TEB);

    bool b=MC_ReadBuffer(mem, (REG)TEB, sizeof (TIB), (BYTE*)&TIB);
    assert (b);

    return (address)TIB.StackBase;
};

address TIB_get_stack_bottom (HANDLE THDL, MemoryCache *mem)
{
    PNT_TIB TEB;
    NT_TIB TIB;

    TEB=get_TIB (THDL);
    assert (TEB);

    bool b=MC_ReadBuffer(mem, (REG)TEB, sizeof (TIB), (BYTE*)&TIB);
    assert (b);

    return (address)TIB.StackLimit;
};

address TIB_get_exceptionlist (HANDLE THDL, MemoryCache *mem)
{
    PNT_TIB TEB;
    NT_TIB TIB;

    TEB=get_TIB (THDL);
    assert (TEB);

    bool b=MC_ReadBuffer(mem, (REG)TEB, sizeof (TIB), (BYTE*)&TIB);
    assert (b);

    return (address)TIB.ExceptionList;
};

bool TIB_is_ptr_in_stack_limits (HANDLE THDL, address p, MemoryCache *mem)
{
    return p<=TIB_get_stack_top (THDL, mem) && p>=TIB_get_stack_bottom (THDL, mem);
};

#if 0
// not sure if it's works
DWORD TIB_get_TLS_ptr (HANDLE THDL, MemoryCache *mem)
{
    BYTE* TEB;

    TEB=(BYTE*)get_TIB ();
    assert (TEB!=NULL);

    DWORD rt;
    bool b=MC_ReadTetrabyte (mem, (REG)(TEB+0x2C), &rt);
    assert (b);
    return rt;
};
#endif
