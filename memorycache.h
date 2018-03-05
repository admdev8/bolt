/*
 *  _           _ _   
 * | |         | | |  
 * | |__   ___ | | |_ 
 * | '_ \ / _ \| | __|
 * | |_) | (_) | | |_ 
 * |_.__/ \___/|_|\__|
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013-2017
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#pragma once

#include <windows.h>
#include <stdbool.h>

#include "datatypes.h"
#include "rbtree.h"
#include "strbuf.h"
#include "logging.h"

// Rationale: x86 emu testing mode should be supported (DryRunFlush())

// change both if need
#define PAGE_SIZE 4096
#define LOG2_PAGE_SIZE 12

#ifdef  __cplusplus
extern "C" {
#endif

struct MemoryCacheElement
{
    BYTE block[PAGE_SIZE];
    //BYTE modified[PAGE_SIZE/8]; // 1 if byte was written there
    bool to_be_flushed; // do we need to flush the block at this address? (was it changed?)
};

struct MemoryCache
{
    HANDLE PHDL;
    // key: адрес без последних 12 бит; value: ссылка на структуру MemoryCacheElement
    rbtree* _cache;

    BYTE * last_ptr;
    address last_ptr_idx;
    bool last_ptr_idx_present;
    bool dont_read_from_quicksilver_places;
#ifdef _DEBUG
    bool testing;
    BYTE *testing_memory;
    SIZE_T testing_memory_size;
#endif
};

BYTE *MC_find_page_ptr(struct MemoryCache* mc, address adr);
void MC_mark_as_to_be_flushed(struct MemoryCache* mc, address idx);
bool MC_LoadPageForAddress (struct MemoryCache* mc, address);

struct MemoryCache* MC_MemoryCache_ctor(HANDLE PHDL, bool dont_read_from_quicksilver_places);
#ifdef _DEBUG
struct MemoryCache* MC_MemoryCache_ctor_testing(BYTE *testing_memory, SIZE_T testing_memory_size);
#endif
void MC_MemoryCache_dtor(struct MemoryCache* mc, bool check_unflushed_elements);

struct MemoryCache* MC_MemoryCache_copy_ctor (struct MemoryCache* mc);

bool MC_ReadBuffer (struct MemoryCache* mc, address, SIZE_T, BYTE*);
bool MC_WriteBuffer (struct MemoryCache* mc, address, SIZE_T, BYTE*);
bool MC_L_print_buf_in_mem_ofs (struct MemoryCache* mc, address adr, REG size, REG ofs);
bool MC_L_print_buf_in_mem (struct MemoryCache* mc, address adr, SIZE_T size);
    
bool MC_ReadByte (struct MemoryCache* mc, address, BYTE*);
bool MC_WriteByte (struct MemoryCache* mc, address, BYTE);
bool MC_ReadWyde (struct MemoryCache* mc, address, WORD*);
bool MC_WriteWyde (struct MemoryCache* mc, address, WORD);
bool MC_ReadTetrabyte (struct MemoryCache* mc, address, DWORD*);
bool MC_WriteTetrabyte (struct MemoryCache* mc, address, DWORD);
bool MC_ReadOctabyte (struct MemoryCache* mc, address, DWORD64*);
bool MC_WriteOctabyte (struct MemoryCache* mc, address, DWORD64);

bool MC_ReadREG (struct MemoryCache* mc, address, REG * out);
bool MC_WriteREG (struct MemoryCache* mc, address, REG);
    
bool MC_WriteValue(struct MemoryCache *mc, address adr, unsigned width, REG val);

bool MC_GetString (struct MemoryCache *mc, address adr, bool unicode, strbuf * out);
    
void MC_Flush(struct MemoryCache* mc);
// was named MC_DryRunFlush()
bool MC_CompareInternalStateWithMemory(struct MemoryCache* mc);
void MC_dump_state(fds *s, struct MemoryCache *mc);
bool MC_get_any_string (struct MemoryCache *mem, const address adr, strbuf *out);

#ifdef  __cplusplus
}
#endif

/* vim: set expandtab ts=4 sw=4 : */
