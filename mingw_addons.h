#pragma once

#include <windows.h>

// absent in MinGW
#ifndef _MSC_VER

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

typedef struct _M128A {
    ULONGLONG Low;
    LONGLONG High;
} M128A, *PM128A;

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
#endif // _MSC_VER

} XSAVE_FORMAT, *PXSAVE_FORMAT;
