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
#include <math.h>

#include "CONTEXT_utils.h"
#include "address.h"
#include "FPU_stuff.h"
#include "bitfields.h"
#include "x86.h"

XSAVE_FORMAT* get_XSAVE_FORMAT(CONTEXT *ctx)
{
#ifdef _WIN64
    return &ctx->FltSave;
#else
    return &ctx->ExtendedRegisters[0];
#endif
};

bool STx_present_in_tag(CONTEXT *ctx, unsigned reg)
{
    return IS_SET(get_XSAVE_FORMAT(ctx)->TagWord, 1<<(7-reg));
};

void FPU_set_tag(CONTEXT *ctx, unsigned reg)
{
    SET_BIT(get_XSAVE_FORMAT(ctx)->TagWord, 1<<(7-reg));
};

double get_STx (CONTEXT *ctx, unsigned reg)
{
    return (double)*(long double*)&get_XSAVE_FORMAT(ctx)->FloatRegisters[reg];
};

void set_TF (CONTEXT *ctx)
{
    SET_BIT (ctx->EFlags, FLAG_TF);
};

void clear_TF (CONTEXT *ctx)
{
    REMOVE_BIT (ctx->EFlags, FLAG_TF);
};

void set_or_clear_flag (CONTEXT * ctx, int flag, uint64_t cond)
{
    if (cond)
        SET_BIT (ctx->EFlags, flag);
    else
        REMOVE_BIT (ctx->EFlags, flag);
};

bool is_NZ_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_ZF)==false)
        return true;
    else
        return false;
};

bool is_BE_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_CF) || IS_SET (ctx->EFlags, FLAG_ZF))
        return true;
    else
        return false;
};

bool is_Z_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_ZF))
        return true;
    else
        return false;
};

bool is_O_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_OF))
        return true;
    else
        return false;
};

bool is_S_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_SF))
        return true;
    else
        return false;
};

bool is_NS_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_SF)==false)
        return true;
    else
        return false;
};

bool is_G_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_ZF)==false && (IS_SET (ctx->EFlags, FLAG_SF)==IS_SET (ctx->EFlags, FLAG_OF)))
        return true;
    else
        return false;
};

bool is_GE_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_SF)==IS_SET (ctx->EFlags, FLAG_OF))
        return true;
    else
        return false;
};

bool is_L_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_SF)!=IS_SET (ctx->EFlags, FLAG_OF))
        return true;
    else
        return false;
};

bool is_A_cond (const CONTEXT * ctx)
{
    if ((IS_SET (ctx->EFlags, FLAG_CF)==false) && (IS_SET (ctx->EFlags, FLAG_ZF)==false))
        return true;
    else
        return false;
};

bool is_B_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_CF))
        return true;
    else
        return false;
};

bool is_NB_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_CF)==false)
        return true;
    else
        return false;
};

bool is_LE_cond (const CONTEXT * ctx)
{
    if (IS_SET (ctx->EFlags, FLAG_ZF) || (IS_SET (ctx->EFlags, FLAG_SF)!=IS_SET (ctx->EFlags, FLAG_OF)))
        return true;
    else
        return false;
};

REG CONTEXT_get_SP (const CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rsp;
#else
    return ctx->Esp;
#endif
};

void CONTEXT_set_SP (CONTEXT * ctx, REG val)
{
#ifdef _WIN64
    ctx->Rsp=val;
#else
    ctx->Esp=val;
#endif
};

REG CONTEXT_get_BP (const CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rbp;
#else
    return ctx->Ebp;
#endif
};

void CONTEXT_set_BP (CONTEXT * ctx, REG val)
{
#ifdef _WIN64
    ctx->Rbp=val;
#else
    ctx->Ebp=val;
#endif
};

REG CONTEXT_get_Accum (const CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rax;
#else
    return ctx->Eax;
#endif
};

void CONTEXT_set_Accum (CONTEXT * ctx, REG A)
{
#ifdef _WIN64
    ctx->Rax=A;
#else
    ctx->Eax=A;
#endif
};

