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

#include "PE.h"

struct PE_get_imports_DLL_info
{
	char *DLL_name;
	char **symbols;
	size_t symbols_t;
	wyde* hints;
	bool allocate_thunks;
	address FirstThunk;
};

struct PE_get_imports_info
{
	address start_RVA;
	unsigned import_descriptors_t;
	struct PE_get_imports_DLL_info* dlls;
};

unsigned PE_count_import_descriptors (LOADED_IMAGE *im);
struct PE_get_imports_info* PE_get_imports (LOADED_IMAGE *im);
void PE_get_imports_info_free(struct PE_get_imports_info *i);
struct PE_get_imports_info* PE_get_imports_info_deep_copy(struct PE_get_imports_info *i);
void dump_imports (struct PE_get_imports_info *);
size_t PE_approx_import_table_size(struct PE_get_imports_info *i);
size_t PE_generate_import_table (struct PE_get_imports_info *i, byte* out, size_t out_size,
		size_t *size_of_IMAGE_DIRECTORY_ENTRY_IMPORT);
void add_DLL_and_symbol_to_imports (struct PE_get_imports_info *i, char *dll, char *symname, wyde hint);
int find_dll_in_imports (struct PE_get_imports_info *i, const char *dll);
address PE_find_thunk_by_import (struct PE_get_imports_info *i, char* dll_name, char* sym_name);

