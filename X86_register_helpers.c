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

#include <windows.h>

#include <assert.h>

#include "bolt_mingw_addons.h"
#include "X86_register.h"

void X86_register_get_value (X86_register r, const CONTEXT *ctx, s_Value* out)
{
#ifdef _WIN64
    XSAVE_FORMAT *t=(XSAVE_FORMAT*)&ctx->FltSave;
#else
    XSAVE_FORMAT *t=(XSAVE_FORMAT*)&ctx->ExtendedRegisters[0];
#endif

    switch (r)
    {
#ifdef _WIN64
    case R_RAX: create_Value (V_QWORD, (uint64_t)ctx->Rax, out); break;
    case R_RBX: create_Value (V_QWORD, (uint64_t)ctx->Rbx, out); break;
    case R_RCX: create_Value (V_QWORD, (uint64_t)ctx->Rcx, out); break;
    case R_RDX: create_Value (V_QWORD, (uint64_t)ctx->Rdx, out); break;
    case R_RSI: create_Value (V_QWORD, (uint64_t)ctx->Rsi, out); break;
    case R_RDI: create_Value (V_QWORD, (uint64_t)ctx->Rdi, out); break;
    case R_RSP: create_Value (V_QWORD, (uint64_t)ctx->Rsp, out); break;
    case R_RBP: create_Value (V_QWORD, (uint64_t)ctx->Rbp, out); break;
    case R_RIP: create_Value (V_QWORD, (uint64_t)ctx->Rip, out); break;

    case R_R8:  create_Value (V_QWORD, (uint64_t)ctx->R8, out); break;
    case R_R9:  create_Value (V_QWORD, (uint64_t)ctx->R9, out); break;
    case R_R10: create_Value (V_QWORD, (uint64_t)ctx->R10, out); break;
    case R_R11: create_Value (V_QWORD, (uint64_t)ctx->R11, out); break;
    case R_R12: create_Value (V_QWORD, (uint64_t)ctx->R12, out); break;
    case R_R13: create_Value (V_QWORD, (uint64_t)ctx->R13, out); break;
    case R_R14: create_Value (V_QWORD, (uint64_t)ctx->R14, out); break;
    case R_R15: create_Value (V_QWORD, (uint64_t)ctx->R15, out); break;

    case R_R8D:  create_Value (V_DWORD, (uint32_t)(ctx->R8 & 0xFFFFFFFF), out); break;
    case R_R9D:  create_Value (V_DWORD, (uint32_t)(ctx->R9 & 0xFFFFFFFF), out); break;
    case R_R10D: create_Value (V_DWORD, (uint32_t)(ctx->R10 & 0xFFFFFFFF), out); break;
    case R_R11D: create_Value (V_DWORD, (uint32_t)(ctx->R11 & 0xFFFFFFFF), out); break;
    case R_R12D: create_Value (V_DWORD, (uint32_t)(ctx->R12 & 0xFFFFFFFF), out); break;
    case R_R13D: create_Value (V_DWORD, (uint32_t)(ctx->R13 & 0xFFFFFFFF), out); break;
    case R_R14D: create_Value (V_DWORD, (uint32_t)(ctx->R14 & 0xFFFFFFFF), out); break;
    case R_R15D: create_Value (V_DWORD, (uint32_t)(ctx->R15 & 0xFFFFFFFF), out); break;

    case R_R8W:  create_Value(V_WORD, (uint16_t)(ctx->R8 & 0xFFFF), out); break;
    case R_R9W:  create_Value(V_WORD, (uint16_t)(ctx->R9 & 0xFFFF), out); break;
    case R_R10W: create_Value(V_WORD, (uint16_t)(ctx->R10 & 0xFFFF), out); break;
    case R_R11W: create_Value(V_WORD, (uint16_t)(ctx->R11 & 0xFFFF), out); break;
    case R_R12W: create_Value(V_WORD, (uint16_t)(ctx->R12 & 0xFFFF), out); break;
    case R_R13W: create_Value(V_WORD, (uint16_t)(ctx->R13 & 0xFFFF), out); break;
    case R_R14W: create_Value(V_WORD, (uint16_t)(ctx->R14 & 0xFFFF), out); break;
    case R_R15W: create_Value(V_WORD, (uint16_t)(ctx->R15 & 0xFFFF), out); break;

    case R_R8L:  create_Value(V_BYTE, (uint8_t)(ctx->R8 & 0xFF), out); break;
    case R_R9L:  create_Value(V_BYTE, (uint8_t)(ctx->R9 & 0xFF), out); break;
    case R_R10L: create_Value(V_BYTE, (uint8_t)(ctx->R10 & 0xFF), out); break;
    case R_R11L: create_Value(V_BYTE, (uint8_t)(ctx->R11 & 0xFF), out); break;
    case R_R12L: create_Value(V_BYTE, (uint8_t)(ctx->R12 & 0xFF), out); break;
    case R_R13L: create_Value(V_BYTE, (uint8_t)(ctx->R13 & 0xFF), out); break;
    case R_R14L: create_Value(V_BYTE, (uint8_t)(ctx->R14 & 0xFF), out); break;
    case R_R15L: create_Value(V_BYTE, (uint8_t)(ctx->R15 & 0xFF), out); break;

    case R_EAX: create_Value (V_DWORD, (uint32_t)(ctx->Rax & 0xFFFFFFFF), out); break;
    case R_EBX: create_Value (V_DWORD, (uint32_t)(ctx->Rbx & 0xFFFFFFFF), out); break;
    case R_ECX: create_Value (V_DWORD, (uint32_t)(ctx->Rcx & 0xFFFFFFFF), out); break;
    case R_EDX: create_Value (V_DWORD, (uint32_t)(ctx->Rdx & 0xFFFFFFFF), out); break;
    case R_ESI: create_Value (V_DWORD, (uint32_t)(ctx->Rsi & 0xFFFFFFFF), out); break;
    case R_EDI: create_Value (V_DWORD, (uint32_t)(ctx->Rdi & 0xFFFFFFFF), out); break;
    case R_EBP: create_Value (V_DWORD, (uint32_t)(ctx->Rbp & 0xFFFFFFFF), out); break;
    case R_ESP: create_Value (V_DWORD, (uint32_t)(ctx->Rsp & 0xFFFFFFFF), out); break;
    case R_EIP: create_Value (V_DWORD, (uint32_t)(ctx->Rip & 0xFFFFFFFF), out); break;

    case R_AL: create_Value(V_BYTE, (uint8_t)(ctx->Rax & 0xFF), out); break;
    case R_AH: create_Value(V_BYTE, (uint8_t)((ctx->Rax>>8) & 0xFF), out); break;
    case R_AX: create_Value(V_WORD, (uint16_t)(ctx->Rax & 0xFFFF), out); break;

    case R_BL: create_Value(V_BYTE, (uint8_t)(ctx->Rbx & 0xFF), out); break;
    case R_BH: create_Value(V_BYTE, (uint8_t)((ctx->Rbx>>8) & 0xFF), out); break;
    case R_BX: create_Value(V_WORD, (uint16_t)(ctx->Rbx & 0xFFFF), out); break;

    case R_CL: create_Value(V_BYTE, (uint8_t)(ctx->Rcx & 0xFF), out); break;
    case R_CH: create_Value(V_BYTE, (uint8_t)((ctx->Rcx>>8) & 0xFF), out); break;
    case R_CX: create_Value(V_WORD, (uint16_t)(ctx->Rcx & 0xFFFF), out); break;

    case R_DL: create_Value(V_BYTE, (uint8_t)(ctx->Rdx & 0xFF), out); break;
    case R_DH: create_Value(V_BYTE, (uint8_t)((ctx->Rdx>>8) & 0xFF), out); break;
    case R_DX: create_Value(V_WORD, (uint16_t)(ctx->Rdx & 0xFFFF), out); break;

    case R_SI: create_Value(V_WORD, (uint16_t)(ctx->Rsi & 0xFFFF), out); break;
    case R_DI: create_Value(V_WORD, (uint16_t)(ctx->Rdi & 0xFFFF), out); break;
    case R_BP: create_Value(V_WORD, (uint16_t)(ctx->Rbp & 0xFFFF), out); break;

    case R_SIL: create_Value(V_BYTE, (uint8_t)(ctx->Rsi & 0xFF), out); break;
    case R_DIL: create_Value(V_BYTE, (uint8_t)(ctx->Rdi & 0xFF), out); break;
    case R_BPL: create_Value(V_BYTE, (uint8_t)(ctx->Rbp & 0xFF), out); break;
#else
    case R_EAX: create_Value(V_DWORD, (uint32_t)ctx->Eax, out); break;
    case R_EBX: create_Value(V_DWORD, (uint32_t)ctx->Ebx, out); break;
    case R_ECX: create_Value(V_DWORD, (uint32_t)ctx->Ecx, out); break;
    case R_EDX: create_Value(V_DWORD, (uint32_t)ctx->Edx, out); break;
    case R_ESI: create_Value(V_DWORD, (uint32_t)ctx->Esi, out); break;
    case R_EDI: create_Value(V_DWORD, (uint32_t)ctx->Edi, out); break;
    case R_EBP: create_Value(V_DWORD, (uint32_t)ctx->Ebp, out); break;
    case R_ESP: create_Value(V_DWORD, (uint32_t)ctx->Esp, out); break;
    case R_EIP: create_Value(V_DWORD, (uint32_t)ctx->Eip, out); break;

    case R_AL: create_Value(V_BYTE, (uint8_t)(ctx->Eax&0xFF), out); break;
    case R_AH: create_Value(V_BYTE, (uint8_t)((ctx->Eax>>8)&0xFF), out); break;
    case R_AX: create_Value(V_WORD, (uint16_t)(ctx->Eax&0xFFFF), out); break;

    case R_BL: create_Value(V_BYTE, (uint8_t)(ctx->Ebx&0xFF), out); break;
    case R_BH: create_Value(V_BYTE, (uint8_t)((ctx->Ebx>>8)&0xFF), out); break;
    case R_BX: create_Value(V_WORD, (uint16_t)(ctx->Ebx&0xFFFF), out); break;

    case R_CL: create_Value(V_BYTE, (uint8_t)(ctx->Ecx&0xFF), out); break;
    case R_CH: create_Value(V_BYTE, (uint8_t)((ctx->Ecx>>8)&0xFF), out); break;
    case R_CX: create_Value(V_WORD, (uint16_t)(ctx->Ecx&0xFFFF), out); break;

    case R_DL: create_Value(V_BYTE, (uint8_t)(ctx->Edx&0xFF), out); break;
    case R_DH: create_Value(V_BYTE, (uint8_t)((ctx->Edx>>8)&0xFF), out); break;
    case R_DX: create_Value(V_WORD, (uint16_t)(ctx->Edx&0xFFFF), out); break;

    case R_SI: create_Value(V_WORD, (uint16_t)(ctx->Esi&0xFFFF), out); break;

    case R_DI: create_Value(V_WORD, (uint16_t)(ctx->Edi&0xFFFF), out); break;

    case R_BP: create_Value(V_WORD, (uint16_t)(ctx->Ebp&0xFFFF), out); break;
#endif

    case R_ES: create_Value(V_WORD, (uint16_t)ctx->SegEs, out); break;
    case R_DS: create_Value(V_WORD, (uint16_t)ctx->SegDs, out); break;
    case R_FS: create_Value(V_WORD, (uint16_t)ctx->SegFs, out); break;
    case R_GS: create_Value(V_WORD, (uint16_t)ctx->SegGs, out); break;
    case R_CS: create_Value(V_WORD, (uint16_t)ctx->SegCs, out); break;
    case R_SS: create_Value(V_WORD, (uint16_t)ctx->SegSs, out); break;
    
    case R_XMM0:
    case R_XMM1:
    case R_XMM2:
    case R_XMM3:
    case R_XMM4:
    case R_XMM5:
    case R_XMM6:
    case R_XMM7:
    case R_XMM8:
    case R_XMM9:
    case R_XMM10:
    case R_XMM11:
    case R_XMM12:
    case R_XMM13:
    case R_XMM14:
    case R_XMM15:
        switch (r)
        {
        case R_XMM0: create_XMM_Value ((uint8_t*)&t->XmmRegisters[0], out); break;
        case R_XMM1: create_XMM_Value ((uint8_t*)&t->XmmRegisters[1], out); break;
        case R_XMM2: create_XMM_Value ((uint8_t*)&t->XmmRegisters[2], out); break;
        case R_XMM3: create_XMM_Value ((uint8_t*)&t->XmmRegisters[3], out); break;
        case R_XMM4: create_XMM_Value ((uint8_t*)&t->XmmRegisters[4], out); break;
        case R_XMM5: create_XMM_Value ((uint8_t*)&t->XmmRegisters[5], out); break;
        case R_XMM6: create_XMM_Value ((uint8_t*)&t->XmmRegisters[6], out); break;
        case R_XMM7: create_XMM_Value ((uint8_t*)&t->XmmRegisters[7], out); break;
        case R_XMM8: create_XMM_Value ((uint8_t*)&t->XmmRegisters[8], out); break;
        case R_XMM9: create_XMM_Value ((uint8_t*)&t->XmmRegisters[9], out); break;
        case R_XMM10: create_XMM_Value ((uint8_t*)&t->XmmRegisters[10], out); break;
        case R_XMM11: create_XMM_Value ((uint8_t*)&t->XmmRegisters[11], out); break;
        case R_XMM12: create_XMM_Value ((uint8_t*)&t->XmmRegisters[12], out); break;
        case R_XMM13: create_XMM_Value ((uint8_t*)&t->XmmRegisters[13], out); break;
        case R_XMM14: create_XMM_Value ((uint8_t*)&t->XmmRegisters[14], out); break;
        case R_XMM15: create_XMM_Value ((uint8_t*)&t->XmmRegisters[15], out); break;
        default: assert(0);
        };
        break;
    
    case R_ST0: create_double_Value ((double)(*(long double*)&t->FloatRegisters[0]), out); break;
    case R_ST1: create_double_Value ((double)(*(long double*)&t->FloatRegisters[1]), out); break;
    case R_ST2: create_double_Value ((double)(*(long double*)&t->FloatRegisters[2]), out); break;
    case R_ST3: create_double_Value ((double)(*(long double*)&t->FloatRegisters[3]), out); break;
    case R_ST4: create_double_Value ((double)(*(long double*)&t->FloatRegisters[4]), out); break;
    case R_ST5: create_double_Value ((double)(*(long double*)&t->FloatRegisters[5]), out); break;
    case R_ST6: create_double_Value ((double)(*(long double*)&t->FloatRegisters[6]), out); break;
    case R_ST7: create_double_Value ((double)(*(long double*)&t->FloatRegisters[7]), out); break;

    case R_ABSENT:
        assert(0);
        break;

    default:
        assert (!"this register isn't implemented here yet");
        break;
    };
};

