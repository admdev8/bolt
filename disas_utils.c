#include "disas_utils.h"
#include "dmalloc.h"
#include <assert.h>
#include "X86_register_helpers.h"

BOOL Da_op_get_value_of_op (Da_op *op, address * rt_adr, const CONTEXT * ctx, MemoryCache *mem, 
        const char *fname, unsigned fileline, s_Value *result)
{
    BOOL b;

    if (op->type==DA_OP_TYPE_REGISTER)
    {
        assert (op->u.reg != R_ABSENT);
        // вытянуть из ctx
        X86_register_get_value (op->u.reg, ctx, result);
        return TRUE;
    };

    if (op->type==DA_OP_TYPE_VALUE)
    {
        copy_Value (result, &op->u.val.v);
        return TRUE;
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
                    create_Value(V_BYTE, (uint8_t)out, result);
                    return TRUE;
                }
                else
                    return FALSE;
            };
        case 16:
            {
                WORD out;
                b=MC_ReadWyde (mem, *rt_adr, &out);
                if (b)
                {
                    create_Value(V_WORD, (uint16_t)out, result);
                    return TRUE;
                }
                else
                    return FALSE;
            };
        case 32:
            {
                DWORD out;
                b=MC_ReadTetrabyte (mem, *rt_adr, &out);
                if (b)
                {
                    create_Value(V_DWORD, (uint32_t)out, result);
                    return TRUE;
                }
                else
                    return FALSE;
            };
        case 64:
            {
                DWORD64 out;
                b=MC_ReadOctabyte (mem, *rt_adr, &out);
                if (b)
                {
                    create_Value(V_QWORD, (uint64_t)out, result);
                    return TRUE;
                }
                else
                    return FALSE;
            };
        case 128:
            {
                M128A xmm;
                if (MC_ReadBuffer (mem, *rt_adr, sizeof (M128A), (BYTE*)&xmm)==FALSE)
                    return FALSE;
                //L ("%s(). rt_adr=0x%x\n", __FUNCTION__, rt_adr);
                //L_print_buf ((BYTE*)&xmm, 16);
                create_XMM_Value ((uint8_t*)&xmm, result);
                //printf ("%s(): val after constructing:\n", __FUNCTION__);
                //val.dump();
                return TRUE;
            };
        default:
            assert(!"unknown value_width_in_bits");
            break;
        };

        assert(0);
    };

#if 0
	L ("%s(): type=%d!\n", __FUNCTION__, type);
#endif
	assert(0); // should not be here
	return FALSE;
};

BOOL Da_op_set_value_of_op (Da_op* op, s_Value *val, CONTEXT * ctx, MemoryCache *mem) 
{
    address adr;

    switch (op->type)
    {
        case DA_OP_TYPE_REGISTER:
            X86_register_set_value (op->u.reg, ctx, val);
            return TRUE;

        case DA_OP_TYPE_VALUE_IN_MEMORY:
            adr=(address)Da_op_calc_adr_of_op(op, ctx, mem);

            if (op->value_width_in_bits==8)
            {
                if (MC_WriteByte (mem, adr, get_as_8(val))==FALSE)
                    goto COPY_FAILED;
            }
            else if (op->value_width_in_bits==16)
            {
                if (MC_WriteWyde (mem, adr, get_as_16(val))==FALSE)
                    goto COPY_FAILED;
            }
            else if (op->value_width_in_bits==32)
            {
                if (MC_WriteTetrabyte (mem, adr, get_as_32(val))==FALSE)
                    goto COPY_FAILED;
            }
            else if (op->value_width_in_bits==64)
            {
                if (MC_WriteOctabyte (mem, adr, val->u.v)==FALSE)
                    goto COPY_FAILED;
            }
            else if (op->value_width_in_bits==128)
            {
                //val.dump();
                BYTE * xmm=get_xmm(val);
                //L ("%s(). writing to adr=0x%x\n", __FUNCTION__, adr);
                //L_print_buf (xmm, 16);
                if (MC_WriteBuffer (mem, adr, 16, xmm)==FALSE)
                    goto COPY_FAILED;
            }
            else
            {
                assert(0);
            };
            return TRUE;
            break;

        default:
            assert(0);
            return FALSE; // make compiler happy
    };
COPY_FAILED:
    L ("%s(): Error writing at 0x" PRI_ADR_HEX ". Copy failed.\n", __FUNCTION__, adr);
    return FALSE;
};

address Da_op_calc_adr_of_op (Da_op* op, const CONTEXT * ctx, MemoryCache *mem)
{
    address adr=0;
    assert (op->type==DA_OP_TYPE_VALUE_IN_MEMORY);
    assert (op->u.adr.adr_index_mult!=0);

    if (op->u.adr.adr_base != R_ABSENT)
        adr=adr+X86_register_get_value_as_u64 (op->u.adr.adr_base, ctx);

    if (op->u.adr.adr_index != R_ABSENT)
        adr=adr+X86_register_get_value_as_u64(op->u.adr.adr_index, ctx) * op->u.adr.adr_index_mult;

    return adr+op->u.adr.adr_disp; // negative values of adr_disp must work! (to be checked)
};

