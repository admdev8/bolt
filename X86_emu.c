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
#include "dmalloc.h"
#include "lisp.h"
#include "x86.h"
#include "x86_disas.h"
#include "x86_emu.h"
#include "bitfields.h"
#include "disas_utils.h"

bool x86_emu_debug=/*true*/ false;

int parity_lookup[256] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

void set_AF (CONTEXT * ctx, obj* rt1, obj* rt2, obj* res)
{
    set_or_clear_flag (ctx, FLAG_AF, (obj_get_4th_bit(rt1) ^ obj_get_4th_bit(rt2)) ^ obj_get_4th_bit(res));
};

void set_ZF (CONTEXT * ctx, obj* res)
{
    set_or_clear_flag (ctx, FLAG_ZF, obj_is_zero(res));
};

octabyte get_sign_bit (int value_width_in_bits)
{
    switch (value_width_in_bits)
    {
        case 64: return 0x8000000000000000;
        case 32: return 0x80000000;
        case 16: return 0x8000;
        case 8: return 0x80;
        default:
                oassert(0);
    };
};

octabyte get_mask (int value_width_in_bits)
{
    switch (value_width_in_bits)
    {
        case 64: return 0xFFFFFFFFFFFFFFFF;
        case 32: return 0xFFFFFFFF;
        case 16: return 0xFFFF;
        case 8: return 0xFF;
        default:
                oassert(0);
    };
};

static enum obj_type bit_width_to_obj_type (unsigned width_in_bits)
{
    switch (width_in_bits)
    {
        case 8: 
            return OBJ_BYTE;
        case 16:
            return OBJ_WYDE;
        case 32:
            return OBJ_TETRABYTE;
        case 64:
            return OBJ_OCTABYTE;
        default:
            oassert(0);
    };
};

void set_SF (CONTEXT * ctx, obj* res)
{
    set_or_clear_flag (ctx, FLAG_SF, get_most_significant_bit(res));
};

void set_PF (CONTEXT * ctx, obj* res)
{
    set_or_clear_flag (ctx, FLAG_PF, parity_lookup[get_lowest_byte(res)]);
};

bool DO_PUSH (CONTEXT * ctx, MemoryCache *mem, REG val)
{
    REG new_SP=CONTEXT_get_SP(ctx)-sizeof(REG);
    bool b=MC_WriteREG (mem, new_SP, val);
    if (b==false)
        return false;
    CONTEXT_set_SP(ctx, new_SP);
    return true;
};

bool DO_POP (CONTEXT * ctx, MemoryCache *mem, REG *outval)
{
    bool b=MC_ReadREG (mem, CONTEXT_get_SP(ctx), outval);
    if (b==false)
        return false;
    REG new_SP=CONTEXT_get_SP(ctx)+sizeof(REG);
    CONTEXT_set_SP(ctx, new_SP);
    return true;
};

Da_emulate_result Da_emulate_MOV_op1_op2(Da* d, CONTEXT * ctx, MemoryCache *mem, unsigned ins_prefixes, address FS)
{
    obj tmp; // we can't allocate it dynamically, it's high performance code after all!
    tmp.t=OBJ_NONE;
    address rt_adr;
    bool b;
    Da_emulate_result r;

    //L (2, __FUNCTION__ "() begin\n");

    b=Da_op_get_value_of_op(&d->op[1], &rt_adr, ctx, mem, __FILE__, __LINE__, &tmp, d->prefix_codes, FS);
    if (b==false)
    {
        /*
           if (L_verbose_level>=2)
           {
        // FIXME: should write to log also
        printf (__FUNCTION__"(): [");
        Da_DumpString(d);
        printf ("]: can't read src (2nd) operand\n");
        };
        */
        r=DA_EMULATED_CANNOT_READ_MEMORY;
        goto exit;
    };

    b=Da_op_set_value_of_op (&d->op[0], &tmp, ctx, mem, d->prefix_codes, FS);

    if (b==false)
    {
        /*
           if (L_verbose_level>=2)
           {
        // FIXME: should write to log also
        printf (__FUNCTION__"(): [");
        Da_DumpString(d);
        printf ("]: can't write dst (1nd) operand\n");
        };
        */
        r=DA_EMULATED_CANNOT_WRITE_MEMORY;
        goto exit;
    };
    CONTEXT_add_to_PC(ctx, d->ins_len);
    //IF_VERBOSE (2, cout << __FUNCTION__ << "() succ end. new PC: 0x" << hex << CONTEXT_get_PC(ctx) << endl; );
    r=DA_EMULATED_OK;
exit:    
    obj_free_structures(&tmp);
    return r;
};

