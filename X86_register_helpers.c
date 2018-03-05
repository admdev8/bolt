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

#include "oassert.h"
#include <stdio.h>
#include "X86_register_helpers.h"
#include "FPU_stuff.h"
#include "CONTEXT_utils.h"

void X86_register_get_value (enum X86_register r, const CONTEXT *ctx, obj* out)
{
    unsigned idx;

    switch (r)
    {
#ifdef _WIN64
        case R_RAX: obj_octa2 ((uint64_t)ctx->Rax, out); break;
        case R_RBX: obj_octa2 ((uint64_t)ctx->Rbx, out); break;
        case R_RCX: obj_octa2 ((uint64_t)ctx->Rcx, out); break;
        case R_RDX: obj_octa2 ((uint64_t)ctx->Rdx, out); break;
        case R_RSI: obj_octa2 ((uint64_t)ctx->Rsi, out); break;
        case R_RDI: obj_octa2 ((uint64_t)ctx->Rdi, out); break;
        case R_RSP: obj_octa2 ((uint64_t)ctx->Rsp, out); break;
        case R_RBP: obj_octa2 ((uint64_t)ctx->Rbp, out); break;
        case R_RIP: obj_octa2 ((uint64_t)ctx->Rip, out); break;

        case R_R8:  obj_octa2 ((uint64_t)ctx->R8, out); break;
        case R_R9:  obj_octa2 ((uint64_t)ctx->R9, out); break;
        case R_R10: obj_octa2 ((uint64_t)ctx->R10, out); break;
        case R_R11: obj_octa2 ((uint64_t)ctx->R11, out); break;
        case R_R12: obj_octa2 ((uint64_t)ctx->R12, out); break;
        case R_R13: obj_octa2 ((uint64_t)ctx->R13, out); break;
        case R_R14: obj_octa2 ((uint64_t)ctx->R14, out); break;
        case R_R15: obj_octa2 ((uint64_t)ctx->R15, out); break;

        case R_R8D:  obj_tetra2 ((uint32_t)(ctx->R8 & 0xFFFFFFFF), out); break;
        case R_R9D:  obj_tetra2 ((uint32_t)(ctx->R9 & 0xFFFFFFFF), out); break;
        case R_R10D: obj_tetra2 ((uint32_t)(ctx->R10 & 0xFFFFFFFF), out); break;
        case R_R11D: obj_tetra2 ((uint32_t)(ctx->R11 & 0xFFFFFFFF), out); break;
        case R_R12D: obj_tetra2 ((uint32_t)(ctx->R12 & 0xFFFFFFFF), out); break;
        case R_R13D: obj_tetra2 ((uint32_t)(ctx->R13 & 0xFFFFFFFF), out); break;
        case R_R14D: obj_tetra2 ((uint32_t)(ctx->R14 & 0xFFFFFFFF), out); break;
        case R_R15D: obj_tetra2 ((uint32_t)(ctx->R15 & 0xFFFFFFFF), out); break;

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

        case R_EAX: obj_tetra2 ((uint32_t)(ctx->Rax & 0xFFFFFFFF), out); break;
        case R_EBX: obj_tetra2 ((uint32_t)(ctx->Rbx & 0xFFFFFFFF), out); break;
        case R_ECX: obj_tetra2 ((uint32_t)(ctx->Rcx & 0xFFFFFFFF), out); break;
        case R_EDX: obj_tetra2 ((uint32_t)(ctx->Rdx & 0xFFFFFFFF), out); break;
        case R_ESI: obj_tetra2 ((uint32_t)(ctx->Rsi & 0xFFFFFFFF), out); break;
        case R_EDI: obj_tetra2 ((uint32_t)(ctx->Rdi & 0xFFFFFFFF), out); break;
        case R_EBP: obj_tetra2 ((uint32_t)(ctx->Rbp & 0xFFFFFFFF), out); break;
        case R_ESP: obj_tetra2 ((uint32_t)(ctx->Rsp & 0xFFFFFFFF), out); break;
        case R_EIP: obj_tetra2 ((uint32_t)(ctx->Rip & 0xFFFFFFFF), out); break;

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
        case R_EAX: obj_tetra2 ((uint32_t)ctx->Eax, out); break;
        case R_EBX: obj_tetra2 ((uint32_t)ctx->Ebx, out); break;
        case R_ECX: obj_tetra2 ((uint32_t)ctx->Ecx, out); break;
        case R_EDX: obj_tetra2 ((uint32_t)ctx->Edx, out); break;
        case R_ESI: obj_tetra2 ((uint32_t)ctx->Esi, out); break;
        case R_EDI: obj_tetra2 ((uint32_t)ctx->Edi, out); break;
        case R_EBP: obj_tetra2 ((uint32_t)ctx->Ebp, out); break;
        case R_ESP: obj_tetra2 ((uint32_t)ctx->Esp, out); break;
        case R_EIP: obj_tetra2 ((uint32_t)ctx->Eip, out); break;

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
                       case R_XMM0: idx=0; break;
                       case R_XMM1: idx=1; break;
                       case R_XMM2: idx=2; break;
                       case R_XMM3: idx=3; break;
                       case R_XMM4: idx=4; break;
                       case R_XMM5: idx=5; break;
                       case R_XMM6: idx=6; break;
                       case R_XMM7: idx=7; break;
#ifdef _WIN64                    
                       case R_XMM8: idx=8; break;
                       case R_XMM9: idx=9; break;
                       case R_XMM10: idx=10; break;
                       case R_XMM11: idx=11; break;
                       case R_XMM12: idx=12; break;
                       case R_XMM13: idx=13; break;
                       case R_XMM14: idx=14; break;
                       case R_XMM15: idx=15; break;
#endif
                       default: oassert(0);
                   };
                   
                   obj_xmm2 ((uint8_t*)&get_XMM_SAVE_AREA32(ctx)->XmmRegisters[idx], out);
                   break;

        case R_ST0: obj_double2 (get_STx(ctx, 0), out); break;
        case R_ST1: obj_double2 (get_STx(ctx, 1), out); break;
        case R_ST2: obj_double2 (get_STx(ctx, 2), out); break;
        case R_ST3: obj_double2 (get_STx(ctx, 3), out); break;
        case R_ST4: obj_double2 (get_STx(ctx, 4), out); break;
        case R_ST5: obj_double2 (get_STx(ctx, 5), out); break;
        case R_ST6: obj_double2 (get_STx(ctx, 6), out); break;
        case R_ST7: obj_double2 (get_STx(ctx, 7), out); break;

        case R_ABSENT:
                    oassert(0);
                    break;

        default:
                    oassert (!"this register isn't implemented here yet");
                    oassert(0);
                    break;
    };
};

