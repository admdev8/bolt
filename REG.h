/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#pragma once

#include <windows.h>
#include <stdint.h>
#include "fmt_utils.h"

#define PRI_REG_HEX     PRI_SIZE_T_HEX
#define PRI_REG_HEX_PAD PRI_SIZE_T_HEX_PAD
#define PRI_REG_DEC     PRI_SIZE_T_DEC

#ifdef _WIN64
#define REG_1 1i64
#else
#define REG_1 1
#endif

#ifdef _WIN64
typedef DWORD64 REG;
typedef int64_t SIGNED_REG;
#else
typedef DWORD REG;
typedef int32_t SIGNED_REG;
#endif

