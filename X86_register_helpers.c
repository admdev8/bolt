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

#include <assert.h>
#include <stdio.h>
#include "X86_register_helpers.h"
#include "bolt_mingw_addons.h"

void X86_register_get_value (X86_register r, const CONTEXT *ctx, obj* out)
{
#ifdef _WIN64
    XSAVE_FORMAT *t=(XSAVE_FORMAT*)&ctx->FltSave;
#else
    XSAVE_FORMAT *t=(XSAVE_FORMAT*)&ctx->ExtendedRegisters[0];
#endif

    switch (r)
    {
#ifdef _WIN64
    case R_RAX: obj_octabyte2 ((uint64_t)ctx->Rax, out); break;
    case R_RBX: obj_octabyte2 ((uint64_t)ctx->Rbx, out); break;
    case R_RCX: obj_octabyte2 ((uint64_t)ctx->Rcx, out); break;
    case R_RDX: obj_octabyte2 ((uint64_t)ctx->Rdx, out); break;
    case R_RSI: obj_octabyte2 ((uint64_t)ctx->Rsi, out); break;
    case R_RDI: obj_octabyte2 ((uint64_t)ctx->Rdi, out); break;
    case R_RSP: obj_octabyte2 ((uint64_t)ctx->Rsp, out); break;
    case R_RBP: obj_octabyte2 ((uint64_t)ctx->Rbp, out); break;
    case R_RIP: obj_octabyte2 ((uint64_t)ctx->Rip, out); break;

    case R_R8:  obj_octabyte2 ((uint64_t)ctx->R8, out); break;
    case R_R9:  obj_octabyte2 ((uint64_t)ctx->R9, out); break;
    case R_R10: obj_octabyte2 ((uint64_t)ctx->R10, out); break;
    case R_R11: obj_octabyte2 ((uint64_t)ctx->R11, out); break;
    case R_R12: obj_octabyte2 ((uint64_t)ctx->R12, out); break;
    case R_R13: obj_octabyte2 ((uint64_t)ctx->R13, out); break;
    case R_R14: obj_octabyte2 ((uint64_t)ctx->R14, out); break;
    case R_R15: obj_octabyte2 ((uint64_t)ctx->R15, out); break;

    case R_R8D:  obj_tetrabyte2 ((uint32_t)(ctx->R8 & 0xFFFFFFFF), out); break;
    case R_R9D:  obj_tetrabyte2 ((uint32_t)(ctx->R9 & 0xFFFFFFFF), out); break;
    case R_R10D: obj_tetrabyte2 ((uint32_t)(ctx->R10 & 0xFFFFFFFF), out); break;
    case R_R11D: obj_tetrabyte2 ((uint32_t)(ctx->R11 & 0xFFFFFFFF), out); break;
    case R_R12D: obj_tetrabyte2 ((uint32_t)(ctx->R12 & 0xFFFFFFFF), out); break;
    case R_R13D: obj_tetrabyte2 ((uint32_t)(ctx->R13 & 0xFFFFFFFF), out); break;
    case R_R14D: obj_tetrabyte2 ((uint32_t)(ctx->R14 & 0xFFFFFFFF), out); break;
    case R_R15D: obj_tetrabyte2 ((uint32_t)(ctx->R15 & 0xFFFFFFFF), out); break;

    case R_R8W:  obj_wyde2 ((uint16_t)(ctx->R8 & 0xFFFF), out); break;
    case R_R9W:  obj_wyde2 ((uint16_t)(ctx->R9 & 0xFFFF), out); break;
    case R_R10W: obj_wyde2 ((uint16_t)(ctx->R10 & 0xFFFF), out); break;
    case R_R11W: obj_wyde2 ((uint16_t)(ctx->R11 & 0xFFFF), out); break;
    case R_R12W: obj_wyde2 ((uint16_t)(ctx->R12 & 0xFFFF), out); break;
    case R_R13W: obj_wyde2 ((uint16_t)(ctx->R13 & 0xFFFF), out); break;
    case R_R14W: obj_wyde2 ((uint16_t)(ctx->R14 & 0xFFFF), out); break;
    case R_R15W: obj_wyde2 ((uint16_t)(ctx->R15 & 0xFFFF), out); break;

    case R_R8L:  obj_byte2 ((uint8_t)(ctx->R8 & 0xFF), out); break;
    case R_R9L:  obj_byte2 ((uint8_t)(ctx->R9 & 0xFF), out); break;
    case R_R10L: obj_byte2 ((uint8_t)(ctx->R10 & 0xFF), out); break;
    case R_R11L: obj_byte2 ((uint8_t)(ctx->R11 & 0xFF), out); break;
    case R_R12L: obj_byte2 ((uint8_t)(ctx->R12 & 0xFF), out); break;
    case R_R13L: obj_byte2 ((uint8_t)(ctx->R13 & 0xFF), out); break;
    case R_R14L: obj_byte2 ((uint8_t)(ctx->R14 & 0xFF), out); break;
    case R_R15L: obj_byte2 ((uint8_t)(ctx->R15 & 0xFF), out); break;

    case R_EAX: obj_tetrabyte2 ((uint32_t)(ctx->Rax & 0xFFFFFFFF), out); break;
    case R_EBX: obj_tetrabyte2 ((uint32_t)(ctx->Rbx & 0xFFFFFFFF), out); break;
    case R_ECX: obj_tetrabyte2 ((uint32_t)(ctx->Rcx & 0xFFFFFFFF), out); break;
    case R_EDX: obj_tetrabyte2 ((uint32_t)(ctx->Rdx & 0xFFFFFFFF), out); break;
    case R_ESI: obj_tetrabyte2 ((uint32_t)(ctx->Rsi & 0xFFFFFFFF), out); break;
    case R_EDI: obj_tetrabyte2 ((uint32_t)(ctx->Rdi & 0xFFFFFFFF), out); break;
    case R_EBP: obj_tetrabyte2 ((uint32_t)(ctx->Rbp & 0xFFFFFFFF), out); break;
    case R_ESP: obj_tetrabyte2 ((uint32_t)(ctx->Rsp & 0xFFFFFFFF), out); break;
    case R_EIP: obj_tetrabyte2 ((uint32_t)(ctx->Rip & 0xFFFFFFFF), out); break;

    case R_AL: obj_byte2 ((uint8_t)(ctx->Rax & 0xFF), out); break;
    case R_AH: obj_byte2 ((uint8_t)((ctx->Rax>>8) & 0xFF), out); break;
    case R_AX: obj_wyde2 ((uint16_t)(ctx->Rax & 0xFFFF), out); break;

    case R_BL: obj_byte2 ((uint8_t)(ctx->Rbx & 0xFF), out); break;
    case R_BH: obj_byte2 ((uint8_t)((ctx->Rbx>>8) & 0xFF), out); break;
    case R_BX: obj_wyde2 ((uint16_t)(ctx->Rbx & 0xFFFF), out); break;

    case R_CL: obj_byte2 ((uint8_t)(ctx->Rcx & 0xFF), out); break;
    case R_CH: obj_byte2 ((uint8_t)((ctx->Rcx>>8) & 0xFF), out); break;
    case R_CX: obj_wyde2 ((uint16_t)(ctx->Rcx & 0xFFFF), out); break;

    case R_DL: obj_byte2 ((uint8_t)(ctx->Rdx & 0xFF), out); break;
    case R_DH: obj_byte2 ((uint8_t)((ctx->Rdx>>8) & 0xFF), out); break;
    case R_DX: obj_wyde2 ((uint16_t)(ctx->Rdx & 0xFFFF), out); break;

    case R_SI: obj_wyde2 ((uint16_t)(ctx->Rsi & 0xFFFF), out); break;
    case R_DI: obj_wyde2 ((uint16_t)(ctx->Rdi & 0xFFFF), out); break;
    case R_BP: obj_wyde2 ((uint16_t)(ctx->Rbp & 0xFFFF), out); break;

    case R_SIL: obj_byte2 ((uint8_t)(ctx->Rsi & 0xFF), out); break;
    case R_DIL: obj_byte2 ((uint8_t)(ctx->Rdi & 0xFF), out); break;
    case R_BPL: obj_byte2 ((uint8_t)(ctx->Rbp & 0xFF), out); break;
#else
    case R_EAX: obj_tetrabyte2 ((uint32_t)ctx->Eax, out); break;
    case R_EBX: obj_tetrabyte2 ((uint32_t)ctx->Ebx, out); break;
    case R_ECX: obj_tetrabyte2 ((uint32_t)ctx->Ecx, out); break;
    case R_EDX: obj_tetrabyte2 ((uint32_t)ctx->Edx, out); break;
    case R_ESI: obj_tetrabyte2 ((uint32_t)ctx->Esi, out); break;
    case R_EDI: obj_tetrabyte2 ((uint32_t)ctx->Edi, out); break;
    case R_EBP: obj_tetrabyte2 ((uint32_t)ctx->Ebp, out); break;
    case R_ESP: obj_tetrabyte2 ((uint32_t)ctx->Esp, out); break;
    case R_EIP: obj_tetrabyte2 ((uint32_t)ctx->Eip, out); break;

    case R_AL: obj_byte2 ((uint8_t)(ctx->Eax&0xFF), out); break;
    case R_AH: obj_byte2 ((uint8_t)((ctx->Eax>>8)&0xFF), out); break;
    case R_AX: obj_wyde2 ((uint16_t)(ctx->Eax&0xFFFF), out); break;

    case R_BL: obj_byte2 ((uint8_t)(ctx->Ebx&0xFF), out); break;
    case R_BH: obj_byte2 ((uint8_t)((ctx->Ebx>>8)&0xFF), out); break;
    case R_BX: obj_wyde2 ((uint16_t)(ctx->Ebx&0xFFFF), out); break;

    case R_CL: obj_byte2 ((uint8_t)(ctx->Ecx&0xFF), out); break;
    case R_CH: obj_byte2 ((uint8_t)((ctx->Ecx>>8)&0xFF), out); break;
    case R_CX: obj_wyde2 ((uint16_t)(ctx->Ecx&0xFFFF), out); break;

    case R_DL: obj_byte2 ((uint8_t)(ctx->Edx&0xFF), out); break;
    case R_DH: obj_byte2 ((uint8_t)((ctx->Edx>>8)&0xFF), out); break;
    case R_DX: obj_wyde2 ((uint16_t)(ctx->Edx&0xFFFF), out); break;

    case R_SI: obj_wyde2 ((uint16_t)(ctx->Esi&0xFFFF), out); break;

    case R_DI: obj_wyde2 ((uint16_t)(ctx->Edi&0xFFFF), out); break;

    case R_BP: obj_wyde2 ((uint16_t)(ctx->Ebp&0xFFFF), out); break;
#endif

    case R_ES: obj_wyde2 ((uint16_t)ctx->SegEs, out); break;
    case R_DS: obj_wyde2 ((uint16_t)ctx->SegDs, out); break;
    case R_FS: obj_wyde2 ((uint16_t)ctx->SegFs, out); break;
    case R_GS: obj_wyde2 ((uint16_t)ctx->SegGs, out); break;
    case R_CS: obj_wyde2 ((uint16_t)ctx->SegCs, out); break;
    case R_SS: obj_wyde2 ((uint16_t)ctx->SegSs, out); break;
    
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
        case R_XMM0: obj_xmm2 ((uint8_t*)&t->XmmRegisters[0], out); break;
        case R_XMM1: obj_xmm2 ((uint8_t*)&t->XmmRegisters[1], out); break;
        case R_XMM2: obj_xmm2 ((uint8_t*)&t->XmmRegisters[2], out); break;
        case R_XMM3: obj_xmm2 ((uint8_t*)&t->XmmRegisters[3], out); break;
        case R_XMM4: obj_xmm2 ((uint8_t*)&t->XmmRegisters[4], out); break;
        case R_XMM5: obj_xmm2 ((uint8_t*)&t->XmmRegisters[5], out); break;
        case R_XMM6: obj_xmm2 ((uint8_t*)&t->XmmRegisters[6], out); break;
        case R_XMM7: obj_xmm2 ((uint8_t*)&t->XmmRegisters[7], out); break;
#ifdef _WIN64                    
        case R_XMM8: obj_xmm2 ((uint8_t*)&t->XmmRegisters[8], out); break;
        case R_XMM9: obj_xmm2 ((uint8_t*)&t->XmmRegisters[9], out); break;
        case R_XMM10: obj_xmm2 ((uint8_t*)&t->XmmRegisters[10], out); break;
        case R_XMM11: obj_xmm2 ((uint8_t*)&t->XmmRegisters[11], out); break;
        case R_XMM12: obj_xmm2 ((uint8_t*)&t->XmmRegisters[12], out); break;
        case R_XMM13: obj_xmm2 ((uint8_t*)&t->XmmRegisters[13], out); break;
        case R_XMM14: obj_xmm2 ((uint8_t*)&t->XmmRegisters[14], out); break;
        case R_XMM15: obj_xmm2 ((uint8_t*)&t->XmmRegisters[15], out); break;
#endif
        default: assert(0);
        };
        break;
    
    case R_ST0: obj_double2 ((double)(*(long double*)&t->FloatRegisters[0]), out); break;
    case R_ST1: obj_double2 ((double)(*(long double*)&t->FloatRegisters[1]), out); break;
    case R_ST2: obj_double2 ((double)(*(long double*)&t->FloatRegisters[2]), out); break;
    case R_ST3: obj_double2 ((double)(*(long double*)&t->FloatRegisters[3]), out); break;
    case R_ST4: obj_double2 ((double)(*(long double*)&t->FloatRegisters[4]), out); break;
    case R_ST5: obj_double2 ((double)(*(long double*)&t->FloatRegisters[5]), out); break;
    case R_ST6: obj_double2 ((double)(*(long double*)&t->FloatRegisters[6]), out); break;
    case R_ST7: obj_double2 ((double)(*(long double*)&t->FloatRegisters[7]), out); break;

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

    obj val;
    X86_register_get_value (r, ctx, &val);
    rt=zero_extend_to_octabyte (&val);
    return rt;
};