REG CONTEXT_get_xCX (const CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rcx;
#else
    return ctx->Ecx;
#endif
};

void CONTEXT_set_xCX (CONTEXT * ctx, REG val)
{
#ifdef _WIN64
    ctx->Rcx=val;
#else
    ctx->Ecx=val;
#endif
};

REG CONTEXT_get_xDX (const CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rdx;
#else
    return ctx->Edx;
#endif
};

void CONTEXT_set_xDX (CONTEXT * ctx, REG val)
{
#ifdef _WIN64
    ctx->Rdx=val;
#else
    ctx->Edx=val;
#endif
};

REG CONTEXT_get_xDI (const CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rdi;
#else
    return ctx->Edi;
#endif
};

void CONTEXT_set_xDI (CONTEXT * ctx, REG val)
{
#ifdef _WIN64
    ctx->Rdi=val;
#else
    ctx->Edi=val;
#endif
};

REG CONTEXT_get_xSI (const CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rsi;
#else
    return ctx->Esi;
#endif
};

void CONTEXT_set_xSI (CONTEXT * ctx, REG val)
{
#ifdef _WIN64
    ctx->Rsi=val;
#else
    ctx->Esi=val;
#endif
};

REG CONTEXT_get_PC (const CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rip;
#else
    return ctx->Eip;
#endif
};

void CONTEXT_set_PC (CONTEXT * ctx, REG PC)
{
#ifdef _WIN64
    ctx->Rip=PC;
#else
    ctx->Eip=PC;
#endif
};

void CONTEXT_decrement_PC (CONTEXT * ctx)
{
    CONTEXT_set_PC(ctx, CONTEXT_get_PC(ctx)-1);
};

void CONTEXT_add_to_PC (CONTEXT * ctx, REG i)
{
#ifdef _WIN64
    ctx->Rip+=i;
#else
    ctx->Eip+=i;
#endif
};

void CONTEXT_setDRx_and_DR7 (CONTEXT * ctx, int bp_i, REG a)
{
    //IF_VERBOSE (2, log_stream() << __FUNCTION__ << " (bp_i=" << bp_i << " a=" << a << ")\n"; );

    oassert(bp_i<4);

    switch (bp_i)
    {
    case 0: ctx->Dr0=a; SET_BIT (ctx->Dr7, FLAG_DR7_L0); break;
    case 1: ctx->Dr1=a; SET_BIT (ctx->Dr7, FLAG_DR7_L1); break;
    case 2: ctx->Dr2=a; SET_BIT (ctx->Dr7, FLAG_DR7_L2); break;
    case 3: ctx->Dr3=a; SET_BIT (ctx->Dr7, FLAG_DR7_L3); break;
    default: 
        oassert (0);
        break;
    };

    //IF_VERBOSE (2, log_stream() << __FUNCTION__ << ": ctx->Dr7 state=0x" << hex << ctx->Dr7 << endl; );
};

static uint8_t *empty_XMM_register=(uint8_t *)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

void dump_DRx (fds* s, const CONTEXT *ctx)
{
#ifdef _WIN64
    L_fds (s, "DR0=0x" PRI_REG_HEX_PAD "\n", ctx->Dr0);
    L_fds (s, "DR1=0x" PRI_REG_HEX_PAD "\n", ctx->Dr1);
    L_fds (s, "DR2=0x" PRI_REG_HEX_PAD "\n", ctx->Dr2);
    L_fds (s, "DR3=0x" PRI_REG_HEX_PAD "\n", ctx->Dr3);
    L_fds (s, "DR6=0x" PRI_REG_HEX_PAD "\n", ctx->Dr6);
    L_fds (s, "DR7=");
    dump_DR7 (s, ctx->Dr7);
    L_fds (s, "\n");
#else
    L_fds (s, "DR0=0x" PRI_REG_HEX_PAD " DR1=0x" PRI_REG_HEX_PAD " DR2=0x" PRI_REG_HEX_PAD " DR3=0x" PRI_REG_HEX_PAD "\n", ctx->Dr0, ctx->Dr1, ctx->Dr2, ctx->Dr3);
    L_fds (s, "DR6=0x" PRI_REG_HEX_PAD " DR7=", ctx->Dr6);
    dump_DR7 (s, ctx->Dr7);
    L_fds (s, "\n");
#endif    
};

