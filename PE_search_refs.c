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

#include <windows.h>
#include <stdbool.h>
#include "oassert.h"
#include <dbghelp.h>
#include <search.h>
#include "bolt_mingw_addons.h"
#include "rbtree.h"
#include "PE.h"
#include "stuff.h"
#include "logging.h"
#include "PE_imports.h"
#include "dmalloc.h"
#include "x86_disas.h"
#include "x86.h"
#include "memutils.h"

struct my_cb_data
{
	bool str_found;
	size_t* op2_can_be_address;
	size_t address_total;
};

bool is_value_in_op2_cb (address a, Da* d, void* void_data)
{
	struct my_cb_data *data=(struct my_cb_data*)void_data;

	if (d->ins_code==I_LEA && Da_2nd_op_is_disp_only(d))
	{
		uint64_t tmp=Da_2nd_op_get_disp(d);

		if (element_in_the_array_of_size_t((size_t)tmp, data->op2_can_be_address, data->address_total))
		{
			data->str_found=true;
			return false;
		};
	}

	return true;
};

bool verbose=false;

void f(LOADED_IMAGE *im, char *str)
{
	address original_base=PE_get_original_base(im);
	size_t str_total;
	size_t *str_RVAs=PE_section_find_needles (im, ".rdata", (byte*)str, strlen(str), 
			&str_total);

	if (verbose)
		for (size_t i=0; i<str_total; i++)
			printf ("%s is at .rdata!0x" PRI_ADR_HEX "\n", str, original_base + str_RVAs[i]);

	// find the instruction where any of str is refered
	struct my_cb_data cb_data;
	bzero (&cb_data, sizeof(struct my_cb_data));
	cb_data.op2_can_be_address=str_RVAs;
	cb_data.address_total=str_total;

	IMAGE_SECTION_HEADER* text_sect=PE_find_section_by_name (im, ".text");
	if (text_sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, ".text");

	// disasming
	IMAGE_SECTION_HEADER* pdata_sect=PE_find_section_by_name (im, ".pdata");

	if (pdata_sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, ".pdata");

	for (struct RUNTIME_FUNCTION *p=
			(struct RUNTIME_FUNCTION*)(im->MappedAddress + pdata_sect->PointerToRawData);
			p->FunctionStart!=0 && p->FunctionEnd!=0; p++)
	{
		cb_data.str_found=false; // reset at each iteration

		PE_disasm_range(im, text_sect, p->FunctionStart, 
				p->FunctionEnd-p->FunctionStart, Fuzzy_True, is_value_in_op2_cb, &cb_data);

		if (cb_data.str_found)
			printf ("0x" PRI_ADR_HEX "\n", original_base + p->FunctionStart);
	};
	DFREE(str_RVAs);
};

int main(int argc, char* argv[])
{
	oassert(argc==3);
	
	LOADED_IMAGE im;

	MapAndLoad_or_die (argv[1], NULL, &im, false, /* ReadOnly */ false);

	f(&im, argv[2]);
	
	UnMapAndLoad_or_die (&im);

	dump_unfreed_blocks();
};
