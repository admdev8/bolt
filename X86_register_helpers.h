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

#ifdef  __cplusplus
extern "C" {
#endif

void X86_register_get_value (X86_register r, const CONTEXT *ctx, s_Value* out);
uint64_t X86_register_get_value_as_u64 (X86_register r, const CONTEXT *ctx);
void X86_register_set_value (X86_register r, CONTEXT *ctx, s_Value *val);

#ifdef  __cplusplus
}
#endif