Da_emulate_result Da_emulate_Jcc (Da* d, bool cond, CONTEXT * ctx)
{
    if (cond)
        CONTEXT_set_PC(ctx, obj_get_as_REG(&d->op[0].val._v));
    else
        CONTEXT_add_to_PC(ctx, d->ins_len);
    return DA_EMULATED_OK;
};

Da_emulate_result Da_emulate_CMOVcc (Da* d, bool cond, CONTEXT * ctx, MemoryCache *mem, unsigned ins_prefixes, address FS)
{
    if (cond)
        return Da_emulate_MOV_op1_op2(d, ctx, mem, ins_prefixes, FS);
    else
    {
        CONTEXT_add_to_PC(ctx, d->ins_len);
        return DA_EMULATED_OK;
    };
};

Da_emulate_result Da_emulate_SETcc (Da* d, bool cond, CONTEXT * ctx, MemoryCache *mem, unsigned ins_prefixes, address FS)
{
    obj dst;

    obj_byte2 (cond ? 1 : 0, &dst);

    bool b=Da_op_set_value_of_op (&d->op[0], &dst, ctx, mem, d->prefix_codes, FS);
    if (b==false)
        return DA_EMULATED_CANNOT_WRITE_MEMORY;

    CONTEXT_add_to_PC(ctx, d->ins_len);
    return DA_EMULATED_OK;
};

bool ins_traced_by_one_step(Ins_codes i)
{
    switch (i)
    {
        case I_REP_STOSB:
        case I_REP_STOSW:
        case I_REP_STOSD:
        case I_REP_MOVSB:
        case I_REP_MOVSW:
        case I_REP_MOVSD:
            return true;
        default:
            return false;
    };
};

