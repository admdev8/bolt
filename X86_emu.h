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

// this is partial user-mode x86 emulator

#pragma once

#include "memorycache.h"
#include "x86_disas.h"
#include "CONTEXT_utils.h"

typedef enum _Da_emulate_result
{
    DA_NOT_EMULATED,
    DA_EMULATED_OK,
    DA_EMULATED_CANNOT_READ_MEMORY,
    DA_EMULATED_CANNOT_WRITE_MEMORY,
    DA_EMULATED_NOT_SUPPORTED
} Da_emulate_result;

// FIXME some functions should be static
bool ins_traced_by_one_step(enum Ins_codes i);
Da_emulate_result Da_emulate_MOV_op1_op2(struct Da *d, CONTEXT * ctx, MemoryCache *mem, unsigned ins_prefixes, address FS);
Da_emulate_result Da_emulate_Jcc (struct Da* d, bool cond, CONTEXT * ctx);
Da_emulate_result Da_emulate_CMOVcc (struct Da* d, bool cond, CONTEXT * ctx, MemoryCache *mem, unsigned ins_prefixes, address FS);
Da_emulate_result Da_emulate_SETcc (struct Da* d, bool cond, CONTEXT * ctx, MemoryCache *mem, unsigned ins_prefixes, address FS);
Da_emulate_result Da_emulate(struct Da* d, CONTEXT * ctx, MemoryCache *mem, bool emulate_FS_accesses, address FS);
const char* Da_emulate_result_to_string(Da_emulate_result r);

/* vim: set expandtab ts=4 sw=4 : */