void dump_CONTEXT (fds* s, CONTEXT * ctx, bool _dump_FPU, bool _dump_DRx, bool dump_xmm_regs)
{
    XSAVE_FORMAT *x=get_XSAVE_FORMAT(ctx);
    int i;

#ifdef _WIN64
    L_fds (s, "RAX=" PRI_REG_HEX_PAD " RBX=" PRI_REG_HEX_PAD " RCX=" PRI_REG_HEX_PAD "\n", ctx->Rax, ctx->Rbx, ctx->Rcx);
    L_fds (s, "RDX=" PRI_REG_HEX_PAD " RSP=" PRI_REG_HEX_PAD " RBP=" PRI_REG_HEX_PAD "\n", ctx->Rdx, ctx->Rsp, ctx->Rbp);
    L_fds (s, "RSI=" PRI_REG_HEX_PAD " RDI=" PRI_REG_HEX_PAD " R8 =" PRI_REG_HEX_PAD "\n", ctx->Rsi, ctx->Rdi, ctx->R8 );
    L_fds (s, "R9 =" PRI_REG_HEX_PAD " R10=" PRI_REG_HEX_PAD " R11=" PRI_REG_HEX_PAD "\n", ctx->R9,  ctx->R10, ctx->R11);
    L_fds (s, "R12=" PRI_REG_HEX_PAD " R13=" PRI_REG_HEX_PAD " R14=" PRI_REG_HEX_PAD "\n", ctx->R12, ctx->R13, ctx->R14);
    L_fds (s, "R15=" PRI_REG_HEX_PAD " RIP=" PRI_REG_HEX_PAD "\n", ctx->R15, ctx->Rip);
#else
    L_fds (s, "EAX=0x" PRI_REG_HEX_PAD " EBX=0x" PRI_REG_HEX_PAD " ECX=0x" PRI_REG_HEX_PAD " EDX=0x" PRI_REG_HEX_PAD "\n", ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx);
    L_fds (s, "ESI=0x" PRI_REG_HEX_PAD " EDI=0x" PRI_REG_HEX_PAD " EBP=0x" PRI_REG_HEX_PAD " ESP=0x" PRI_REG_HEX_PAD "\n", ctx->Esi, ctx->Edi, ctx->Ebp, ctx->Esp);
    L_fds (s, "EIP=0x" PRI_REG_HEX_PAD "\n", ctx->Eip);
#endif
    L_fds (s, "FLAGS=");
    dump_flags(s, ctx->EFlags);
    L_fds (s, "\n");

    if (_dump_DRx)
        dump_DRx(s, ctx);

    if (_dump_FPU)
        dump_FPU(s, ctx);

    if (sse_supported() && dump_xmm_regs)
    {
        strbuf sb_MXCSR=STRBUF_INIT;
        MXCSR_to_str(x->MxCsr, &sb_MXCSR);
        L_fds(s, "MxCsr=%s\n", sb_MXCSR.buf);
        strbuf_deinit(&sb_MXCSR);
        
        for (i=0; i<XMM_REGISTERS_TOTAL; i++)
            if (memcmp (((BYTE*)&x->XmmRegisters[i]), empty_XMM_register, 16)!=0) // isn't empty?
            {
                strbuf sb=STRBUF_INIT;
                XMM_to_strbuf((BYTE*)&x->XmmRegisters[i], &sb);
                L_fds (s, "XMM%d = %s\n", i, sb.buf);
                strbuf_deinit(&sb);
            };
    };
};

