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

#include <windows.h>
#include <stdbool.h>

#include "address.h"
#include "datatypes.h"
#include "rbtree.h"
#include "strbuf.h"
#include "logging.h"

// Rationale: x86 emu testing mode should be supported (DryRunFlush())

// TODO: здесь может быть также такой режим, когда все ф-ции будут работать, но на прямую, без кеша
// т.н. cache bypass

// change both if need
#define PAGE_SIZE 4096
#define LOG2_PAGE_SIZE 12

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _MemoryCacheElement
{
    BYTE block[PAGE_SIZE];
    bool to_be_flushed; // do we need to flush the block at this address? (was it changed?)
} MemoryCacheElement;

typedef struct _MemoryCache
{
    HANDLE PHDL;
    // адрес без последних 12 бит, ссылка на структуру MemoryCacheElement
    rbtree* _cache;
    
    BYTE * last_ptr;
    address last_ptr_idx;
    bool dont_read_from_quicksilver_places;
} MemoryCache;
    
BYTE *MC_find_page_ptr(MemoryCache* mc, address adr);
void MC_mark_as_to_be_flushed(MemoryCache* mc, address idx);
bool MC_LoadPageForAddress (MemoryCache* mc, address);

MemoryCache* MC_MemoryCache_ctor(HANDLE PHDL, bool dont_read_from_quicksilver_places);
void MC_MemoryCache_dtor(MemoryCache* mc, bool check_unflushed_elements);

MemoryCache* MC_MemoryCache_copy_ctor (MemoryCache* mc);

bool MC_ReadBuffer (MemoryCache* mc, address, SIZE_T, BYTE*);
bool MC_WriteBuffer (MemoryCache* mc, address, SIZE_T, BYTE*);
bool MC_L_print_buf_in_mem_ofs (MemoryCache* mc, address adr, REG size, REG ofs);
bool MC_L_print_buf_in_mem (MemoryCache* mc, address adr, SIZE_T size);
    
bool MC_ReadByte (MemoryCache* mc, address, BYTE*);
bool MC_WriteByte (MemoryCache* mc, address, BYTE);
bool MC_ReadWyde (MemoryCache* mc, address, WORD*);
bool MC_WriteWyde (MemoryCache* mc, address, WORD);
bool MC_ReadTetrabyte (MemoryCache* mc, address, DWORD*);
bool MC_WriteTetrabyte (MemoryCache* mc, address, DWORD);
bool MC_ReadOctabyte (MemoryCache* mc, address, DWORD64*);
bool MC_WriteOctabyte (MemoryCache* mc, address, DWORD64);

bool MC_ReadREG (MemoryCache* mc, address, REG * out);
bool MC_WriteREG (MemoryCache* mc, address, REG);
    
bool MC_WriteValue(MemoryCache *mc, address adr, unsigned width, REG val);

bool MC_GetString (MemoryCache *mc, address adr, bool unicode, strbuf * out);
    
void MC_Flush(MemoryCache* mc);
bool MC_DryRunFlush(MemoryCache* mc);
void MC_dump_state(fds *s, MemoryCache *mc);
bool MC_get_any_string (MemoryCache *mem, const address adr, strbuf *out);

#ifdef  __cplusplus
}
#endif

/* vim: set expandtab ts=4 sw=4 : */
