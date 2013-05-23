#include <windows.h>
#include "mingw_addons.h"
#include "address.h"
#include "memorycache.h"

PNT_TIB get_TIB (HANDLE THDL);
address TIB_get_stack_top (HANDLE THDL, MemoryCache *mem);
address TIB_get_stack_bottom (HANDLE THDL, MemoryCache *mem);
address TIB_get_exceptionlist (HANDLE THDL, MemoryCache *mem);
bool TIB_is_ptr_in_stack_limits (HANDLE THDL, address p, MemoryCache *mem);
