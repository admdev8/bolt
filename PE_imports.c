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

#include "dmalloc.h"
#include "oassert.h"
#include "PE.h"
#include "PE_imports.h"
#include "memutils.h"
#include "stuff.h"
#include "ostrings.h"
#include "stuff.h"

struct PE_get_imports_info* PE_get_imports (LOADED_IMAGE *im)
{
	struct PE_get_imports_info* rt=DMALLOC(struct PE_get_imports_info, 1, "struct PE_get_imports_info");
	bool PE32_plus=PE_is_PE32(im);
	rt->start_RVA=PE_get_import_descriptor_RVA(im, PE32_plus);
	IMAGE_IMPORT_DESCRIPTOR* import_dir=PE_get_import_descriptor(im, PE32_plus);

	if (import_dir==NULL)
	{
		free(rt);
		return NULL; // No imports
	};

	rt->import_descriptors_t=PE_count_import_descriptors (im);
	rt->dlls=DMALLOC(struct PE_get_imports_DLL_info, rt->import_descriptors_t, 
			"struct PE_get_imports_DLL_info");

	int j;
	IMAGE_IMPORT_DESCRIPTOR *i;
	for (i=import_dir, j=0; i->OriginalFirstThunk; i++, j++)
	{
		address* OriginalFirstThunk_a=(address*)ImageRvaToVa (im->FileHeader, im->MappedAddress, i->OriginalFirstThunk, NULL);
		char* name=(char*)ImageRvaToVa (im->FileHeader, im->MappedAddress, i->Name, NULL);
		struct PE_get_imports_DLL_info* DLL=rt->dlls+j;
		DLL->DLL_name=DSTRDUP(name,"char*");
		DLL->FirstThunk=i->FirstThunk;
		DLL->allocate_thunks=false;

		unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)OriginalFirstThunk_a);

		DLL->symbols=DMALLOC(byte*, symbols_t+1, "byte*");
		DLL->symbols[symbols_t]=NULL;
		
		for (address *s=OriginalFirstThunk_a, si=0; *s; s++, si++)
		{
			byte *tmp=(byte*)ImageRvaToVa(im->FileHeader, im->MappedAddress, *s, NULL);
			DLL->symbols[si]=DMEMDUP (tmp, strlen((char*)tmp+2)+3, "byte*");
		};
	};

	return rt;
};

void PE_get_imports_info_free(struct PE_get_imports_info *i)
{
	for (int DLLs=0; DLLs < i->import_descriptors_t; DLLs++)
	{
		struct PE_get_imports_DLL_info* DLL=i->dlls + DLLs;

		DFREE (DLL->DLL_name);
		for (int s=0; DLL->symbols[s]; s++)
			DFREE(DLL->symbols[s]);
		DFREE(DLL->symbols);
	};
	DFREE(i->dlls);
	DFREE(i);
};

struct PE_get_imports_info* PE_get_imports_info_deep_copy(struct PE_get_imports_info *i)
{
	struct PE_get_imports_info *rt;

	rt=DMEMDUP (i, sizeof(struct PE_get_imports_info), "struct PE_get_imports_info");
	
	for (int d=0; d < rt->import_descriptors_t; d++)
	{
		struct PE_get_imports_DLL_info* DLL_d=rt->dlls + d;
		struct PE_get_imports_DLL_info* DLL_s=i->dlls + d;

		DLL_d->DLL_name=DSTRDUP (DLL_s->DLL_name, "char*");
		unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)DLL_s->symbols);
		DLL_d->symbols=DCALLOC(byte*, symbols_t+1, "byte*");

		for (int s=0; s<symbols_t; s++)
			DLL_d->symbols[s]=
				DMEMDUP(DLL_s->symbols[s], 
						strlen ((char*)DLL_s->symbols[s]+2)+3, "byte*");
		DLL_d->allocate_thunks=DLL_s->allocate_thunks;
	};

	return rt;
};

int find_dll_in_imports (struct PE_get_imports_info *i, const char *dll)
{
	for (unsigned j=0; j<i->import_descriptors_t; j++)
	{
		struct PE_get_imports_DLL_info *DLL=i->dlls + j;

		if (stricmp(DLL->DLL_name, dll)==0)
			return j;
	};
	return -1;
};

void dump_imports (struct PE_get_imports_info *i)
{
	printf ("start_RVA=0x" PRI_ADR_HEX "\n", i->start_RVA);
	for (int d=0; d < i->import_descriptors_t; d++)
	{
		struct PE_get_imports_DLL_info* DLL=i->dlls + d;
		printf ("DLL name %s. FirstThunk=0x" PRI_ADR_HEX "\n", DLL->DLL_name, DLL->FirstThunk);
		for (int s=0; DLL->symbols[s]; s++)
		{
			byte* tmp=DLL->symbols[s];
			printf ("hint=0x%x, name=%s\n", *(wyde*)tmp, (char*)tmp+2);
		};
		printf ("\n");
	};
};