bool CONTEXT_compare (fds* s, CONTEXT * ctx1, CONTEXT * ctx2) // ignoring TP/TF flag!
{
    bool rt=true; // so far so good
    DWORD new_eflags1, new_eflags2;

    XSAVE_FORMAT *t1=get_XSAVE_FORMAT(ctx1);
    XSAVE_FORMAT *t2=get_XSAVE_FORMAT(ctx2);

    unsigned i;
#ifdef _WIN64
    oassert(!"should be rewritten");
    if (ctx1->Rax!=ctx2->Rax) { L_fds (s, "ctx1->RAX=0x" PRI_REG_HEX_PAD " ctx2->RAX=0x" PRI_REG_HEX_PAD "\n", ctx1->Rax, ctx2->Rax); rt=false; }
    if (ctx1->Rbx!=ctx2->Rbx) { L_fds (s, "ctx1->RBX=0x" PRI_REG_HEX_PAD " ctx2->RBX=0x" PRI_REG_HEX_PAD "\n", ctx1->Rbx, ctx2->Rbx); rt=false; }
    if (ctx1->Rcx!=ctx2->Rcx) { L_fds (s, "ctx1->RCX=0x" PRI_REG_HEX_PAD " ctx2->RCX=0x" PRI_REG_HEX_PAD "\n", ctx1->Rcx, ctx2->Rcx); rt=false; }
    if (ctx1->Rdx!=ctx2->Rdx) { L_fds (s, "ctx1->RDX=0x" PRI_REG_HEX_PAD " ctx2->RDX=0x" PRI_REG_HEX_PAD "\n", ctx1->Rdx, ctx2->Rdx); rt=false; }
    if (ctx1->Rsp!=ctx2->Rsp) { L_fds (s, "ctx1->RSP=0x" PRI_REG_HEX_PAD " ctx2->RSP=0x" PRI_REG_HEX_PAD "\n", ctx1->Rsp, ctx2->Rsp); rt=false; }
    if (ctx1->Rbp!=ctx2->Rbp) { L_fds (s, "ctx1->RBP=0x" PRI_REG_HEX_PAD " ctx2->RBP=0x" PRI_REG_HEX_PAD "\n", ctx1->Rbp, ctx2->Rbp); rt=false; }
    if (ctx1->Rsi!=ctx2->Rsi) { L_fds (s, "ctx1->RSI=0x" PRI_REG_HEX_PAD " ctx2->RSI=0x" PRI_REG_HEX_PAD "\n", ctx1->Rsi, ctx2->Rsi); rt=false; }
    if (ctx1->Rdi!=ctx2->Rdi) { L_fds (s, "ctx1->RDI=0x" PRI_REG_HEX_PAD " ctx2->RDI=0x" PRI_REG_HEX_PAD "\n", ctx1->Rdi, ctx2->Rdi); rt=false; }
    if (ctx1->R8 !=ctx2->R8)  { L_fds (s, "ctx1->R8 =0x" PRI_REG_HEX_PAD " ctx2->R8 =0x" PRI_REG_HEX_PAD "\n", ctx1->R8,  ctx2->R8);  rt=false; }
    if (ctx1->R9 !=ctx2->R9)  { L_fds (s, "ctx1->R9 =0x" PRI_REG_HEX_PAD " ctx2->R9 =0x" PRI_REG_HEX_PAD "\n", ctx1->R9,  ctx2->R9);  rt=false; }
    if (ctx1->R10!=ctx2->R10) { L_fds (s, "ctx1->R10=0x" PRI_REG_HEX_PAD " ctx2->R10=0x" PRI_REG_HEX_PAD "\n", ctx1->R10, ctx2->R10); rt=false; }
    if (ctx1->R11!=ctx2->R11) { L_fds (s, "ctx1->R11=0x" PRI_REG_HEX_PAD " ctx2->R11=0x" PRI_REG_HEX_PAD "\n", ctx1->R11, ctx2->R11); rt=false; }
    if (ctx1->R12!=ctx2->R12) { L_fds (s, "ctx1->R12=0x" PRI_REG_HEX_PAD " ctx2->R12=0x" PRI_REG_HEX_PAD "\n", ctx1->R12, ctx2->R12); rt=false; }
    if (ctx1->R13!=ctx2->R13) { L_fds (s, "ctx1->R13=0x" PRI_REG_HEX_PAD " ctx2->R13=0x" PRI_REG_HEX_PAD "\n", ctx1->R13, ctx2->R13); rt=false; }
    if (ctx1->R14!=ctx2->R14) { L_fds (s, "ctx1->R14=0x" PRI_REG_HEX_PAD " ctx2->R14=0x" PRI_REG_HEX_PAD "\n", ctx1->R14, ctx2->R14); rt=false; }
    if (ctx1->R15!=ctx2->R15) { L_fds (s, "ctx1->R15=0x" PRI_REG_HEX_PAD " ctx2->R15=0x" PRI_REG_HEX_PAD "\n", ctx1->R15, ctx2->R15); rt=false; }
    if (ctx1->Rip!=ctx2->Rip) { L_fds (s, "ctx1->RIP=0x" PRI_REG_HEX_PAD " ctx2->RIP=0x" PRI_REG_HEX_PAD "\n", ctx1->Rip, ctx2->Rip); rt=false; }
#else
    if (ctx1->Eax!=ctx2->Eax) { L_fds (s, "ctx1->EAX=0x" PRI_REG_HEX_PAD " ctx2->EAX=0x" PRI_REG_HEX_PAD "\n", ctx1->Eax, ctx2->Eax); rt=false; }
    if (ctx1->Ebx!=ctx2->Ebx) { L_fds (s, "ctx1->EBX=0x" PRI_REG_HEX_PAD " ctx2->EBX=0x" PRI_REG_HEX_PAD "\n", ctx1->Ebx, ctx2->Ebx); rt=false; }
    if (ctx1->Ecx!=ctx2->Ecx) { L_fds (s, "ctx1->ECX=0x" PRI_REG_HEX_PAD " ctx2->ECX=0x" PRI_REG_HEX_PAD "\n", ctx1->Ecx, ctx2->Ecx); rt=false; }
    if (ctx1->Edx!=ctx2->Edx) { L_fds (s, "ctx1->EDX=0x" PRI_REG_HEX_PAD " ctx2->EDX=0x" PRI_REG_HEX_PAD "\n", ctx1->Edx, ctx2->Edx); rt=false; }
    if (ctx1->Esi!=ctx2->Esi) { L_fds (s, "ctx1->ESI=0x" PRI_REG_HEX_PAD " ctx2->ESI=0x" PRI_REG_HEX_PAD "\n", ctx1->Esi, ctx2->Esi); rt=false; }
    if (ctx1->Edi!=ctx2->Edi) { L_fds (s, "ctx1->EDI=0x" PRI_REG_HEX_PAD " ctx2->EDI=0x" PRI_REG_HEX_PAD "\n", ctx1->Edi, ctx2->Edi); rt=false; }
    if (ctx1->Eip!=ctx2->Eip) { L_fds (s, "ctx1->EIP=0x" PRI_REG_HEX_PAD " ctx2->EIP=0x" PRI_REG_HEX_PAD "\n", ctx1->Eip, ctx2->Eip); rt=false; }
    if (ctx1->Esp!=ctx2->Esp) { L_fds (s, "ctx1->ESP=0x" PRI_REG_HEX_PAD " ctx2->ESP=0x" PRI_REG_HEX_PAD "\n", ctx1->Esp, ctx2->Esp); rt=false; }
#endif
    if (ctx1->Dr0!=ctx2->Dr0) { L_fds (s, "ctx1->DR0=0x" PRI_REG_HEX_PAD " ctx2->DR0=0x" PRI_REG_HEX_PAD "\n", ctx1->Dr0, ctx2->Dr0); rt=false; }
    if (ctx1->Dr1!=ctx2->Dr1) { L_fds (s, "ctx1->DR1=0x" PRI_REG_HEX_PAD " ctx2->DR1=0x" PRI_REG_HEX_PAD "\n", ctx1->Dr1, ctx2->Dr1); rt=false; }
    if (ctx1->Dr2!=ctx2->Dr2) { L_fds (s, "ctx1->DR2=0x" PRI_REG_HEX_PAD " ctx2->DR2=0x" PRI_REG_HEX_PAD "\n", ctx1->Dr2, ctx2->Dr2); rt=false; }
    if (ctx1->Dr3!=ctx2->Dr3) { L_fds (s, "ctx1->DR3=0x" PRI_REG_HEX_PAD " ctx2->DR3=0x" PRI_REG_HEX_PAD "\n", ctx1->Dr3, ctx2->Dr3); rt=false; }
    //if (ctx1->Dr6!=ctx2->Dr6) { L ("ctx1->DR6=0x" PRI_REG_HEX_PAD " ctx2->DR6=0x" PRI_REG_HEX_PAD "\n", ctx1->Dr6, ctx2->Dr6); rt=false; }
    if (ctx1->Dr7!=ctx2->Dr7) 
    { 
        L_fds (s, "ctx1->DR7=0x" PRI_REG_HEX_PAD ": ", ctx1->Dr7);
        dump_DR7(s, ctx1->Dr7);
        L_fds (s, "\n");

        L_fds (s, "ctx2->DR7=0x" PRI_REG_HEX_PAD ": ", ctx2->Dr7); 
        dump_DR7(s, ctx2->Dr7);
        L_fds (s, "\n");

        rt=false; 
    }
    new_eflags1=ctx1->EFlags&(~FLAG_TF)&(~FLAG_RSRV1)&(~FLAG_RF);
    new_eflags2=ctx2->EFlags&(~FLAG_TF)&(~FLAG_RSRV1)&(~FLAG_RF);
    if (new_eflags1 != new_eflags2) 
    { 
        L_fds (s, "ctx1->EFLAGS (new_eflags1)=0x%x (", new_eflags1);
        dump_flags(s, new_eflags1);
        L_fds (s, ")\n");

        L_fds (s, "ctx2->EFLAGS (new_eflags2)=0x%x (", new_eflags2);
        dump_flags(s, new_eflags2);
        L_fds (s, ")\n");

        rt=false;
    };

    for (i=0; i<16; i++)
    {
        if (memcmp (&t1->XmmRegisters[i], &t2->XmmRegisters[i], 16)!=0)
        {
            L ("XMM%d is different in ctx1 and ctx2\n", i);
            rt=false;
        };
    };

    return rt;
};

