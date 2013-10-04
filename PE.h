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
bool PE_is_PE32 (LOADED_IMAGE *im);
IMAGE_EXPORT_DIRECTORY* PE_get_export_directory (LOADED_IMAGE *im, bool PE32_plus);
unsigned PE_count_import_descriptors (LOADED_IMAGE *im);
address PE_get_import_descriptor_RVA (LOADED_IMAGE *im, bool PE32_plus);
tetrabyte* PE_get_reloc_directory (LOADED_IMAGE *im, bool PE32_plus);
IMAGE_IMPORT_DESCRIPTOR* PE_get_import_descriptor (LOADED_IMAGE *im, bool PE32_plus);
address PE_get_original_base (LOADED_IMAGE *im);

void PE_get_sections_info (char *fname, IMAGE_SECTION_HEADER **sections, unsigned *sections_total);
void PE_get_info (char *fname, address loaded_base, PE_info *out, callback_add_symbol add_symbol_fn, void *add_symbol_fn_params);
void PE_info_free (PE_info *i);

typedef void (*callback_enum_fixups)(unsigned i, byte type, address a, void* param);
void enum_all_fixups (LOADED_IMAGE *im, callback_enum_fixups callback_fn, void* param);
unsigned count_fixups (LOADED_IMAGE *im);
IMAGE_SECTION_HEADER* get_last_section (LOADED_IMAGE *im);
void calculate_next_available_RVA_and_phys_ofs(LOADED_IMAGE *im, address *next_available_RVA, 
		address *next_available_phys_ofs);
size_t add_PE_section_at_end(LOADED_IMAGE *im, char* name, SIZE_T sz, DWORD characteristics);
void set_data_directory_entry (LOADED_IMAGE *im, unsigned no, DWORD adr, DWORD sz);
IMAGE_SECTION_HEADER* PE_find_section_by_name (LOADED_IMAGE *im, char *name);

