/*
 *  _           _ _   
 * | |         | | |  
 * | |__   ___ | | |_ 
 * | '_ \ / _ \| | __|
 * | |_) | (_) | | |_ 
 * |_.__/ \___/|_|\__|
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2017-2018
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
#include "rbtree.h"
#include "PE.h"
#include "stuff.h"
#include "logging.h"
#include "PE_imports.h"
#include "dmalloc.h"
#include "x86_disas.h"
#include "x86.h"
#include "memutils.h"
#include "fmt_utils.h"

address import_adr;

struct my_CB_data
{
	obj*	lst;
	bool	find_JMP;
};

bool is_it_JMP_CALL (address a, struct Da* d, void* void_data)
{
	struct my_CB_data *data=(struct my_CB_data*)void_data;

	if (
		((data->find_JMP==false && d->ins_code==I_CALL) ||
		(data->find_JMP && d->ins_code==I_JMP))
		&& Da_1st_op_is_disp_only(d))
	{
		uint64_t tmp=Da_1st_op_get_disp(d);
		if (tmp==import_adr)
			data->lst=add_to_list(data->lst, obj_octa(a));
	}

	return true; // continue anyway
};

obj* find_CALL_JMP_to_patch(LOADED_IMAGE *im, IMAGE_SECTION_HEADER* text_sect, bool find_JMP)
{
	const char *ins=find_JMP ? "JMP" : "CALL";

	struct my_CB_data CB_data;
	bzero (&CB_data, sizeof(struct my_CB_data));

	printf ("%s() disasming... searching for %s's\n", __FUNCTION__, ins);

	IMAGE_SECTION_HEADER* pdata_sect=PE_find_section_by_name (im, ".pdata");

	if (pdata_sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, ".pdata");

	for (struct RUNTIME_FUNCTION *p=
			(struct RUNTIME_FUNCTION*)(im->MappedAddress + pdata_sect->PointerToRawData);
			p->FunctionStart!=0 && p->FunctionEnd!=0; p++)
	{
		CB_data.find_JMP=find_JMP;
		if (p->FunctionEnd < p->FunctionStart)
		{
			printf ("garbage in .pdata...\n");
			break;
		};
		//printf ("p->FunctionStart=0x%x, p->FunctionEnd=0x%x\n", p->FunctionStart, p->FunctionEnd);
		//printf ("running disasm. start=0x%x size=0x%x\n", p->FunctionStart, p->FunctionEnd - p->FunctionStart);
		PE_disasm_range(im, text_sect, p->FunctionStart, p->FunctionEnd-p->FunctionStart, Fuzzy_True, is_it_JMP_CALL, &CB_data, /* report_error */ false);
	};

	if (LENGTH(CB_data.lst)==0)
	{
		printf ("%s() no %s's found\n", __FUNCTION__, ins);
		return NULL;
	}

	return CB_data.lst;
};

void put_MOV (address adr, byte *ptr, int val, bool is_JMP)
{
	const char *ins=is_JMP ? "JMP" : "CALL";

	if (ptr[0]==0xFF && 
		((is_JMP && ptr[1]==0x25) ||
		(is_JMP==false && ptr[1]==0x15)))
	{
		ptr[0]=0xB8; // MOV EAX, imm32
		memcpy (ptr+1, &val, sizeof(int));
		ptr[5]=0x90; // NOP
		if (is_JMP)
			ptr[6]=0xC3; // RET
		printf ("%s() %s patched at 0x%" PRIx64 " (VA)\n", __FUNCTION__, ins, adr);
	}
	else
		printf ("Error: not a FF x5 %s here.\n", ins);
};

#define PNAME "PE_patch_CALL_JMP_import"

int main(int argc, char* argv[])
{
	printf ("Simple tool for 'removing' import in PE executable\n");
	printf ("<dennis@yurichev.com> (%s %s)\n", __DATE__, __TIME__);
	if (argc!=5)
	{
		printf ("Usage: %s filename.exe import.dll import_func_name value\n", PNAME);
		printf ("for example:\n");
		printf ("\t%s filename.exe kernel32.dll WriteFile 1234\n", PNAME);
		printf ("\t... and all CALLs to kernel32.dll!WriteFile will be replaced by MOV EAX, 1234\n");
		printf ("\t... all JMPs to kernel32.dll!WriteFile will be replaced by MOV EAX, 1234 / RETN\n");
		printf ("\t%s filename.exe advapi32.dll CryptVerifySignatureA 1\n", PNAME);
		printf ("\t... as if CryptVerifySignatureA() would always return 1 (TRUE)\n");
		printf ("\t... however, only FF 15 CALL / FF 25 JMP instructions are handled\n");
		return 0;
	};
	
	int val;
	if (sscanf(argv[4], "%d", &val)!=1)
		die ("Can't parse [%s], must be a number\n", argv[3]);

	LOADED_IMAGE im;

	MapAndLoad_or_die (argv[1], NULL, &im, false, /* ReadOnly */ false);

	struct PE_get_imports_info *imports=PE_get_imports (&im);
	import_adr=PE_find_thunk_by_import (imports, argv[2], argv[3]);

	IMAGE_SECTION_HEADER* text_sect=PE_find_section_by_name (&im, ".text");
	if (text_sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, ".text");

	// TODO join these parts:

	// ---- CALLs

	obj *calls=find_CALL_JMP_to_patch(&im, text_sect, false);

	if (calls)
	{
		// enumerate CALLs we've found:
		for (obj* c=calls; c; c=cdr(c))
		{
			byte *ptr=PE_section_get_ptr_in(&im, text_sect, obj_get_as_octa(c->u.c->head));
			put_MOV (PE_get_original_base(&im) + obj_get_as_octa(c->u.c->head), ptr, val, false);
		};
	};

	obj_free(calls);

	// ---- JMPs

	obj *jmps=find_CALL_JMP_to_patch(&im, text_sect, true);

	if (jmps)
	{
		// enumerate CALLs we've found:
		for (obj* c=jmps; c; c=cdr(c))
		{
			byte *ptr=PE_section_get_ptr_in(&im, text_sect, obj_get_as_octa(c->u.c->head));
			put_MOV (PE_get_original_base(&im) + obj_get_as_octa(c->u.c->head), ptr, val, true);
		};
	};

	obj_free(jmps);

	// ----

	PE_get_imports_info_free(imports);

	UnMapAndLoad_or_die (&im);

	dump_unfreed_blocks();
};
