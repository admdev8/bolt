#pragma once

#include <windows.h>
#include <stdint.h>
#include "fmt_utils.h"
#include "datatypes.h"

#define PRI_ADR_HEX     PRI_SIZE_T_HEX
#define PRI_ADR_HEX_PAD PRI_SIZE_T_HEX_PAD
#define PRI_ADR_DEC     PRI_SIZE_T_DEC

#ifdef _WIN64
typedef octabyte address;
typedef octabyte_s address_offset;
#else
typedef tetrabyte address;
typedef tetrabyte_s address_offset;
#endif

// can be wrong for MS-DOS, for example
#define A_NO_ADDRESS 0