void X86_register_set_value (X86_register r, CONTEXT *ctx, obj *val)
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
        case R_XMM0: memcpy (&t->XmmRegisters[0], obj_get_as_xmm(val), 16); break;
        case R_XMM1: memcpy (&t->XmmRegisters[1], obj_get_as_xmm(val), 16); break;
        case R_XMM2: memcpy (&t->XmmRegisters[2], obj_get_as_xmm(val), 16); break;
        case R_XMM3: memcpy (&t->XmmRegisters[3], obj_get_as_xmm(val), 16); break;
        case R_XMM4: memcpy (&t->XmmRegisters[4], obj_get_as_xmm(val), 16); break;
        case R_XMM5: memcpy (&t->XmmRegisters[5], obj_get_as_xmm(val), 16); break;
        case R_XMM6: memcpy (&t->XmmRegisters[6], obj_get_as_xmm(val), 16); break;
        case R_XMM7: memcpy (&t->XmmRegisters[7], obj_get_as_xmm(val), 16); break;
        case R_XMM8: memcpy (&t->XmmRegisters[8], obj_get_as_xmm(val), 16); break;
#ifdef _WIN64
        case R_XMM9: memcpy (&t->XmmRegisters[9], obj_get_as_xmm(val), 16); break;
        case R_XMM10: memcpy (&t->XmmRegisters[10], obj_get_as_xmm(val), 16); break;
        case R_XMM11: memcpy (&t->XmmRegisters[11], obj_get_as_xmm(val), 16); break;
        case R_XMM12: memcpy (&t->XmmRegisters[12], obj_get_as_xmm(val), 16); break;
        case R_XMM13: memcpy (&t->XmmRegisters[13], obj_get_as_xmm(val), 16); break;
        case R_XMM14: memcpy (&t->XmmRegisters[14], obj_get_as_xmm(val), 16); break;
        case R_XMM15: memcpy (&t->XmmRegisters[15], obj_get_as_xmm(val), 16); break;
