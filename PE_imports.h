#include "PE.h"

struct PE_get_imports_info
{
	address start_RVA;
	unsigned import_descriptors_t;
	char** DLL_names;
	byte*** symbols;
	address* FirstThunk; // array of RVA addresses
};

unsigned PE_count_import_descriptors (LOADED_IMAGE *im);
struct PE_get_imports_info* PE_get_imports (LOADED_IMAGE *im);
void PE_get_imports_info_free(struct PE_get_imports_info *i);
struct PE_get_imports_info* PE_get_imports_info_deep_copy(struct PE_get_imports_info *i);
void dump_imports (struct PE_get_imports_info *);
size_t PE_generate_import_table (struct PE_get_imports_info *i, bool place_thunks, 
		byte* out, unsigned out_size);
void add_DLL_and_symbol_to_imports (struct PE_get_imports_info *i, char *dll, char *symname, wyde hint,
		address new_thunk);
bool PE_find_import_by_thunk(struct PE_get_imports_info *i, address val, 
		unsigned *DLL_no, unsigned *sym_no);
address PE_find_thunk_by_import (struct PE_get_imports_info *i, unsigned DLL_no, unsigned sym_no);
bool dll_present_in_imports (struct PE_get_imports_info *i, char *dll);

