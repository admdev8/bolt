#pragma once

#include <windows.h>

#include "value.h"
#include "CONTEXT_utils.h"
#include "memorycache.h"
#include "datatypes.h"
#include "x86_disas.h"

#ifdef  __cplusplus
extern "C" {
#endif

bool Da_op_get_value_of_op (Da_op *op, address * rt_adr, const CONTEXT * ctx, MemoryCache *mem, const char *fname, unsigned fileline, s_Value *result);
bool Da_op_set_value_of_op (Da_op* op, s_Value *val, CONTEXT * ctx, MemoryCache *mem);
address Da_op_calc_adr_of_op (Da_op* op, const CONTEXT * ctx, MemoryCache *mem);

#ifdef  __cplusplus
}
#endif