void CONTEXT_set_reg (CONTEXT * ctx, X86_register r, REG v)
{
    switch (r)
    {
#ifdef _WIN64
    case R_RAX: ctx->Rax=v; break;
    case R_RBX: ctx->Rbx=v; break;
    case R_RCX: ctx->Rcx=v; break;
    case R_RDX: ctx->Rdx=v; break;
    case R_RSI: ctx->Rsi=v; break;
    case R_RDI: ctx->Rdi=v; break;
    case R_RSP: ctx->Rsp=v; break;
    case R_RBP: ctx->Rbp=v; break;
    case R_RIP: ctx->Rip=v; break;
    case R_R8:  ctx->R8=v;  break;
    case R_R9:  ctx->R9=v;  break;
    case R_R10: ctx->R10=v; break;
    case R_R11: ctx->R11=v; break;
    case R_R12: ctx->R12=v; break;
    case R_R13: ctx->R13=v; break;
    case R_R14: ctx->R14=v; break;
    case R_R15: ctx->R15=v; break;
#else
    case R_EAX: ctx->Eax=v; break;
    case R_EBX: ctx->Ebx=v; break;
    case R_ECX: ctx->Ecx=v; break;
    case R_EDX: ctx->Edx=v; break;
    case R_ESI: ctx->Esi=v; break;
    case R_EDI: ctx->Edi=v; break;
    case R_EBP: ctx->Ebp=v; break;
    case R_ESP: ctx->Esp=v; break;
    case R_EIP: ctx->Eip=v; break;
#endif

    // FIXME: FLAG... here!
    case R_PF: v ? SET_BIT (ctx->EFlags, 1<<2) : REMOVE_BIT (ctx->EFlags, 1<<2); break;
    case R_SF: v ? SET_BIT (ctx->EFlags, 1<<7) : REMOVE_BIT (ctx->EFlags, 1<<7); break;
    case R_AF: v ? SET_BIT (ctx->EFlags, 1<<4) : REMOVE_BIT (ctx->EFlags, 1<<4); break;
    case R_ZF: v ? SET_BIT (ctx->EFlags, 1<<6) : REMOVE_BIT (ctx->EFlags, 1<<6); break;
    case R_OF: v ? SET_BIT (ctx->EFlags, 1<<11) : REMOVE_BIT (ctx->EFlags, 1<<11); break;
    case R_CF: v ? SET_BIT (ctx->EFlags, 1) : REMOVE_BIT (ctx->EFlags, 1); break;
    case R_DF: v ? SET_BIT (ctx->EFlags, 1<<10) : REMOVE_BIT (ctx->EFlags, 1<<10); break;
    case R_TF: v ? SET_BIT (ctx->EFlags, 1<<8) : REMOVE_BIT (ctx->EFlags, 1<<8); break;
    default:
        oassert (0);
        break;
    };
};

