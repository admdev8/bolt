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
#include "ostrings.h"
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

address original_base;
bool verbose=false, unicode=false;
LOADED_IMAGE im;
struct my_cb_data cb_data;

struct my_cb_data
{
	bool str_found;
	size_t* op2_can_be_address;
	size_t address_total;
	address ins_referring_str;

	bool track_func_start;
	address last_func_start;
	Da *prev_ins;
};

static bool f1(struct my_cb_data *data, REG tmp, address a)
{
	//printf ("%s (tmp=0x" PRI_ADR_HEX " a=0x" PRI_ADR_HEX ")\n", __func__, tmp, a);
	if (element_in_the_array_of_size_t(tmp, data->op2_can_be_address, data->address_total))
	{
		//printf ("%s():%d found\n", __func__, __LINE__);
		data->str_found=true;
		data->ins_referring_str=a;
		return false;
	};
	return true;
};

bool is_value_in_op2_cb (address a, Da* d, void* void_data)
{
	struct my_cb_data *data=(struct my_cb_data*)void_data;

	//printf ("%s():%d a=0x%x\n", __func__, __LINE__, a);
	if (d->ins_code==I_LEA && Da_2nd_op_is_disp_only(d))
	{
		//printf ("%s():%d a=0x%x\n", __func__, __LINE__, a);
		if (f1 (data, Da_2nd_op_get_disp(d), a)==false)
			return false;
	}
	else
		if (d->ins_code==I_PUSH && Da_1st_op_is_val(d))
		{
			if (f1 (data, Da_1st_op_get_val(d)-original_base, a)==false)
				return false;
		};

	if (data->track_func_start)
	{
		// look for function start
		if (data->prev_ins && Da_is_PUSH_EBP(data->prev_ins) && Da_is_MOV_EBP_ESP(d))
		{
			data->last_func_start=a-X86_PUSH_EBP_LEN;
			//printf ("function start: 0x" PRI_ADR_HEX "\n", data->last_func_start);
		};

		// rotate last instructions
		DFREE(data->prev_ins);
		data->prev_ins=DMEMDUP(d, sizeof(Da), "");
	};
	return true;
};

void disasm_by_pdata(IMAGE_SECTION_HEADER* text_sect, IMAGE_SECTION_HEADER* pdata_sect)
{
	for (struct RUNTIME_FUNCTION *p=
			(struct RUNTIME_FUNCTION*)(im.MappedAddress + pdata_sect->PointerToRawData);
			p->FunctionStart!=0 && p->FunctionEnd!=0; p++)
	{
		cb_data.str_found=false; // reset at each iteration

		//printf ("p->FunctionStart=0x%x\n", p->FunctionStart);
		PE_disasm_range(&im, text_sect, p->FunctionStart, 
				p->FunctionEnd-p->FunctionStart, Fuzzy_True, is_value_in_op2_cb, &cb_data);

		if (cb_data.str_found)
			printf ("0x" PRI_ADR_HEX "\n", original_base + p->FunctionStart);
	};
};

void f(char *str)
{
	if (verbose)
		printf ("searching for [%s]\n", str);
	original_base=PE_get_original_base(&im);
	size_t str_total;
	size_t *str_RVAs;
	size_t str_len=strlen(str);
	
	if (unicode)
	{
		size_t ustr_len;
		byte *ustr=cvt_to_widestr_and_allocate(str, &ustr_len);
		// not count terminating zero! user may set partial string!
		str_RVAs=PE_section_find_needles (&im, ".rdata", ustr, ustr_len-2, &str_total);
		DFREE(ustr);
	}
	else
		str_RVAs=PE_section_find_needles (&im, ".rdata", (byte*)str, str_len, &str_total);

	if (str_total==0)
	{
		printf ("String [%s] not found\n", str);
		exit(0);
	};

	if (verbose)
		for (size_t i=0; i<str_total; i++)
			printf ("%s is at .rdata!0x" PRI_ADR_HEX "\n", str, original_base + str_RVAs[i]);

	// find the instruction where any of str is refered
	bzero (&cb_data, sizeof(struct my_cb_data));
	cb_data.op2_can_be_address=str_RVAs;
	cb_data.address_total=str_total;

	IMAGE_SECTION_HEADER* text_sect=PE_find_section_by_name (&im, ".text");
	if (text_sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, ".text"); // FIXME: not only .text!

	// disasming
#ifdef _WIN64
	IMAGE_SECTION_HEADER* pdata_sect=PE_find_section_by_name (&im, ".pdata");
	if (pdata_sect)
		disasm_by_pdata(text_sect, pdata_sect);
	else
		die ("no .pdata section found!\n");
#else
	cb_data.track_func_start=true;
	PE_section_disasm (&im, text_sect, Fuzzy_Undefined /* x64_code */, &is_value_in_op2_cb, &cb_data);
	if (cb_data.str_found)
	{
		printf ("0x" PRI_ADR_HEX "\n", original_base + cb_data.last_func_start);
		if (verbose)
			printf ("diff=%d bytes\n", cb_data.ins_referring_str - cb_data.last_func_start); 
	};
	DFREE(cb_data.prev_ins);
#endif
	DFREE(str_RVAs);
};

int main(int argc, char* argv[])
{
	printf ("Simple tool for searching for a function in PE executables which use some text string\n");
	printf ("<dennis@yurichev.com> (%s %s)\n", __DATE__, __TIME__);
	if(argc<3)
	{
		printf ("insufficient arguments\n");
		printf ("usage: %s [--unicode] [--verbose] filename.exe text_string\n", argv[0]);
		return 0;
	};

	int current_a=1;

	while (argv[current_a][0]=='-')
	{
		if (stricmp(argv[current_a], "--unicode")==0)
		{
			unicode=true;
			current_a++;
		};

		if (stricmp(argv[current_a], "--verbose")==0)
		{
			verbose=true;
			current_a++;
		};
	};
	
	MapAndLoad_or_die (argv[current_a], NULL, &im, false, /* ReadOnly */ false);
	current_a++;

	f(argv[current_a]);
	current_a++;
	
	UnMapAndLoad_or_die (&im);

	dump_unfreed_blocks();
};