#endif
        default: assert(0);
        };

        break;

#ifdef _WIN64
    case R_R8D:  ctx->R8= (ctx->R8&0xFFFFFFFF00000000)  | (obj_get_as_tetrabyte (val)); break;
    case R_R9D:  ctx->R9= (ctx->R9&0xFFFFFFFF00000000)  | (obj_get_as_tetrabyte (val)); break;
    case R_R10D: ctx->R10=(ctx->R10&0xFFFFFFFF00000000) | (obj_get_as_tetrabyte (val)); break;
    case R_R11D: ctx->R11=(ctx->R11&0xFFFFFFFF00000000) | (obj_get_as_tetrabyte (val)); break;
    case R_R12D: ctx->R12=(ctx->R12&0xFFFFFFFF00000000) | (obj_get_as_tetrabyte (val)); break;
    case R_R13D: ctx->R13=(ctx->R13&0xFFFFFFFF00000000) | (obj_get_as_tetrabyte (val)); break;
    case R_R14D: ctx->R14=(ctx->R14&0xFFFFFFFF00000000) | (obj_get_as_tetrabyte (val)); break;
    case R_R15D: ctx->R15=(ctx->R15&0xFFFFFFFF00000000) | (obj_get_as_tetrabyte (val)); break;
        // TODO: to add a lot