// if place_thunks==true, this function ignores FirstThunk array, 
// frees it and fills it again by new values
// otherwise, thunks are not placed and FirstThunk are pointed to what is in PE_get_imports_info struct
size_t PE_generate_import_table (struct PE_get_imports_info *i, byte* out, size_t out_size,
		size_t *size_of_IMAGE_DIRECTORY_ENTRY_IMPORT)
{
	address out_buf_end_RVA=i->start_RVA + out_size;
	size_t descriptors_size=sizeof(IMAGE_IMPORT_DESCRIPTOR) * (i->import_descriptors_t+1); // including terminator
	address cur_RVA=i->start_RVA;
	byte* cur_ptr=out;
	
	address FirstThunk_cur=cur_RVA;
	// IMAGE_IMPORT_DESCRIPTOR for each DLL will be filled later.
	byte* desc_tbl=cur_ptr;
	
	// fill terminating IMAGE_IMPORT_DESCRIPTOR descriptor with zeroes
	bzero(&((IMAGE_IMPORT_DESCRIPTOR*)desc_tbl)[i->import_descriptors_t], sizeof(IMAGE_IMPORT_DESCRIPTOR));

	cur_ptr+=descriptors_size; cur_RVA+=descriptors_size;
	*size_of_IMAGE_DIRECTORY_ENTRY_IMPORT=descriptors_size;

	for (int d=0; d<i->import_descriptors_t; d++)
	{
		struct PE_get_imports_DLL_info* DLL=i->dlls + d;
		// count number of hints/symbols
		unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)DLL->symbols);

		address* table_of_pointers=(address*)cur_ptr;
		table_of_pointers[symbols_t]=0; // terminator

		// update OriginalFirstThunk
		((IMAGE_IMPORT_DESCRIPTOR*)desc_tbl)[d].OriginalFirstThunk=cur_RVA;
		
		// reserve place for all pointers to symbol hints/names including terminator
		size_t symbols_table_size=(symbols_t+1) * sizeof(address);
		cur_RVA+=symbols_table_size; cur_ptr+=symbols_table_size;
		oassert (cur_RVA < out_buf_end_RVA);

		// put all symbol hints/names
		for (unsigned s=0; s<symbols_t; s++)
		{
			// update pointer to this hint/name
			table_of_pointers[s]=cur_RVA;
			// put symbol hint/name
			byte *hint_name=DLL->symbols[s];
			unsigned name_z_len_2=strlen((char*)hint_name+2)+1+2; // length including 16-bit hint and terminating zero
			memcpy (cur_ptr, hint_name, name_z_len_2);
			cur_ptr+=name_z_len_2; cur_RVA+=name_z_len_2;
			oassert (cur_RVA < out_buf_end_RVA);
		};

		// update RVA of DLL name
		((IMAGE_IMPORT_DESCRIPTOR*)desc_tbl)[d].Name=cur_RVA;
		// put DLL name
		unsigned DLL_name_z_len=strlen(DLL->DLL_name)+1;
		memcpy (cur_ptr, DLL->DLL_name, DLL_name_z_len);
		cur_ptr+=DLL_name_z_len; cur_RVA+=DLL_name_z_len;
		oassert (cur_RVA < out_buf_end_RVA);
		((IMAGE_IMPORT_DESCRIPTOR*)desc_tbl)[d].TimeDateStamp=0;
		((IMAGE_IMPORT_DESCRIPTOR*)desc_tbl)[d].ForwarderChain=-1;
	};

	// add FirstThunk's
	for (int d=0; d<i->import_descriptors_t; d++)
	{
		struct PE_get_imports_DLL_info* DLL=i->dlls + d;
		
		if (DLL->allocate_thunks)
		{
			// count number of hints/symbols
			unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)DLL->symbols);

			size_t FirstThunk_a_size=symbols_t * sizeof(address);
			
			// write new FirstThunk value
			((IMAGE_IMPORT_DESCRIPTOR*)desc_tbl)[d].FirstThunk=cur_RVA;
			// update it also here:
			DLL->FirstThunk=cur_RVA;
	
			// I don't know why this is, but it should be so
			bytefill(cur_ptr, FirstThunk_a_size, 0xFF); 

			cur_RVA+=FirstThunk_a_size; cur_ptr+=FirstThunk_a_size;
		}
		else
			((IMAGE_IMPORT_DESCRIPTOR*)desc_tbl)[d].FirstThunk=DLL->FirstThunk;
	};

	return cur_RVA - i->start_RVA;
};

void add_DLL_and_symbol_to_imports (struct PE_get_imports_info *i, char *dll, char *symname, wyde hint)
{
	int DLL_idx=find_dll_in_imports(i, dll);
	if (DLL_idx==-1)
	{	// add new DLL to the table
		DLL_idx=i->import_descriptors_t;
		i->import_descriptors_t++;
		i->dlls=DREALLOC (i->dlls, struct PE_get_imports_DLL_info, 
				i->import_descriptors_t, "struct PE_get_imports_DLL_info");
		struct PE_get_imports_DLL_info *DLL=i->dlls + DLL_idx;
		DLL->DLL_name=DSTRDUP(dll, "DLL name");
		DLL->symbols=DCALLOC (byte*, 1, "byte*"); // NULL as terminator
		DLL->allocate_thunks=true;
		// FirstThunk is NULL here
	};
	struct PE_get_imports_DLL_info *DLL=i->dlls + DLL_idx;
	unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)DLL->symbols);
	DLL->symbols=DREALLOC (DLL->symbols, byte*, symbols_t+2, "byte*");
	byte** symbols=DLL->symbols;
	symbols[symbols_t]=DCALLOC (byte, 2+strlen (symname)+1, "byte*");
	memcpy (symbols[symbols_t], &hint, sizeof(wyde));
	memcpy (symbols[symbols_t]+2, symname, strlen(symname));
	symbols[symbols_t+1]=NULL; // terminator
};

address PE_find_thunk_by_import (struct PE_get_imports_info *i, char* dll_name, char* sym_name)
{
	int DLL_no=find_dll_in_imports (i, dll_name);
	oassert(DLL_no!=-1);
	oassert(DLL_no < i->import_descriptors_t);

	struct PE_get_imports_DLL_info* DLL=i->dlls + DLL_no;
	unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)DLL->symbols);

	for (unsigned s=0; s<symbols_t; s++)
		if (stricmp((char*)(DLL->symbols[s]+2), sym_name)==0)
			return i->dlls[DLL_no].FirstThunk + s*sizeof(address);

	die ("%s(): nothing found\n", __FUNCTION__);
};