uint64_t X86_register_get_value_as_u64 (X86_register r, const CONTEXT *ctx)
{
    uint64_t rt;

    s_Value val;
    X86_register_get_value (r, ctx, &val);
    rt=val.u.v;
    return rt;
};

void X86_register_set_value (X86_register r, CONTEXT *ctx, s_Value *val)
{
#ifdef _WIN64
    XSAVE_FORMAT *t=(XSAVE_FORMAT*)&ctx->FltSave;
#else
    XSAVE_FORMAT *t=(XSAVE_FORMAT*)&ctx->ExtendedRegisters[0];
#endif

    switch (r)
    {
    case R_XMM0:
    case R_XMM1:
    case R_XMM2:
    case R_XMM3:
    case R_XMM4:
    case R_XMM5:
    case R_XMM6:
    case R_XMM7:
    case R_XMM8:
    case R_XMM9:
    case R_XMM10:
    case R_XMM11:
    case R_XMM12:
    case R_XMM13:
    case R_XMM14:
    case R_XMM15:

        switch (r)
        {
        case R_XMM0: memcpy (&t->XmmRegisters[0], get_xmm(val), 16); break;
        case R_XMM1: memcpy (&t->XmmRegisters[1], get_xmm(val), 16); break;
        case R_XMM2: memcpy (&t->XmmRegisters[2], get_xmm(val), 16); break;
        case R_XMM3: memcpy (&t->XmmRegisters[3], get_xmm(val), 16); break;
        case R_XMM4: memcpy (&t->XmmRegisters[4], get_xmm(val), 16); break;
        case R_XMM5: memcpy (&t->XmmRegisters[5], get_xmm(val), 16); break;
        case R_XMM6: memcpy (&t->XmmRegisters[6], get_xmm(val), 16); break;
        case R_XMM7: memcpy (&t->XmmRegisters[7], get_xmm(val), 16); break;
        case R_XMM8: memcpy (&t->XmmRegisters[8], get_xmm(val), 16); break;
        case R_XMM9: memcpy (&t->XmmRegisters[9], get_xmm(val), 16); break;
        case R_XMM10: memcpy (&t->XmmRegisters[10], get_xmm(val), 16); break;
        case R_XMM11: memcpy (&t->XmmRegisters[11], get_xmm(val), 16); break;
        case R_XMM12: memcpy (&t->XmmRegisters[12], get_xmm(val), 16); break;
        case R_XMM13: memcpy (&t->XmmRegisters[13], get_xmm(val), 16); break;
        case R_XMM14: memcpy (&t->XmmRegisters[14], get_xmm(val), 16); break;
        case R_XMM15: memcpy (&t->XmmRegisters[15], get_xmm(val), 16); break;
        default: assert(0);
        };

        break;

#ifdef _WIN64
    case R_R8D:  assert (val->t==V_DWORD); ctx->R8= (ctx->R8&0xFFFFFFFF00000000)  | (val->u.v&0xFFFFFFFF); break;
    case R_R9D:  assert (val->t==V_DWORD); ctx->R9= (ctx->R9&0xFFFFFFFF00000000)  | (val->u.v&0xFFFFFFFF); break;
    case R_R10D: assert (val->t==V_DWORD); ctx->R10=(ctx->R10&0xFFFFFFFF00000000) | (val->u.v&0xFFFFFFFF); break;
    case R_R11D: assert (val->t==V_DWORD); ctx->R11=(ctx->R11&0xFFFFFFFF00000000) | (val->u.v&0xFFFFFFFF); break;
    case R_R12D: assert (val->t==V_DWORD); ctx->R12=(ctx->R12&0xFFFFFFFF00000000) | (val->u.v&0xFFFFFFFF); break;
    case R_R13D: assert (val->t==V_DWORD); ctx->R13=(ctx->R13&0xFFFFFFFF00000000) | (val->u.v&0xFFFFFFFF); break;
    case R_R14D: assert (val->t==V_DWORD); ctx->R14=(ctx->R14&0xFFFFFFFF00000000) | (val->u.v&0xFFFFFFFF); break;
    case R_R15D: assert (val->t==V_DWORD); ctx->R15=(ctx->R15&0xFFFFFFFF00000000) | (val->u.v&0xFFFFFFFF); break;
        // TODO: a lot
#else
    case R_EAX: assert (val->t==V_DWORD); ctx->Eax=val->u.v&0xFFFFFFFF; break;
    case R_EBX: assert (val->t==V_DWORD); ctx->Ebx=val->u.v&0xFFFFFFFF; break;
    case R_ECX: assert (val->t==V_DWORD); ctx->Ecx=val->u.v&0xFFFFFFFF; break;
    case R_EDX: assert (val->t==V_DWORD); ctx->Edx=val->u.v&0xFFFFFFFF; break;
    case R_ESI: assert (val->t==V_DWORD); ctx->Esi=val->u.v&0xFFFFFFFF; break;
    case R_EDI: assert (val->t==V_DWORD); ctx->Edi=val->u.v&0xFFFFFFFF; break;
    case R_EBP: assert (val->t==V_DWORD); ctx->Ebp=val->u.v&0xFFFFFFFF; break;
    case R_ESP: assert (val->t==V_DWORD); ctx->Esp=val->u.v&0xFFFFFFFF; break;
    case R_EIP: assert (val->t==V_DWORD); ctx->Eip=val->u.v&0xFFFFFFFF; break;

    case R_AX: assert (val->t==V_WORD); ctx->Eax=(ctx->Eax&0xFFFF0000) | (val->u.v&0xFFFF); break;
    case R_BX: assert (val->t==V_WORD); ctx->Ebx=(ctx->Ebx&0xFFFF0000) | (val->u.v&0xFFFF); break;
    case R_CX: assert (val->t==V_WORD); ctx->Ecx=(ctx->Ecx&0xFFFF0000) | (val->u.v&0xFFFF); break;
    case R_DX: assert (val->t==V_WORD); ctx->Edx=(ctx->Edx&0xFFFF0000) | (val->u.v&0xFFFF); break;
    case R_SI: assert (val->t==V_WORD); ctx->Esi=(ctx->Esi&0xFFFF0000) | (val->u.v&0xFFFF); break;
    case R_DI: assert (val->t==V_WORD); ctx->Edi=(ctx->Edi&0xFFFF0000) | (val->u.v&0xFFFF); break;
    case R_BP: assert (val->t==V_WORD); ctx->Ebp=(ctx->Ebp&0xFFFF0000) | (val->u.v&0xFFFF); break;
    case R_SP: assert (val->t==V_WORD); ctx->Esp=(ctx->Esp&0xFFFF0000) | (val->u.v&0xFFFF); break;

    case R_AL: assert (val->t==V_BYTE); ctx->Eax=(ctx->Eax&0xFFFFFF00) | (val->u.v&0xFF); break;
    case R_BL: assert (val->t==V_BYTE); ctx->Ebx=(ctx->Ebx&0xFFFFFF00) | (val->u.v&0xFF); break;
    case R_CL: assert (val->t==V_BYTE); ctx->Ecx=(ctx->Ecx&0xFFFFFF00) | (val->u.v&0xFF); break;
    case R_DL: assert (val->t==V_BYTE); ctx->Edx=(ctx->Edx&0xFFFFFF00) | (val->u.v&0xFF); break;

    case R_AH: assert (val->t==V_BYTE); ctx->Eax=(ctx->Eax&0xFFFF00FF) | ((val->u.v&0xFF)<<8); break;
    case R_BH: assert (val->t==V_BYTE); ctx->Ebx=(ctx->Ebx&0xFFFF00FF) | ((val->u.v&0xFF)<<8); break;
    case R_CH: assert (val->t==V_BYTE); ctx->Ecx=(ctx->Ecx&0xFFFF00FF) | ((val->u.v&0xFF)<<8); break;
    case R_DH: assert (val->t==V_BYTE); ctx->Edx=(ctx->Edx&0xFFFF00FF) | ((val->u.v&0xFF)<<8); break;
#endif
    default:
        printf ("%s. r=%s\n", __FUNCTION__, X86_register_ToString (r));
        assert (0); break;
    };
};

/* vim: set expandtab ts=4 sw=4 : */
