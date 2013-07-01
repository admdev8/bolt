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

#pragma once

#include "X86_register.h"
#include "datatypes.h"
#include <windows.h>
#include "address.h"
#include "strbuf.h"
#include <stdio.h>
#include "logging.h"
#include "x86_disas.h"
#include "mingw_addons.h"

#ifdef  __cplusplus
extern "C" {
#endif

void set_TF (CONTEXT *ctx);
void clear_TF (CONTEXT *ctx);
void set_or_clear_flag (CONTEXT * ctx, int flag, uint64_t cond);

REG CONTEXT_get_SP (const CONTEXT * ctx);
void CONTEXT_set_SP (CONTEXT * ctx, REG val);

REG CONTEXT_get_BP (const CONTEXT * ctx);
void CONTEXT_set_BP (CONTEXT * ctx, REG val);

REG CONTEXT_get_Accum (const CONTEXT * ctx);
void CONTEXT_set_Accum (CONTEXT * ctx, REG A);

REG CONTEXT_get_xCX (const CONTEXT * ctx);
void CONTEXT_set_xCX (CONTEXT * ctx, REG val);

REG CONTEXT_get_xDX (const CONTEXT * ctx);
void CONTEXT_set_xDX (CONTEXT * ctx, REG val);

REG CONTEXT_get_xDI (const CONTEXT * ctx);
void CONTEXT_set_xDI (CONTEXT * ctx, REG val);

REG CONTEXT_get_xSI (const CONTEXT * ctx);
void CONTEXT_set_xSI (CONTEXT * ctx, REG val);

REG CONTEXT_get_PC (const CONTEXT * ctx);
void CONTEXT_set_PC (CONTEXT * ctx, REG PC);
void CONTEXT_add_to_PC (CONTEXT * ctx, REG i);
void CONTEXT_decrement_PC (CONTEXT * ctx);

void CONTEXT_setDRx_and_DR7 (CONTEXT * ctx, int bp_i, REG a);

void dump_DRx (fds* s, const CONTEXT *ctx);
void dump_CONTEXT (fds* s, const CONTEXT * ctx, bool dump_DRx, bool dump_xmm_regs);

bool CONTEXT_compare (fds* s, const CONTEXT * ctx1, const CONTEXT * ctx2);

void CONTEXT_set_reg (CONTEXT * ctx, X86_register r, REG v);
void CONTEXT_set_reg_STx (CONTEXT * ctx, X86_register r, double v);
void CONTEXT_setDRx_and_DR7 (CONTEXT * ctx, int bp_i, REG a);
void CONTEXT_clear_bp_in_DR7 (CONTEXT * ctx, int bp_n);
void CONTEXT_setAccum (CONTEXT * ctx, REG v);
REG CONTEXT_getAccum (CONTEXT * ctx);
REG CONTEXT_get_reg (CONTEXT * ctx, X86_register r);
address CONTEXT_calc_adr_of_op (CONTEXT * ctx, Da_op *op);
void CONTEXT_dump_DRx(fds *s, CONTEXT *ctx);

bool is_NZ_cond (const CONTEXT * ctx);
bool is_BE_cond (const CONTEXT * ctx);
bool is_Z_cond (const CONTEXT * ctx);
bool is_O_cond (const CONTEXT * ctx);
bool is_S_cond (const CONTEXT * ctx);
bool is_NS_cond (const CONTEXT * ctx);
bool is_G_cond (const CONTEXT * ctx);
bool is_GE_cond (const CONTEXT * ctx);
bool is_L_cond (const CONTEXT * ctx);
bool is_LE_cond (const CONTEXT * ctx);
bool is_A_cond (const CONTEXT * ctx);
bool is_B_cond (const CONTEXT * ctx);
bool is_NB_cond (const CONTEXT * ctx);

void dump_FPU_in_XSAVE_FORMAT (fds* s, XSAVE_FORMAT *t);
void dump_FPU_in_FLOATING_SAVE_AREA_if_need (fds* s, FLOATING_SAVE_AREA *f);
const char *get_BP_register_name();
const char *get_AX_register_name();
const char *get_CX_register_name();
const char *get_DX_register_name();

#ifdef  __cplusplus
}
#endif
