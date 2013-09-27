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
#include <dbghelp.h>
#include <stdbool.h>

#include "address.h"

typedef struct _PE_info
{
    bool PE32_plus;
    address original_base;
    address OEP;
    DWORD timestamp;
    DWORD machine;
    SIZE_T size;
    char *internal_name; // allocated via DSTRDUP. may be NULL.
} PE_info;

typedef void (*callback_add_symbol)(address a, char *name, void* param);

void MapAndLoad_or_die(PSTR image_name, PSTR dllpath, 
        PLOADED_IMAGE LoadedImage, bool DotDll, bool ReadOnly);
void UnMapAndLoad_or_die(PLOADED_IMAGE LoadedImage);

void PE_get_sections_info (char *fname, IMAGE_SECTION_HEADER **sections, unsigned *sections_total);
void PE_get_info (char *fname, address loaded_base, PE_info *out, callback_add_symbol add_symbol_fn, void *add_symbol_fn_params);
void PE_info_free (PE_info *i);

/* vim: set expandtab ts=4 sw=4 : */
