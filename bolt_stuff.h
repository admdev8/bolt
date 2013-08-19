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

#include <windows.h>
#include "bolt_mingw_addons.h"
#include "address.h"
#include "memorycache.h"

PNT_TIB get_TIB (HANDLE THDL);
address TIB_get_stack_top (HANDLE THDL, MemoryCache *mem);
address TIB_get_stack_bottom (HANDLE THDL, MemoryCache *mem);
address TIB_get_exceptionlist (HANDLE THDL, MemoryCache *mem);
bool TIB_is_ptr_in_stack_limits (HANDLE THDL, address p, MemoryCache *mem);

/* vim: set expandtab ts=4 sw=4 : */