uint64_t X86_register_get_value_as_u64 (enum X86_register r, const CONTEXT *ctx)
{
    uint64_t rt;

    obj val;
    X86_register_get_value (r, ctx, &val);
    rt=zero_extend_to_octa (&val);
    return rt;
};

void X86_register_set_value (enum X86_register r, CONTEXT *ctx, obj *val)
{
    unsigned idx;

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
                case R_XMM0: idx=0; break;
                case R_XMM1: idx=1; break;
                case R_XMM2: idx=2; break;
                case R_XMM3: idx=3; break;
                case R_XMM4: idx=4; break;
                case R_XMM5: idx=5; break;
                case R_XMM6: idx=6; break;
                case R_XMM7: idx=7; break;
#ifdef _WIN64
                case R_XMM8: idx=8; break;
                case R_XMM9: idx=9; break;
                case R_XMM10: idx=10; break;
                case R_XMM11: idx=11; break;
                case R_XMM12: idx=12; break;
                case R_XMM13: idx=13; break;
                case R_XMM14: idx=14; break;
                case R_XMM15: idx=15; break;
#endif
                default: oassert(0);
            };
    
            memcpy (&get_XMM_SAVE_AREA32(ctx)->XmmRegisters[idx], obj_get_as_xmm(val), 16); 
            break;