void CONTEXT_set_reg_STx (CONTEXT * ctx, X86_register r, double v)
{
    int idx;

    switch (r)
    {
    case R_ST0: idx=0; break;
    case R_ST1: idx=1; break;
    case R_ST2: idx=2; break;
    case R_ST3: idx=3; break;
    case R_ST4: idx=4; break;
    case R_ST5: idx=5; break;
    case R_ST6: idx=6; break;
    case R_ST7: idx=7; break;
    default:
        oassert (0);
        break;
    };

    *(long double*)&get_XSAVE_FORMAT(ctx)->FloatRegisters[idx]=(long double)v;
};

void CONTEXT_clear_bp_in_DR7 (CONTEXT * ctx, int bp_n)
{
    REMOVE_BIT (ctx->Dr7, 1<<(bp_n*2));
};

void CONTEXT_setAccum (CONTEXT * ctx, REG v)
{
#ifdef _WIN64
    ctx->Rax=v;
#else
    ctx->Eax=v;
#endif
};

REG CONTEXT_getAccum (CONTEXT * ctx)
{
#ifdef _WIN64
    return ctx->Rax;
#else
    return ctx->Eax;
#endif
};

REG CONTEXT_get_reg (CONTEXT * ctx, X86_register r)
{
    switch (r)
    {
#ifdef _WIN64
    case R_RAX: return ctx->Rax;
    case R_RBX: return ctx->Rbx;
    case R_RCX: return ctx->Rcx;
    case R_RDX: return ctx->Rdx;
    case R_RSI: return ctx->Rsi;
    case R_RDI: return ctx->Rdi;
    case R_RSP: return ctx->Rsp;
    case R_RBP: return ctx->Rbp;
    case R_RIP: return ctx->Rip;
    case R_R8:  return ctx->R8;
    case R_R9:  return ctx->R9;
    case R_R10: return ctx->R10;
    case R_R11: return ctx->R11;
    case R_R12: return ctx->R12;
    case R_R13: return ctx->R13;
    case R_R14: return ctx->R14;
    case R_R15: return ctx->R15;
#else
    case R_EAX: return ctx->Eax;
    case R_EBX: return ctx->Ebx;
    case R_ECX: return ctx->Ecx;
    case R_EDX: return ctx->Edx;
    case R_ESI: return ctx->Esi;
    case R_EDI: return ctx->Edi;
    case R_EBP: return ctx->Ebp;
    case R_ESP: return ctx->Esp;
    case R_EIP: return ctx->Eip;

    case R_AL: return ctx->Eax&0xFF;
    case R_AH: return (ctx->Eax>>8)&0xFF;
    case R_AX: return ctx->Eax&0xFFFF;

    case R_BL: return ctx->Ebx&0xFF;
    case R_BH: return (ctx->Ebx>>8)&0xFF;
    case R_BX: return ctx->Ebx&0xFFFF;

    case R_CL: return ctx->Ecx&0xFF;
    case R_CH: return (ctx->Ecx>>8)&0xFF;
    case R_CX: return ctx->Ecx&0xFFFF;

    case R_DL: return ctx->Edx&0xFF;
    case R_DH: return (ctx->Edx>>8)&0xFF;
    case R_DX: return ctx->Edx&0xFFFF;

    case R_SI: return ctx->Esi&0xFFFF;

    case R_DI: return ctx->Edi&0xFFFF;

    case R_BP: return ctx->Ebp&0xFFFF;

    case R_ES: return ctx->SegEs;
    case R_DS: return ctx->SegDs;
    case R_FS: return ctx->SegFs;
    case R_GS: return ctx->SegGs;
    case R_CS: return ctx->SegCs;
    case R_SS: return ctx->SegSs;
#endif
    case R_ABSENT:
        oassert(0);
        break;

    default:
        oassert (!"register isn't implemented");
        break;
    };
};

