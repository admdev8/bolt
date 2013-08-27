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

bool Da_op_get_value_of_op (Da_op *op, address * rt_adr, const CONTEXT * ctx, MemoryCache *mem, 
        const char *fname, unsigned fileline, obj *result)
{
    bool b;

    if (op->type==DA_OP_TYPE_REGISTER)
    {
        oassert (op->u.reg != R_ABSENT);
        // вытянуть из ctx
        X86_register_get_value (op->u.reg, ctx, result);
        return true;
    };

    if (op->type==DA_OP_TYPE_VALUE)
    {
        obj_copy2 (result, &op->u.val._v);
        return true;
    };

    if (op->type==DA_OP_TYPE_VALUE_IN_MEMORY)
    {
        *rt_adr=(REG)Da_op_calc_adr_of_op (op, ctx, mem);

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
                    obj_tetrabyte2((uint32_t)out, result);
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
                    obj_octabyte2((uint64_t)out, result);
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
            break;
        };

        oassert(0);
    };

#if 0
	L ("%s(): type=%d!\n", __FUNCTION__, type);
#endif
	oassert(0); // should not be here
};

bool Da_op_set_value_of_op (Da_op* op, obj *val, CONTEXT * ctx, MemoryCache *mem) 
{
    address adr;

    switch (op->type)
    {
        case DA_OP_TYPE_REGISTER:
            X86_register_set_value (op->u.reg, ctx, val);
            return true;

        case DA_OP_TYPE_VALUE_IN_MEMORY:
            adr=(address)Da_op_calc_adr_of_op(op, ctx, mem);

            if (op->value_width_in_bits==8)
            {
                if (MC_WriteByte (mem, adr, obj_get_as_byte(val))==false)
                    goto COPY_FAILED;
            }
            else if (op->value_width_in_bits==16)
            {
                if (MC_WriteWyde (mem, adr, obj_get_as_wyde(val))==false)
                    goto COPY_FAILED;
            }
            else if (op->value_width_in_bits==32)
            {
                if (MC_WriteTetrabyte (mem, adr, obj_get_as_tetrabyte(val))==false)
                    goto COPY_FAILED;
            }
            else if (op->value_width_in_bits==64)
            {
                if (MC_WriteOctabyte (mem, adr, obj_get_as_octabyte(val))==false)
                    goto COPY_FAILED;
            }
            else if (op->value_width_in_bits==128)
            {
                //val.dump();
                byte * xmm=obj_get_as_xmm(val);
                //L ("%s(). writing to adr=0x%x\n", __FUNCTION__, adr);
                //L_print_buf (xmm, 16);
                if (MC_WriteBuffer (mem, adr, 16, xmm)==false)
                    goto COPY_FAILED;
            }
            else
            {
                oassert(!"unsupported value_width_in_bits");
            };
            break;

        default:
            oassert(!"unsupported type");
    };
COPY_FAILED:
    L ("%s(): Error writing at 0x" PRI_ADR_HEX ". Copy failed.\n", __FUNCTION__, adr);
    return false;
};

address Da_op_calc_adr_of_op (Da_op* op, const CONTEXT * ctx, MemoryCache *mem)
{
    address adr=0;
    oassert (op->type==DA_OP_TYPE_VALUE_IN_MEMORY);
    oassert (op->u.adr.adr_index_mult!=0);

    if (op->u.adr.adr_base != R_ABSENT)
        adr=adr+X86_register_get_value_as_u64 (op->u.adr.adr_base, ctx);

    if (op->u.adr.adr_index != R_ABSENT)
        adr=adr+X86_register_get_value_as_u64(op->u.adr.adr_index, ctx) * op->u.adr.adr_index_mult;

    return adr+op->u.adr.adr_disp; // negative values of adr_disp must work! (to be checked)
};

/* vim: set expandtab ts=4 sw=4 : */
