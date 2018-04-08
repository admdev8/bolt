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
#include "disas_utils.h"
#include "dmalloc.h"
#include "lisp.h"
#include "X86_register_helpers.h"
#include "fmt_utils.h"

bool Da_op_get_value_of_op (struct Da_op *op, address * rt_adr, const CONTEXT * ctx, struct MemoryCache *mem, 
        const char *fname, unsigned fileline, obj *result, unsigned ins_prefixes, address FS)
{
    bool b;

    if (op->type==DA_OP_TYPE_REGISTER)
    {
        oassert (op->reg != R_ABSENT);
        // вытянуть из ctx
        X86_register_get_value (op->reg, ctx, result);
        return true;
    };

    if (op->type==DA_OP_TYPE_VALUE)
    {
        obj_copy2 (result, &op->val._v);
        return true;
    };

    if (op->type==DA_OP_TYPE_VALUE_IN_MEMORY)
    {
        *rt_adr=(REG)Da_op_calc_adr_of_op (op, ctx, mem, ins_prefixes, FS);

        switch (op->value_width_in_bits)
        {
        case 8:
            {
                BYTE out;
                b=MC_ReadByte (mem, *rt_adr, &out);
                if (b)
                {
                    obj_byte2((uint8_t)out, result);
                    return true;
                }
                else
                    return false;
            };
        case 16:
            {
                WORD out;
                b=MC_ReadWyde (mem, *rt_adr, &out);
                if (b)
                {
                    obj_wyde2((uint16_t)out, result);
                    return true;
                }
                else
                    return false;
            };
        case 32:
            {
                DWORD out;
                b=MC_ReadTetrabyte (mem, *rt_adr, &out);
                if (b)
                {
                    obj_tetra2((uint32_t)out, result);
                    return true;
                }
                else
                    return false;
            };
        case 64:
            {
                DWORD64 out;
                b=MC_ReadOctabyte (mem, *rt_adr, &out);
                if (b)
                {
                    obj_octa2((uint64_t)out, result);
                    return true;
                }
                else
                    return false;
            };
        case 128:
            {
                M128A xmm;
                if (MC_ReadBuffer (mem, *rt_adr, sizeof (M128A), (BYTE*)&xmm)==false)
                    return false;
                obj_xmm2 ((uint8_t*)&xmm, result);
                return true;
            };
        default:
            oassert(!"unknown value_width_in_bits");
            fatal_error();
            break;
        };

        oassert(0);
    };

#if 0
	L ("%s(): type=%d!\n", __FUNCTION__, type);
#endif
	oassert(0); // should not be here
    fatal_error();
};

bool Da_op_set_value_of_op (struct Da_op* op, obj *val, CONTEXT * ctx, struct MemoryCache *mem, unsigned ins_prefixes, address FS, bool clear_high_tetra_if_ExX) 
{
    address adr;

    switch (op->type)
    {
        case DA_OP_TYPE_REGISTER:
            //L ("%s() line %d clear_high_tetra_if_ExX=%d\n", __FUNCTION__, __LINE__, clear_high_tetra_if_ExX);
            X86_register_set_value (op->reg, ctx, val, clear_high_tetra_if_ExX);
            return true;

        case DA_OP_TYPE_VALUE_IN_MEMORY:
            adr=(address)Da_op_calc_adr_of_op(op, ctx, mem, ins_prefixes, FS);

            switch (op->value_width_in_bits)
            {
                case 8:
                    if (MC_WriteByte (mem, adr, obj_get_as_byte(val))==false)
                        goto COPY_FAILED;
                    return true;
                case 16:
                    if (MC_WriteWyde (mem, adr, obj_get_as_wyde(val))==false)
                        goto COPY_FAILED;
                    return true;
                case 32:
                    if (MC_WriteTetrabyte (mem, adr, obj_get_as_tetra(val))==false)
                        goto COPY_FAILED;
                    return true;
                case 64:
                    if (MC_WriteOctabyte (mem, adr, obj_get_as_octa(val))==false)
                        goto COPY_FAILED;
                    return true;
                case 128:
                    {
                        //val.dump();
                        byte * xmm=obj_get_as_xmm(val);
                        //L ("%s(). writing to adr=0x%x\n", __FUNCTION__, adr);
                        //L_print_buf (xmm, 16);
                        if (MC_WriteBuffer (mem, adr, 16, xmm)==false)
                            goto COPY_FAILED;
                        return true;
                    }

                default:
                    oassert(!"unsupported value_width_in_bits");
                    fatal_error();
            };

        default:
            oassert(!"unsupported type");
            fatal_error();
    };
COPY_FAILED:
    L ("%s(): Error writing at 0x" PRI_ADR_HEX ". Copy failed.\n", __FUNCTION__, adr);
    return false;
};

/*
#if __WORDSIZE==64
#define SREG int64_t
#elif __WORDSIZE==32
#define SREG int32_t
#else
#error "stop"
#endif
*/
address Da_op_calc_adr_of_op (struct Da_op* op, const CONTEXT * ctx, struct MemoryCache *mem, unsigned ins_prefixes, address FS)
{
    address adr=0;
    address rt;

    oassert (op->type==DA_OP_TYPE_VALUE_IN_MEMORY);
    oassert (op->adr.adr_index_mult!=0);

    if (op->adr.adr_base != R_ABSENT)
        adr=adr+X86_register_get_value_as_u64 (op->adr.adr_base, ctx);

    L ("%s() line=%d adr=0x" PRI_REG_HEX "\n", __func__, __LINE__, adr);
    
    if (op->adr.adr_index != R_ABSENT)
        adr=adr+X86_register_get_value_as_u64(op->adr.adr_index, ctx) * op->adr.adr_index_mult;

    L ("%s() line=%d adr=0x" PRI_REG_HEX "\n", __func__, __LINE__, adr);

    L ("%s() line=%d op->adr.adr_disp=0x" PRI_REG_HEX "\n", __func__, __LINE__, op->adr.adr_disp);
/*
    if (__WORDSIZE==64 && op->value_width_in_bits==32)
    {
        rt=adr+(int64_t)(int32_t)op->adr.adr_disp; // negative values of adr_disp must work! (to be checked)
        L ("%s() line=%d (int64_t)(int32_t)op->adr.adr_disp=0x" PRI_REG_HEX "\n", __func__, __LINE__, (int64_t)(int32_t)op->adr.adr_disp);
    }
    else
*/
        rt=adr+op->adr.adr_disp; // negative values of adr_disp must work! (to be checked)

    L ("%s() line=%d rt=0x" PRI_REG_HEX "\n", __func__, __LINE__, rt);

    if (IS_SET(ins_prefixes, PREFIX_FS))
        rt+=FS;
    
    return rt;
};

/* vim: set expandtab ts=4 sw=4 : */
