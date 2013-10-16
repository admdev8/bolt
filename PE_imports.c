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
	rt->DLL_names=DMALLOC(char*, rt->import_descriptors_t, "char*");
	rt->symbols=DMALLOC(byte**, rt->import_descriptors_t, "byte**");
	rt->FirstThunk=DMALLOC(address, rt->import_descriptors_t, "address");

	int j;
	IMAGE_IMPORT_DESCRIPTOR *i;
	for (i=import_dir, j=0; i->OriginalFirstThunk; i++, j++)
	{
		address* OriginalFirstThunk_a=(address*)ImageRvaToVa (im->FileHeader, im->MappedAddress, i->OriginalFirstThunk, NULL);
		char* name=(char*)ImageRvaToVa (im->FileHeader, im->MappedAddress, i->Name, NULL);
		rt->DLL_names[j]=DSTRDUP(name,"char*");
		rt->FirstThunk[j]=i->FirstThunk;

		unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)OriginalFirstThunk_a);

		rt->symbols[j]=DMALLOC(byte*, symbols_t+1, "byte*");
		rt->symbols[j][symbols_t]=NULL;
		
		for (address *s=OriginalFirstThunk_a, si=0; *s; s++, si++)
		{
			byte *tmp=(byte*)ImageRvaToVa(im->FileHeader, im->MappedAddress, *s, NULL);
			rt->symbols[j][si]=
				DMEMDUP (tmp, strlen((char*)tmp+2)+3, "byte*");
		};
	};

	return rt;
};

void PE_get_imports_info_free(struct PE_get_imports_info *i)
{
	for (int DLLs=0; DLLs < i->import_descriptors_t; DLLs++)
	{
		DFREE (i->DLL_names[DLLs]);
		for (int s=0; i->symbols[DLLs][s]; s++)
			DFREE(i->symbols[DLLs][s]);
		DFREE(i->symbols[DLLs]);
	};
	DFREE(i->FirstThunk);
	DFREE(i->DLL_names);
	DFREE(i->symbols);
	DFREE(i);
};

struct PE_get_imports_info* PE_get_imports_info_deep_copy(struct PE_get_imports_info *i)
{
	struct PE_get_imports_info *rt;

	rt=DMEMDUP (i, sizeof(struct PE_get_imports_info), "struct PE_get_imports_info");
	rt->DLL_names=DMALLOC(char*, rt->import_descriptors_t, "char*");
	rt->symbols=DMALLOC(byte**, rt->import_descriptors_t, "byte**");
	rt->FirstThunk=DMEMDUP (i->FirstThunk, sizeof(address)*rt->import_descriptors_t, "address*");

	for (int DLL=0; DLL < rt->import_descriptors_t; DLL++)
	{
		rt->DLL_names[DLL]=DSTRDUP (i->DLL_names[DLL], "char*");

		unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)i->symbols[DLL]);
		rt->symbols[DLL]=DCALLOC(byte*, symbols_t+1, "byte*");

		for (int s=0; s<symbols_t; s++)
			rt->symbols[DLL][s]=
				DMEMDUP(i->symbols[DLL][s], strlen ((char*)i->symbols[DLL][s]+2)+3, "byte*");
	};

	return rt;
};

bool dll_present_in_imports (struct PE_get_imports_info *i, char *dll)
{
	for (int DLL=0; DLL < i->import_descriptors_t; DLL++)
		if (stricmp (dll, i->DLL_names[DLL])==0)
			return true;
	return false;
};

void dump_imports (struct PE_get_imports_info *i)
{
	printf ("start_RVA=0x" PRI_ADR_HEX "\n", i->start_RVA);
	for (int DLL=0; DLL < i->import_descriptors_t; DLL++)
	{
		printf ("DLL name %s. FirstThunk=0x%x\n", i->DLL_names[DLL], i->FirstThunk[DLL]);
		for (int s=0; i->symbols[DLL][s]; s++)
		{
			byte* tmp=i->symbols[DLL][s];
			printf ("hint=0x%x, name=%s\n", *(wyde*)tmp, (char*)tmp+2);
		};
		printf ("\n");
	};
};