address CONTEXT_calc_adr_of_op (CONTEXT * ctx, Da_op *op)
{
    REG adr=0;
    oassert (op->type==DA_OP_TYPE_VALUE_IN_MEMORY);
    oassert (op->adr.adr_index_mult!=0);

    if (op->adr.adr_base!=R_ABSENT)
        adr=adr+CONTEXT_get_reg (ctx, op->adr.adr_base);

    if (op->adr.adr_index!=R_ABSENT)
        adr=adr+CONTEXT_get_reg (ctx, op->adr.adr_index) * op->adr.adr_index_mult;

    return (address)(adr+op->adr.adr_disp); // negative values of adr_disp must work! (to be checked)
};

void CONTEXT_dump_DRx(fds *s, CONTEXT *ctx)
{
    L_fds (s, "DR0=" PRI_REG_HEX " DR1=" PRI_REG_HEX " DR2=" PRI_REG_HEX " DR3=" PRI_REG_HEX "\n", ctx->Dr0, ctx->Dr1, ctx->Dr2, ctx->Dr3);
    
    L_fds (s, "DR7=");
    dump_DR7(s, ctx->Dr7);
    L_fds (s, "\n");
};

void dump_FPU (fds* s, CONTEXT *ctx)
{
    strbuf sb_FCW=STRBUF_INIT;
    strbuf sb_FSW=STRBUF_INIT;
    unsigned r, i;
    XSAVE_FORMAT *x=get_XSAVE_FORMAT(ctx);
    
    if (x->TagWord==0)
        return;

    FCW_to_str(x->ControlWord, &sb_FCW);
    FSW_to_str(x->StatusWord, &sb_FSW);

    L_fds (s, "FPU ControlWord=%s\n", sb_FCW.buf);
    L_fds (s, "FPU StatusWord=%s\n", sb_FSW.buf);
    L_fds (s, "FPU TagWord=0x%x\n", x->TagWord);
    strbuf_deinit(&sb_FCW);
    strbuf_deinit(&sb_FSW);

    for (r=0; r<8; r++)
        if (STx_present_in_tag (ctx, r))
        {
            BYTE *b=(BYTE*)&x->FloatRegisters[r];
            double a=get_STx(ctx, r);

            if (_isnan (a)==0)
                L_fds (s, "FPU ST(%d): %lf\n", r, a);
            else
            {
                L_fds (s, "FPU ST(%d): %lf / MM%d=", r, a, r);
                for (i=0; i<8; i++)
                    L_fds (s, "%02X", *(b+(7-i)));
                L_fds (s, "\n");
            };
        };
};

const char *get_BP_register_name()
{
#ifdef _WIN64
    return "RBP";
#else
    return "EBP";
#endif
};

const char *get_AX_register_name()
{
#ifdef _WIN64
    return "RAX";
#else
    return "EAX";
#endif
};

const char *get_CX_register_name()
{
#ifdef _WIN64
    return "RCX";
#else
    return "ECX";
#endif
};

const char *get_DX_register_name()
{
#ifdef _WIN64
    return "RDX";
#else
    return "EDX";
#endif
};

/* vim: set expandtab ts=4 sw=4 : */