#ifdef _WIN64
        case R_RAX: ctx->Rax=obj_get_as_octa (val); break;
        case R_RBX: ctx->Rbx=obj_get_as_octa (val); break;
        case R_RCX: ctx->Rcx=obj_get_as_octa (val); break;
        case R_RDX: ctx->Rdx=obj_get_as_octa (val); break;
        case R_RSI: ctx->Rsi=obj_get_as_octa (val); break;
        case R_RDI: ctx->Rdi=obj_get_as_octa (val); break;
        case R_RBP: ctx->Rbp=obj_get_as_octa (val); break;
        case R_RSP: ctx->Rsp=obj_get_as_octa (val); break;

        case R_R8: ctx->R8=obj_get_as_octa (val); break;
        case R_R9: ctx->R9=obj_get_as_octa (val); break;
        case R_R10: ctx->R10=obj_get_as_octa (val); break;
        case R_R11: ctx->R11=obj_get_as_octa (val); break;
        case R_R12: ctx->R12=obj_get_as_octa (val); break;
        case R_R13: ctx->R13=obj_get_as_octa (val); break;
        case R_R14: ctx->R14=obj_get_as_octa (val); break;
        case R_R15: ctx->R15=obj_get_as_octa (val); break;

        case R_R8D:  ctx->R8= (ctx->R8&0xFFFFFFFF00000000)  | (obj_get_as_tetra (val)); break;
        case R_R9D:  ctx->R9= (ctx->R9&0xFFFFFFFF00000000)  | (obj_get_as_tetra (val)); break;
        case R_R10D: ctx->R10=(ctx->R10&0xFFFFFFFF00000000) | (obj_get_as_tetra (val)); break;
        case R_R11D: ctx->R11=(ctx->R11&0xFFFFFFFF00000000) | (obj_get_as_tetra (val)); break;
        case R_R12D: ctx->R12=(ctx->R12&0xFFFFFFFF00000000) | (obj_get_as_tetra (val)); break;
        case R_R13D: ctx->R13=(ctx->R13&0xFFFFFFFF00000000) | (obj_get_as_tetra (val)); break;
        case R_R14D: ctx->R14=(ctx->R14&0xFFFFFFFF00000000) | (obj_get_as_tetra (val)); break;
        case R_R15D: ctx->R15=(ctx->R15&0xFFFFFFFF00000000) | (obj_get_as_tetra (val)); break;

        case R_R8L: ctx->R8=(ctx->R8&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_R9L: ctx->R9=(ctx->R9&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_R10L: ctx->R10=(ctx->R10&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_R11L: ctx->R11=(ctx->R11&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_R12L: ctx->R12=(ctx->R12&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_R13L: ctx->R13=(ctx->R13&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_R14L: ctx->R14=(ctx->R14&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_R15L: ctx->R15=(ctx->R15&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;

        case R_DIL: ctx->Rdi=(ctx->Rdi&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_SIL: ctx->Rsi=(ctx->Rsi&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;

        case R_AL: ctx->Rax=(ctx->Rax&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_BL: ctx->Rbx=(ctx->Rbx&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_CL: ctx->Rcx=(ctx->Rcx&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;
        case R_DL: ctx->Rdx=(ctx->Rdx&0xFFFFFFFFFFFFFF00) | (obj_get_as_byte(val)); break;

        case R_RIP:  ctx->Rip=obj_get_as_octa (val); break;

        case R_EAX: ctx->Rax=(ctx->Rax&0xFFFFFFFF00000000)  | obj_get_as_tetra (val); break;
        case R_EBX: ctx->Rax=(ctx->Rbx&0xFFFFFFFF00000000)  | obj_get_as_tetra (val); break;
        case R_ECX: ctx->Rax=(ctx->Rcx&0xFFFFFFFF00000000)  | obj_get_as_tetra (val); break;
        case R_EDX: ctx->Rax=(ctx->Rdx&0xFFFFFFFF00000000)  | obj_get_as_tetra (val); break;
        case R_ESI: ctx->Rax=(ctx->Rsi&0xFFFFFFFF00000000)  | obj_get_as_tetra (val); break;
        case R_EDI: ctx->Rax=(ctx->Rdi&0xFFFFFFFF00000000)  | obj_get_as_tetra (val); break;
        case R_EBP: ctx->Rbp=(ctx->Rbp&0xFFFFFFFF00000000)  | obj_get_as_tetra (val); break;
#else
        case R_EAX: ctx->Eax=obj_get_as_tetra (val); break;
        case R_EBX: ctx->Ebx=obj_get_as_tetra (val); break;
        case R_ECX: ctx->Ecx=obj_get_as_tetra (val); break;
        case R_EDX: ctx->Edx=obj_get_as_tetra (val); break;
        case R_ESI: ctx->Esi=obj_get_as_tetra (val); break;
        case R_EDI: ctx->Edi=obj_get_as_tetra (val); break;
        case R_EBP: ctx->Ebp=obj_get_as_tetra (val); break;
        case R_ESP: ctx->Esp=obj_get_as_tetra (val); break;
        case R_EIP: ctx->Eip=obj_get_as_tetra (val); break;

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

        case R_ST0: CONTEXT_set_reg_STx (ctx, 0, obj_get_as_double(val)); break;
        case R_ST1: CONTEXT_set_reg_STx (ctx, 1, obj_get_as_double(val)); break;
        case R_ST2: CONTEXT_set_reg_STx (ctx, 2, obj_get_as_double(val)); break;
        case R_ST3: CONTEXT_set_reg_STx (ctx, 3, obj_get_as_double(val)); break;
        case R_ST4: CONTEXT_set_reg_STx (ctx, 4, obj_get_as_double(val)); break;
        case R_ST5: CONTEXT_set_reg_STx (ctx, 5, obj_get_as_double(val)); break;
        case R_ST6: CONTEXT_set_reg_STx (ctx, 6, obj_get_as_double(val)); break;
        case R_ST7: CONTEXT_set_reg_STx (ctx, 7, obj_get_as_double(val)); break;
        default:
                    printf ("%s. r=%s, %d\n", __FUNCTION__, X86_register_ToString (r), r);
                    oassert(0);
    };
};

/* vim: set expandtab ts=4 sw=4 : */