// if place_thunks==true, this function ignores FirstThunk array, 
// frees it and fills it again by new values
// otherwise, thunks are not placed and FirstThunk are pointed to what is in PE_get_imports_info struct
size_t PE_generate_import_table (struct PE_get_imports_info *i, bool place_thunks, 
		byte* out, unsigned out_size)
{
	address out_buf_end_RVA=i->start_RVA + out_size;
	size_t descriptors_size=sizeof(IMAGE_IMPORT_DESCRIPTOR) * (i->import_descriptors_t+1); // including terminator
	if (place_thunks)
	{
		DFREE (i->FirstThunk);
		i->FirstThunk=DMALLOC (address, i->import_descriptors_t, "address"); // allocate new
	};
	//bytefill(out, descriptors_size, 0xff);
	// fill terminating descriptor with zeroes
	bytefill(&((IMAGE_IMPORT_DESCRIPTOR*)out)[i->import_descriptors_t], sizeof(IMAGE_IMPORT_DESCRIPTOR), 0);
	
	address cur_RVA=i->start_RVA + descriptors_size;
	byte* cur_ptr=out + descriptors_size;

	for (int DLL=0; DLL<i->import_descriptors_t; DLL++)
	{
		// count number of hints/symbols
		unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)i->symbols[DLL]);

		size_t FirstThunk_a_size=(symbols_t+1) * sizeof(address);
	
		if (place_thunks)
		{
			// write new FirstThunk value
			((IMAGE_IMPORT_DESCRIPTOR*)out)[DLL].FirstThunk=cur_RVA;
			// update it also here:
			i->FirstThunk[DLL]=cur_RVA;

			// put FirstThunk array
			bzero (cur_ptr, FirstThunk_a_size);
			cur_ptr+=FirstThunk_a_size; cur_RVA+=FirstThunk_a_size;
			oassert (cur_RVA < out_buf_end_RVA);
		}
		else
			((IMAGE_IMPORT_DESCRIPTOR*)out)[DLL].FirstThunk=i->FirstThunk[DLL];
	
		address* table_of_pointers=(address*)cur_ptr;
		table_of_pointers[symbols_t]=0; // terminator

		// update OriginalFirstThunk
		((IMAGE_IMPORT_DESCRIPTOR*)out)[DLL].OriginalFirstThunk=cur_RVA;
		
		// reserve place for all pointers to symbol hints/names including terminator
		cur_RVA+=FirstThunk_a_size; cur_ptr+=FirstThunk_a_size;
		oassert (cur_RVA < out_buf_end_RVA);

		// put all symbol hints/names
		for (unsigned s=0; s<symbols_t; s++)
		{
			// update pointer to this hint/name
			table_of_pointers[s]=cur_RVA;
			// put symbol hint/name
			byte *hint_name=i->symbols[DLL][s];
			unsigned name_z_len_2=strlen((char*)hint_name+2)+1+2; // length including 16-bit hint and terminating zero
			memcpy (cur_ptr, hint_name, name_z_len_2);
			cur_ptr+=name_z_len_2; cur_RVA+=name_z_len_2;
			oassert (cur_RVA < out_buf_end_RVA);
		};

		// update RVA of DLL name
		((IMAGE_IMPORT_DESCRIPTOR*)out)[DLL].Name=cur_RVA;
		// put DLL name
		unsigned DLL_name_z_len=strlen(i->DLL_names[DLL])+1;
		memcpy (cur_ptr, i->DLL_names[DLL], DLL_name_z_len);
		cur_ptr+=DLL_name_z_len; cur_RVA+=DLL_name_z_len;
		oassert (cur_RVA < out_buf_end_RVA);
	};
	return cur_RVA - i->start_RVA;
};

void add_DLL_and_symbol_to_imports (struct PE_get_imports_info *i, char *dll, char *symname, wyde hint,
		address new_thunk)
{
	unsigned new_DLL_pos=i->import_descriptors_t;
	i->import_descriptors_t++;
	i->DLL_names=DREALLOC (i->DLL_names, char*, i->import_descriptors_t, "char*");
	i->DLL_names[new_DLL_pos]=DSTRDUP(dll, "DLL name");
	i->symbols=DREALLOC (i->symbols, byte**, i->import_descriptors_t, "byte**");
	byte** new_symbols=DCALLOC (byte*, 2, "byte*");
	new_symbols[0]=DCALLOC (byte, 2+strlen (symname)+1, "byte*");
	memcpy (new_symbols[0], &hint, sizeof(wyde));
	memcpy (new_symbols[0]+2, symname, strlen(symname));
	i->symbols[new_DLL_pos]=new_symbols;
	i->FirstThunk=DREALLOC (i->FirstThunk, address, i->import_descriptors_t, "address*");
	i->FirstThunk[new_DLL_pos]=new_thunk;
};

bool PE_find_import_by_thunk(struct PE_get_imports_info *i, address val, 
		unsigned *DLL_no, unsigned *sym_no)
{
	for (int DLL=0; DLL < i->import_descriptors_t; DLL++)
	{
		unsigned symbols_t=NULL_terminated_array_of_pointers_size((void**)i->symbols[DLL]);
		if (VAL_IN_BOUNDS_INCL(val, i->FirstThunk[DLL], i->FirstThunk[DLL] + symbols_t*sizeof(address)))
		{
			*DLL_no=DLL;
			*sym_no=(val - i->FirstThunk[DLL])/sizeof(address);
			return true;
		};
	};
	return false;
};

address PE_find_thunk_by_import (struct PE_get_imports_info *i, unsigned DLL_no, unsigned sym_no)
{
	oassert(DLL_no < i->import_descriptors_t);
	oassert(sym_no <= NULL_terminated_array_of_pointers_size((void**)i->symbols[DLL_no]));
	return i->FirstThunk[DLL_no] + sym_no*sizeof(address);
};