Da_emulate_result Da_emulate(Da* d, CONTEXT * ctx, MemoryCache *mem, bool emulate_FS_accesses, address FS)
{
#ifdef _WIN64
    return DA_NOT_EMULATED;
#endif        
    //bool SF=IS_SET(ctx->EFlags, FLAG_SF);
    //bool OF=IS_SET(ctx->EFlags, FLAG_OF);
    //bool ZF=IS_SET(ctx->EFlags, FLAG_ZF);
    bool CF=IS_SET(ctx->EFlags, FLAG_CF);
    bool b;

    if (x86_emu_debug)
    {
        // FIXME: write to log also?
        L ("%s() begin: [", __func__);
        Da_DumpString(&cur_fds, d);
        L ("]\n");
    };

    if ((emulate_FS_accesses==false && IS_SET(d->prefix_codes, PREFIX_FS)) ||
            (IS_SET(d->prefix_codes, PREFIX_SS) || IS_SET(d->prefix_codes, PREFIX_GS)))
    {
        if (x86_emu_debug)
            L ("%s() skipping (data selector prefix present) at 0x" PRI_ADR_HEX "\n", __func__, CONTEXT_get_PC(ctx));
        return DA_EMULATED_CANNOT_READ_MEMORY;
    };

    switch (d->ins_code)
    {
        case I_CDQ:
            {
                uint32_t a=CONTEXT_get_Accum (ctx)&0xFFFFFFFF;
                CONTEXT_set_xDX (ctx, (a&0x80000000) ? 0xFFFFFFFF : 0);
                goto add_to_PC_and_return_OK;
            };
            break;

        case I_PUSH:
            {
                address rt_adr;
                obj v;
                b=Da_op_get_value_of_op (&d->op[0], &rt_adr, ctx, mem, __FILE__, __LINE__, &v, d->prefix_codes, FS);
                if (b==false)
                {
                    if (x86_emu_debug)
                        L ("%s() I_PUSH: can't read memory\n", __func__);
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                };
                b=DO_PUSH (ctx, mem, obj_get_as_REG(&v));
                if (b==false)
                {
                    if (x86_emu_debug)
                        L ("%s() I_PUSH: can't write memory\n", __func__);
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
                };
                goto add_to_PC_and_return_OK;
            };
            break;

        case I_PUSHFD:
            if (DO_PUSH (ctx, mem, ctx->EFlags | FLAG_TF)==false)
            {
                if (x86_emu_debug)
                    L ("%s() I_PUSHFD: can't write memory\n", __func__);
                return DA_EMULATED_CANNOT_WRITE_MEMORY;
            };
            goto add_to_PC_and_return_OK;
            break;

        case I_POP:
            {
                REG val;
                if (DO_POP(ctx, mem, &val)==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                obj v;
                obj_tetrabyte2 (val, &v);
                Da_op_set_value_of_op (&d->op[0], &v, ctx, mem, d->prefix_codes, FS);
                goto add_to_PC_and_return_OK;
            };
            break;

        case I_POPFD:
            {
                REG val;
                if (DO_POP(ctx, mem, &val)==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                ctx->EFlags=val;
                goto add_to_PC_and_return_OK;
            };
            break;
        
        case I_LEAVE:
            {
                //ESP <- EBP
                //POP EBP

                REG val;
                CONTEXT_set_SP(ctx, CONTEXT_get_BP(ctx));
                if (DO_POP(ctx, mem, &val)==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY; // FIXME: а надо еще SP назад возвращать!
                CONTEXT_set_BP(ctx, val);
                goto add_to_PC_and_return_OK;
            };
            break;

        case I_REP_STOSB:
        case I_REP_STOSW:
        case I_REP_STOSD:
            {
                BYTE *buf;
                bool DF=IS_SET(ctx->EFlags, FLAG_DF);
                if (DF)
                    return DA_NOT_EMULATED; // not supported... bug here

                SIZE_T BUF_SIZE;

                if (d->ins_code==I_REP_STOSB)
                    BUF_SIZE=CONTEXT_get_xCX (ctx);
                else if (d->ins_code==I_REP_STOSW)
                    BUF_SIZE=CONTEXT_get_xCX (ctx)*2;
                else if (d->ins_code==I_REP_STOSD)
                    BUF_SIZE=CONTEXT_get_xCX (ctx)*4;

                buf=DMALLOC(BYTE, BUF_SIZE, "buf");

                if (d->ins_code==I_REP_STOSB)
                {
                    for (REG i=0; i<CONTEXT_get_xCX(ctx); i++) // FIXME: rewrite to my own bzero()!
                        buf[i]=CONTEXT_get_Accum(ctx)&0xFF;
                } 
                else if (d->ins_code==I_REP_STOSW)
                {
                    for (REG i=0; i<CONTEXT_get_xCX(ctx); i++) // FIXME: rewrite to my own bzero()!
                        ((WORD*)buf)[i]=CONTEXT_get_Accum(ctx)&0xFFFF;
                } 
                else if (d->ins_code==I_REP_STOSD)
                {
                    for (REG i=0; i<CONTEXT_get_xCX(ctx); i++) // FIXME: rewrite to my own bzero()!
                        ((DWORD*)buf)[i]=(DWORD)(CONTEXT_get_Accum(ctx)&0xFFFFFFFF);
                } 
                else
                {
                    oassert(0);
                };

                if (MC_WriteBuffer (mem, CONTEXT_get_xDI (ctx), BUF_SIZE, buf)==false)
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
                DFREE(buf);
                CONTEXT_set_xDI (ctx, CONTEXT_get_xDI (ctx) + BUF_SIZE);
                CONTEXT_set_xCX (ctx, 0);
                goto add_to_PC_and_return_OK;
            };
            break;

        case I_REP_MOVSB:
        case I_REP_MOVSW:
        case I_REP_MOVSD:
            {
                BYTE *buf;
                bool DF=IS_SET(ctx->EFlags, FLAG_DF);
                if (DF)
                    return DA_NOT_EMULATED; // not supported... bug here

                SIZE_T BUF_SIZE;
                SIZE_T sizeof_element;

                if (d->ins_code==I_REP_MOVSB)
                    sizeof_element=1;
                else if (d->ins_code==I_REP_MOVSW)
                    sizeof_element=2;
                else if (d->ins_code==I_REP_MOVSD)
                    sizeof_element=4;
                else
                {
                    oassert(0);
                };

                BUF_SIZE=CONTEXT_get_xCX(ctx)*sizeof_element;

                //printf ("%s() BUF_SIZE=0x%x\n", __func__, BUF_SIZE);
                //printf ("%s() (before) SI=0x" PRI_REG_HEX "\n", __func__, CONTEXT_get_xSI(ctx));
                //printf ("%s() (before) DI=0x" PRI_REG_HEX "\n", __func__, CONTEXT_get_xDI(ctx));

                buf=DMALLOC(BYTE, BUF_SIZE, "buf");

                address blk_src=CONTEXT_get_xSI(ctx);
                address blk_dst=CONTEXT_get_xDI(ctx);

                if (DF)
                {
                    blk_src-=BUF_SIZE; // +sizeof_element;
                    blk_dst-=BUF_SIZE; // +sizeof_element;
                };

                if (MC_ReadBuffer (mem, blk_src, BUF_SIZE, buf)==false)
                {
                    DFREE(buf);
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                };

                if (MC_WriteBuffer (mem, blk_dst, BUF_SIZE, buf)==false)
                {
                    DFREE(buf);
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
                };

                DFREE(buf);
                if (DF==false)
                {
                    CONTEXT_set_xSI (ctx, CONTEXT_get_xSI (ctx) + BUF_SIZE);
                    CONTEXT_set_xDI (ctx, CONTEXT_get_xDI (ctx) + BUF_SIZE);
                }
                else
                {
                    CONTEXT_set_xSI (ctx, CONTEXT_get_xSI (ctx) - BUF_SIZE);
                    CONTEXT_set_xDI (ctx, CONTEXT_get_xDI (ctx) - BUF_SIZE);
                };
                CONTEXT_set_xCX (ctx, 0);

                //printf ("%s() (after) SI=0x" PRI_REG_HEX "\n", __func__, CONTEXT_get_xSI(ctx));
                //printf ("%s() (after) DI=0x" PRI_REG_HEX "\n", __func__, CONTEXT_get_xDI(ctx));

                goto add_to_PC_and_return_OK;
            };

        case I_STD:
            SET_BIT (ctx->EFlags, FLAG_DF);
            goto add_to_PC_and_return_OK;

        case I_CLD:
            REMOVE_BIT (ctx->EFlags, FLAG_DF);
            goto add_to_PC_and_return_OK;

        case I_RETN:
            {
                WORD ret_arg=0;
                if (d->ops_total==1)
                {
                    oassert (d->op[0].type==DA_OP_TYPE_VALUE);
                    ret_arg=obj_get_as_wyde(&d->op[0].val._v); // RETN arg is 16-bit
                };

                address newPC;
                if (DO_POP(ctx, mem, &newPC)==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                CONTEXT_set_SP(ctx, CONTEXT_get_SP(ctx)+ret_arg);
                CONTEXT_set_PC(ctx, newPC);
                return DA_EMULATED_OK;
            };
            break;

        case I_ADD:
        case I_ADC:
        case I_INC:
            {
                //L (__FUNCTION__ "() I_ADD/I_INC begin: [%s]\n", ToString().c_str());

                obj rt1, rt2;
                REG rt1_adr, rt2_adr;
                b=Da_op_get_value_of_op (&d->op[0], &rt1_adr, ctx, mem, __FILE__, __LINE__, &rt1, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                if (d->ins_code==I_ADD || d->ins_code==I_ADC)
                {
                    oassert (d->op[0].value_width_in_bits==d->op[1].value_width_in_bits);
                    b=Da_op_get_value_of_op (&d->op[1], &rt2_adr, ctx, mem, __FILE__, __LINE__, &rt2, d->prefix_codes, FS);
                    if (b==false)
                        return DA_EMULATED_CANNOT_READ_MEMORY;
                }
                else
                {   // INC case
                    // make second op 1
                    obj_REG2_and_set_type (rt1.t, 1, 0, &rt2);
                };

                obj res_sum;
                obj_add (&rt1, &rt2, &res_sum);
                if (d->ins_code==I_ADC && CF)
                    obj_increment(&res_sum); 

                set_PF (ctx, &res_sum);
                set_SF (ctx, &res_sum);
                set_ZF (ctx, &res_sum);
                set_AF (ctx, &rt1, &rt2, &res_sum);
                if (d->ins_code==I_ADD || d->ins_code==I_ADC)
                    set_or_clear_flag (ctx, FLAG_CF, obj_compare (&res_sum, &rt1)==-1); // res_sum < rt1

                octabyte tmp=((zero_extend_to_REG(&rt1) ^ zero_extend_to_REG(&rt2) ^ 
                            get_sign_bit (d->op[0].value_width_in_bits)) & 
                        (zero_extend_to_REG(&res_sum) ^ zero_extend_to_REG(&rt1))) 
                    & 
                    get_sign_bit (d->op[0].value_width_in_bits);
                set_or_clear_flag (ctx, FLAG_OF, tmp);

                b=Da_op_set_value_of_op (&d->op[0], &res_sum, ctx, mem, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
                goto add_to_PC_and_return_OK;
            };
            break;

        case I_NOT:
            {
                obj rt1, res;
                REG rt1_adr;
                if (Da_op_get_value_of_op (&d->op[0], &rt1_adr, ctx, mem, __FILE__, __LINE__, &rt1, d->prefix_codes, FS)==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;

                obj_NOT(&rt1, &res);

                if (Da_op_set_value_of_op (&d->op[0], &res, ctx, mem, d->prefix_codes, FS))
                    goto add_to_PC_and_return_OK;
            };
            break;

        case I_NEG:
            {
                obj rt1, res;
                REG rt1_adr;
                if (Da_op_get_value_of_op (&d->op[0], &rt1_adr, ctx, mem, __FILE__, __LINE__, &rt1, d->prefix_codes, FS)==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;

                obj_NEG(&rt1, &res);
                set_or_clear_flag (ctx, FLAG_CF, obj_is_zero(&rt1)==false);

                set_PF (ctx, &res);
                set_SF (ctx, &res);
                set_ZF (ctx, &res);
                set_or_clear_flag (ctx, FLAG_AF, (0 ^ obj_get_4th_bit(&rt1)) ^ obj_get_4th_bit(&res));
                REMOVE_BIT (ctx->EFlags, FLAG_OF);
                //SET_BIT (ctx->EFlags, FLAG_AF);

                if (Da_op_set_value_of_op (&d->op[0], &res, ctx, mem, d->prefix_codes, FS))
                    goto add_to_PC_and_return_OK;
            };
            break;


        case I_OR:
        case I_XOR:
        case I_AND:
        case I_TEST:
            {
                oassert (d->op[0].value_width_in_bits==d->op[1].value_width_in_bits);
                obj rt1, rt2, res;
                REG rt1_adr, rt2_adr;
                b=Da_op_get_value_of_op (&d->op[0], &rt1_adr, ctx, mem, __FILE__, __LINE__, &rt1, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                b=Da_op_get_value_of_op (&d->op[1], &rt2_adr, ctx, mem, __FILE__, __LINE__, &rt2, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;

                switch (d->ins_code)
                {
                    case I_OR:  
                        obj_OR(&rt1, &rt2, &res); 
                        break;
                    case I_XOR: 
                        obj_XOR(&rt1, &rt2, &res); 
                        break;
                    case I_TEST: 
                    case I_AND: 
                        obj_AND(&rt1, &rt2, &res); 
                        break;
                    default: 
                        oassert(0);
                        break;
                };

                set_PF (ctx, &res);
                set_SF (ctx, &res);
                set_ZF (ctx, &res);
                REMOVE_BIT (ctx->EFlags, FLAG_AF);
                REMOVE_BIT (ctx->EFlags, FLAG_CF);
                REMOVE_BIT (ctx->EFlags, FLAG_OF);

                if (d->ins_code==I_TEST)
                    b=true;
                else
                    b=Da_op_set_value_of_op (&d->op[0], &res, ctx, mem, d->prefix_codes, FS);

                if (b)
                    goto add_to_PC_and_return_OK;
                else
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
            };
            break;

        case I_DEC:
        case I_SUB:
        case I_SBB:
        case I_CMP:
            {
                if (d->ins_code==I_SUB || d->ins_code==I_SBB || d->ins_code==I_CMP)
                {
                    oassert (d->op[0].value_width_in_bits==d->op[1].value_width_in_bits);
                };
                obj rt1, rt2;
                REG rt1_adr, rt2_adr;
                b=Da_op_get_value_of_op (&d->op[0], &rt1_adr, ctx, mem, __FILE__, __LINE__, &rt1, d->prefix_codes, FS);
                if (b==false) 
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                if (d->ins_code==I_DEC)
                {
                    // make second op 1
                    obj_REG2_and_set_type (rt1.t, 1, 0, &rt2);
                }
                else
                {
                    b=Da_op_get_value_of_op (&d->op[1], &rt2_adr, ctx, mem, __FILE__, __LINE__, &rt2, d->prefix_codes, FS);
                    if (b==false)
                        return DA_EMULATED_CANNOT_READ_MEMORY;
                };

                obj res;
                obj_subtract (&rt1, &rt2, &res);
                if (d->ins_code==I_SBB && CF)
                    obj_decrement (&res);

                set_PF (ctx, &res);
                set_SF (ctx, &res);
                set_ZF (ctx, &res);
                set_AF (ctx, &rt1, &rt2, &res);

                if (d->ins_code==I_SBB)
                {
                    int tmp=(obj_compare (&rt1, &res)==-1 /* rt1<res */) || (CF && obj_get_as_tetrabyte(&rt2)==0xffffffff);
                    set_or_clear_flag (ctx, FLAG_CF, tmp);
                }
                else
                {
                    if (d->ins_code!=I_DEC) // DEC leave CF flag unaffected
                        set_or_clear_flag (ctx, FLAG_CF, obj_compare (&rt1, &rt2)==-1); /* rt1<rt2 */
                };

                octabyte tmp=((zero_extend_to_octabyte(&rt1) ^ zero_extend_to_octabyte(&rt2)) & 
                        (zero_extend_to_octabyte(&res) ^ zero_extend_to_octabyte(&rt1))) &
                    get_sign_bit (d->op[0].value_width_in_bits);
                set_or_clear_flag (ctx, FLAG_OF, tmp);

                if (d->ins_code==I_CMP)
                    b=true;
                else
                    b=Da_op_set_value_of_op (&d->op[0], &res, ctx, mem, d->prefix_codes, FS);

                if (b)
                    goto add_to_PC_and_return_OK;
                else
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
            };
            break;

        case I_XCHG:
            {
                REG op1_adr, op2_adr;

                obj op1;
                b=Da_op_get_value_of_op(&d->op[0], &op1_adr, ctx, mem, __FILE__, __LINE__, &op1, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;

                obj op2;
                b=Da_op_get_value_of_op(&d->op[1], &op2_adr, ctx, mem, __FILE__, __LINE__, &op2, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;

                if (Da_op_set_value_of_op (&d->op[0], &op2, ctx, mem, d->prefix_codes, FS)==false)
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;

                if (Da_op_set_value_of_op (&d->op[1], &op1, ctx, mem, d->prefix_codes, FS)==false)
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;

                goto add_to_PC_and_return_OK;
            };
            break;

        case I_MOV:
        case I_MOVDQA:
        case I_MOVDQU:
            return Da_emulate_MOV_op1_op2(d, ctx, mem, d->prefix_codes, FS);

        case I_MOVZX:
        case I_MOVSX:
            {
                address rt_adr;
                obj op2;
                bool b=Da_op_get_value_of_op(&d->op[1], &rt_adr, ctx, mem, __FILE__, __LINE__, &op2, d->prefix_codes, FS);
                if (b==false)
                {
                    /*
                       if (L_verbose_level>=2)
                       {
                       printf (__FUNCTION__ "(): [");
                       Da_DumpString(d);
                       printf ("]: can't read src (2nd) operand\n");
                       };
                       */
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
                };

                obj to_be_stored_v;

                if (d->ins_code==I_MOVZX)
                {
                    enum obj_type op1_type_will_be=bit_width_to_obj_type (d->op[0].value_width_in_bits);
                    obj_zero_extend (&op2, op1_type_will_be, &to_be_stored_v);
                }
                else if (d->ins_code==I_MOVSX)
                {
                    enum obj_type op1_type_will_be=bit_width_to_obj_type (d->op[0].value_width_in_bits);
                    obj_sign_extend (&op2, op1_type_will_be, &to_be_stored_v);
                }
                else
                {
                    oassert (0);
                };

                b=Da_op_set_value_of_op (&d->op[0], &to_be_stored_v, ctx, mem, d->prefix_codes, FS);

                if (b==false)
                {
                    /*
                       if (L_verbose_level>=2)
                       {
                       printf(__FUNCTION__ "(): [");
                       Da_DumpString(d);
                       printf ("]: can't write dst (1st) operand\n");
                       };
                       */
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
                };
                goto add_to_PC_and_return_OK;
            };

        case I_NOP:
            goto add_to_PC_and_return_OK;

        case I_LEA:
            {
                address a=(address)Da_op_calc_adr_of_op(&d->op[1], ctx, mem, d->prefix_codes, FS);
                obj val;
                obj_tetrabyte2(a, &val);
                b=Da_op_set_value_of_op (&d->op[0], &val, ctx, mem, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
                goto add_to_PC_and_return_OK;
            };

        case I_SAR:
        case I_SHR:
        case I_SHL:
            {
                // http://cs.smith.edu/~thiebaut/ArtOfAssembly/CH06/CH06-3.html

                REG op1_adr, op2_adr;

                obj op1;
                bool b=Da_op_get_value_of_op(&d->op[0], &op1_adr, ctx, mem, __FILE__, __LINE__, &op1, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;

                // should be read anyway!
                obj op2;
                b=Da_op_get_value_of_op(&d->op[1], &op2_adr, ctx, mem, __FILE__, __LINE__, &op2, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;
#ifdef _WIN64 
                obj_AND_with (&op2, 0x3F);
#else
                obj_AND_with (&op2, 0x1F);
#endif
                if (obj_is_zero(&op2)==false)
                {
                    unsigned new_CF;

                    if (d->ins_code==I_SHR || d->ins_code==I_SAR)
                        new_CF=(zero_extend_to_octabyte(&op1) >> (obj_get_as_byte (&op2) - 1)) & 1;
                    else // SHL
                        new_CF=(zero_extend_to_octabyte(&op1) >> (obj_width_in_bits(&op1) - obj_get_as_byte (&op2))) & 1;

                    obj new_op1;

                    if (d->ins_code==I_SHR)
                    {
                        REG new_v=zero_extend_to_REG(&op1) >> obj_get_as_byte (&op2);
                        obj_REG2_and_set_type(op1.t, new_v, 0, &new_op1);
                    }
                    else if (d->ins_code==I_SAR)
                        obj2_sign_extended_shift_right (&op1, obj_get_as_byte(&op2), &new_op1);
                    else // SHL
                    {
                        REG new_v=zero_extend_to_REG(&op1) << obj_get_as_byte (&op2);
                        obj_REG2_and_set_type(op1.t, new_v, 0, &new_op1);
                    };

                    if (Da_op_set_value_of_op (&d->op[0], &new_op1, ctx, mem, d->prefix_codes, FS)==false)
                        return DA_EMULATED_CANNOT_WRITE_MEMORY;

                    set_PF (ctx, &new_op1);
                    set_SF (ctx, &new_op1);
                    set_ZF (ctx, &new_op1);

                    REMOVE_BIT (ctx->EFlags, FLAG_AF);

                    set_or_clear_flag (ctx, FLAG_CF, new_CF);

                    if (d->ins_code==I_SHR)
                    {
                        set_or_clear_flag (ctx, FLAG_OF, get_most_significant_bit(&op1));
                    }
                    else if (d->ins_code==I_SAR)
                    {
                        REMOVE_BIT (ctx->EFlags, FLAG_OF);
                    }
                    else
                    { // SHL
                        set_or_clear_flag (ctx, FLAG_OF, 
                                get_most_significant_bit(&op1) != get_2nd_most_significant_bit(&op1));
                    };
                };

                goto add_to_PC_and_return_OK;
            };
            break;

            // conditional instructions
        case I_JZ:  
            return Da_emulate_Jcc (d, is_Z_cond(ctx), ctx);
        case I_JBE: 
            return Da_emulate_Jcc (d, is_BE_cond(ctx), ctx);
        case I_JA:  
            return Da_emulate_Jcc (d, is_A_cond(ctx), ctx);
        case I_JLE: 
            return Da_emulate_Jcc (d, is_LE_cond(ctx), ctx);
        case I_JL:  
            return Da_emulate_Jcc (d, is_L_cond(ctx), ctx);
        case I_JNZ: 
            return Da_emulate_Jcc (d, is_NZ_cond(ctx), ctx); // AKA JNE
        case I_JS:  
            return Da_emulate_Jcc (d, is_S_cond(ctx), ctx);
        case I_JB:  
            return Da_emulate_Jcc (d, is_B_cond(ctx), ctx);  // aka JC
        case I_JNB: 
            return Da_emulate_Jcc (d, is_NB_cond(ctx), ctx); // aka JNC
        case I_JNS: 
            return Da_emulate_Jcc (d, is_NS_cond(ctx), ctx);
        case I_JGE: 
            return Da_emulate_Jcc (d, is_GE_cond(ctx), ctx);
        case I_JG:  
            return Da_emulate_Jcc (d, is_G_cond(ctx), ctx);

        case I_CMOVZ:  
            return Da_emulate_CMOVcc (d, is_Z_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVNZ: 
            return Da_emulate_CMOVcc (d, is_NZ_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVBE: 
            return Da_emulate_CMOVcc (d, is_BE_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVG:  
            return Da_emulate_CMOVcc (d, is_G_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVGE: 
            return Da_emulate_CMOVcc (d, is_GE_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVNS: 
            return Da_emulate_CMOVcc (d, is_NS_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVS:  
            return Da_emulate_CMOVcc (d, is_S_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVL:  
            return Da_emulate_CMOVcc (d, is_L_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVLE: 
            return Da_emulate_CMOVcc (d, is_LE_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVA:  
            return Da_emulate_CMOVcc (d, is_A_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVB:  
            return Da_emulate_CMOVcc (d, is_B_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_CMOVNB: 
            return Da_emulate_CMOVcc (d, is_NB_cond(ctx), ctx, mem, d->prefix_codes, FS);

        case I_SETE:  
            return Da_emulate_SETcc (d, is_Z_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETNE: 
            return Da_emulate_SETcc (d, is_NZ_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETNB: 
            return Da_emulate_SETcc (d, is_NB_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETO:  
            return Da_emulate_SETcc (d, is_O_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETA:  
            return Da_emulate_SETcc (d, is_A_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETB:  
            return Da_emulate_SETcc (d, is_B_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETBE: 
            return Da_emulate_SETcc (d, is_BE_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETGE: 
            return Da_emulate_SETcc (d, is_GE_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETG:  
            return Da_emulate_SETcc (d, is_G_cond(ctx), ctx, mem, d->prefix_codes, FS);
        case I_SETL:  
            return Da_emulate_SETcc (d, is_L_cond(ctx), ctx, mem, d->prefix_codes, FS);

        case I_JMP:
            {
                address rt_adr;
                obj rt;
                bool b=Da_op_get_value_of_op(&d->op[0], &rt_adr, ctx, mem, __FILE__, __LINE__, &rt, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                CONTEXT_set_PC(ctx, obj_get_as_REG(&rt));
                return DA_EMULATED_OK;
            };

        case I_CALL:
            {
                address rt_adr;
                obj rt;
                bool b=Da_op_get_value_of_op(&d->op[0], &rt_adr, ctx, mem, __FILE__, __LINE__, &rt, d->prefix_codes, FS);
                if (b==false)
                    return DA_EMULATED_CANNOT_READ_MEMORY;
                if (DO_PUSH(ctx, mem, CONTEXT_get_PC(ctx)+d->ins_len)==false)
                    return DA_EMULATED_CANNOT_WRITE_MEMORY;
                CONTEXT_set_PC(ctx, obj_get_as_REG(&rt));
                return DA_EMULATED_OK;
            };

        default:
            if (x86_emu_debug)
            {
                L ("%s() ins not emulated: [", __func__);
                Da_DumpString(&cur_fds, d);
                L ("]\n");
            };
            return DA_EMULATED_NOT_SUPPORTED;
    };

add_to_PC_and_return_OK:    
    CONTEXT_add_to_PC(ctx, d->ins_len);
    return DA_EMULATED_OK;
};

const char* Da_emulate_result_to_string(Da_emulate_result r)
{
    switch (r)
    {
    case DA_NOT_EMULATED: 
        return "DA_NOT_EMULATED";
    case DA_EMULATED_OK: 
        return "DA_EMULATED_OK";
    case DA_EMULATED_CANNOT_READ_MEMORY:
        return "DA_EMULATED_CANNOT_READ_MEMORY";
    case DA_EMULATED_CANNOT_WRITE_MEMORY:
        return "DA_EMULATED_CANNOT_WRITE_MEMORY";
    case DA_EMULATED_NOT_SUPPORTED:
        return "DA_EMULATED_NOT_SUPPORTED";
    default:
        oassert(0);
    };
};

/* vim: set expandtab ts=4 sw=4 : */