#else
    case R_EAX: ctx->Eax=obj_get_as_tetrabyte (val); break;
    case R_EBX: ctx->Ebx=obj_get_as_tetrabyte (val); break;
    case R_ECX: ctx->Ecx=obj_get_as_tetrabyte (val); break;
    case R_EDX: ctx->Edx=obj_get_as_tetrabyte (val); break;
    case R_ESI: ctx->Esi=obj_get_as_tetrabyte (val); break;
    case R_EDI: ctx->Edi=obj_get_as_tetrabyte (val); break;
    case R_EBP: ctx->Ebp=obj_get_as_tetrabyte (val); break;
    case R_ESP: ctx->Esp=obj_get_as_tetrabyte (val); break;
    case R_EIP: ctx->Eip=obj_get_as_tetrabyte (val); break;

    case R_AX: ctx->Eax=(ctx->Eax&0xFFFF0000) | (obj_get_as_wyde(val)); break;
    case R_BX: ctx->Ebx=(ctx->Ebx&0xFFFF0000) | (obj_get_as_wyde(val)); break;
    case R_CX: ctx->Ecx=(ctx->Ecx&0xFFFF0000) | (obj_get_as_wyde(val)); break;
    case R_DX: ctx->Edx=(ctx->Edx&0xFFFF0000) | (obj_get_as_wyde(val)); break;
    case R_SI: ctx->Esi=(ctx->Esi&0xFFFF0000) | (obj_get_as_wyde(val)); break;
    case R_DI: ctx->Edi=(ctx->Edi&0xFFFF0000) | (obj_get_as_wyde(val)); break;
    case R_BP: ctx->Ebp=(ctx->Ebp&0xFFFF0000) | (obj_get_as_wyde(val)); break;
    case R_SP: ctx->Esp=(ctx->Esp&0xFFFF0000) | (obj_get_as_wyde(val)); break;

    case R_AL: ctx->Eax=(ctx->Eax&0xFFFFFF00) | (obj_get_as_byte(val)); break;
    case R_BL: ctx->Ebx=(ctx->Ebx&0xFFFFFF00) | (obj_get_as_byte(val)); break;
    case R_CL: ctx->Ecx=(ctx->Ecx&0xFFFFFF00) | (obj_get_as_byte(val)); break;
    case R_DL: ctx->Edx=(ctx->Edx&0xFFFFFF00) | (obj_get_as_byte(val)); break;

    case R_AH: ctx->Eax=(ctx->Eax&0xFFFF00FF) | ((obj_get_as_byte(val))<<8); break;
    case R_BH: ctx->Ebx=(ctx->Ebx&0xFFFF00FF) | ((obj_get_as_byte(val))<<8); break;
    case R_CH: ctx->Ecx=(ctx->Ecx&0xFFFF00FF) | ((obj_get_as_byte(val))<<8); break;
    case R_DH: ctx->Edx=(ctx->Edx&0xFFFF00FF) | ((obj_get_as_byte(val))<<8); break;
#endif
    default:
        printf ("%s. r=%s\n", __FUNCTION__, X86_register_ToString (r));
        assert (0); break;
    };
};

/* vim: set expandtab ts=4 sw=4 : */
