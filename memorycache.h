#pragma once

#include <windows.h>
#include "address.h"
#include "reg.h"
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
    BOOL to_be_flushed:8; // do we need to flush the block at this address? (was it changed?)
} MemoryCacheElement;

typedef struct _MemoryCache
{
    HANDLE PHDL;
    // адрес без последних 12 бит, ссылка на структуру MemoryCacheElement
    rbtree* _cache;
    
    BYTE * last_ptr;
    address last_ptr_idx;
    BOOL dont_read_from_quicksilver_places;
} MemoryCache;
    
BYTE *MC_find_page_ptr(MemoryCache* mc, address adr);
void MC_mark_as_to_be_flushed(MemoryCache* mc, address idx);
BOOL MC_LoadPageForAddress (MemoryCache* mc, address);

MemoryCache* MC_MemoryCache_ctor(HANDLE PHDL, BOOL dont_read_from_quicksilver_places);
void MC_MemoryCache_dtor(MemoryCache* mc, BOOL check_unflushed_elements);

MemoryCache* MC_MemoryCache_copy_ctor (MemoryCache* mc);

BOOL MC_ReadBuffer (MemoryCache* mc, address, SIZE_T, BYTE*);
BOOL MC_WriteBuffer (MemoryCache* mc, address, SIZE_T, BYTE*);
BOOL MC_L_print_buf_in_mem_ofs (MemoryCache* mc, address adr, REG size, REG ofs);
BOOL MC_L_print_buf_in_mem (MemoryCache* mc, address adr, SIZE_T size);
    
BOOL MC_ReadByte (MemoryCache* mc, address, BYTE*);
BOOL MC_WriteByte (MemoryCache* mc, address, BYTE);
BOOL MC_ReadWyde (MemoryCache* mc, address, WORD*);
BOOL MC_WriteWyde (MemoryCache* mc, address, WORD);
BOOL MC_ReadTetrabyte (MemoryCache* mc, address, DWORD*);
BOOL MC_WriteTetrabyte (MemoryCache* mc, address, DWORD);
BOOL MC_ReadOctabyte (MemoryCache* mc, address, DWORD64*);
BOOL MC_WriteOctabyte (MemoryCache* mc, address, DWORD64);

BOOL MC_ReadREG (MemoryCache* mc, address, REG * out);
BOOL MC_WriteREG (MemoryCache* mc, address, REG);

BOOL MC_GetString (MemoryCache *mc, address adr, BOOL unicode, strbuf * out);
    
void MC_Flush(MemoryCache* mc);
BOOL MC_DryRunFlush(MemoryCache* mc);
void MC_dump_state(fds *s, MemoryCache *mc);

#ifdef  __cplusplus
}
#endif
