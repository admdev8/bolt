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
#include "lisp.h"
#include "stuff.h"
#include "PE_filling_code_patterns.h"

/*
  RATIONALE

  we are going to find code patterns like this:

  mov     dword ptr [edi+0D4h], offset sub_55FD70
  mov     dword ptr [edi+0D8h], offset sub_55FE90
  mov     dword ptr [edi+0DCh], offset sub_55FF10
  mov     dword ptr [edi+0E0h], offset sub_560030
  mov     dword ptr [edi+0E4h], offset sub_560150
  mov     dword ptr [edi+0E8h], offset sub_560280
  mov     dword ptr [edi+0ECh], offset sub_5603A0

  or this:

  lea     rcx, sub_1401722F0
  mov     [rax+90h], rcx
  lea     rcx, sub_140172630
  mov     [rax+98h], rcx
  lea     rcx, sub_140172610
  mov     [rax+0A0h], rcx
  lea     rcx, sub_140172540
  mov     [rax+0A8h], rcx
  lea     rcx, loc_140172470
  mov     [rax+0B0h], rcx
  lea     rcx, loc_140172700
  mov     [rax+0B8h], rcx
*/

struct my_cb_data
{
	enum X86_register last_LEA_op0;
	REG last_LEA_op1;
	address first_MOV;
	enum X86_register first_MOV_op0_reg;
	bool last_MOV_was_OK;
	int64_t last_MOV_adr_disp;
	obj* table;
	struct my_range offsets;
	uint32_t total;
};

// FIXME: TLS
static LOADED_IMAGE *im;
static struct my_cb_data cb_data;
static address original_base;
static obj* tables=NULL;
static bool ptrs_to_exec_sections;
static unsigned minimal_table_size;

static void add_to_list_and_update (struct my_cb_data *data, struct Da *d, REG VA_overriden_op1)
{
	// it's VA for x86, but RVA for x64! dunno why
#ifndef _WIN64
	VA_overriden_op1-=original_base;
#endif

	if (ptrs_to_exec_sections)
	{
		// check overriden_op1
		if (PE_is_address_in_executable_section(im, VA_overriden_op1)==false)
			return;
	};
	// N.B.: FIXUPs can be there!
	struct Da_op op0=d->op[0];

	// add cons (tuple) to list: (offset, value)
	REG ofs=op0.adr.adr_disp;
	data->table=add_to_list(data->table, cons(obj_octa(ofs), obj_REG(VA_overriden_op1)));
	data->last_MOV_was_OK=true;
	data->last_MOV_adr_disp=op0.adr.adr_disp;
	data->total++;
	range_update (&data->offsets, ofs);
};

static void dump_table_info(struct table_info *s)
{
	printf ("%s()\n", __func__);
	printf ("begin (VA)=0x" PRI_ADR_HEX "\n", original_base + s->begin);
	printf ("min offset=0x%x\n", s->offsets._min);
	printf ("max offset=0x%x\n", s->offsets._max);
	printf ("total=%d\n", s->total);
	oassert (LISTP(s->list_of_pairs));

	for (obj* c=s->list_of_pairs; c; c=c->u.c->tail)
	{
		obj* cons_obj=c->u.c->head;
		oassert(CONSP(cons_obj));
		obj_dump (car(cons_obj));
		printf (" ");
		obj_dump (cdr(cons_obj));
		printf ("\n");
	};
};

static void free_table_info(struct table_info *s)
{
	obj_free (s->list_of_pairs);
	DFREE(s);
};

static void save_and_reset(struct my_cb_data *data)
{
	if (data->table && LENGTH(data->table)>=minimal_table_size)
	{
		struct table_info* tmp=DMALLOC (struct table_info, sizeof (struct table_info), "struct table_info");
		tmp->begin=data->first_MOV;
		tmp->list_of_pairs=data->table;

		tables=add_to_list(tables, 
				   create_obj_opaque (tmp, 
						      dump_table_info /* dumper */, 
						      free_table_info /* free */));
		tmp->offsets=data->offsets; // copy structure
		tmp->total=data->total;
	}
	else
		obj_free (data->table);

	bzero (data, sizeof (struct my_cb_data));
};

static void handle_MOV (struct my_cb_data *data, address a, struct Da* d, REG overriden_op1)
{
	struct Da_op op0=d->op[0];

	if (data->last_MOV_was_OK==false)
	{
		// the first MOV?
		data->first_MOV=a;
		data->first_MOV_op0_reg=op0.adr.adr_base;
		add_to_list_and_update(data, d, overriden_op1);
	}
	else
	{
		// not the first MOV? check register sameness and ascension!
		if (data->first_MOV_op0_reg==op0.adr.adr_base 
		    && data->last_MOV_adr_disp+sizeof(REG)==op0.adr.adr_disp)
			add_to_list_and_update(data, d, overriden_op1);
		else
			save_and_reset(data); // otherwise
	};
};

static bool disasm_cb (address a, struct Da* d, void* void_data)
{
	//printf ("%s() begin, a=0x%x\n", __func__, a);
	struct my_cb_data *data=(struct my_cb_data*)void_data;
	struct Da_op *op0=&d->op[0];
	struct Da_op *op1=&d->op[1];

	// skip compiler noise MOV:
	if (Da_is_ins_and_2ops_are (d, I_MOV, DA_OP_TYPE_REGISTER, DA_OP_TYPE_ANY /* wildcard */))
	{
		/*
		  this must be handled:

		  lea     rcx, ...
		  mov     [rax+3A0h], rcx
		  lea     rcx, ...
		  mov     [rax+3A8h], rcx
		  lea     rcx, loc_1401F2820
		  mov     rbx, [rsp+28h+arg_0] ; compiler quirk, we should skip it!
		  mov     [rax+3B0h], rcx
		  lea     rcx, ...
		  mov     [rax+3B8h], rcx
		  lea     rcx, loc_1401F3330
		  mov     [rax+3C0h], rcx
		*/
		if (value_not_in2 (op0->reg, data->first_MOV_op0_reg, data->last_LEA_op0))
		{
			//printf ("%s() line %d\n", __func__, __LINE__);
			// fine, handle it
			return true;
		};
		// fallthrough otherwise
	}

	/*
	  save LEA info from this pattern:
	  lea     rcx, sub_1401F13D0
	  mov     [rax+3D0h], rcx
	*/
	if (Da_is_ins_and_2ops_are (d, I_LEA, DA_OP_TYPE_REGISTER, DA_OP_TYPE_VALUE_IN_MEMORY))
	{
		// save register & value
		data->last_LEA_op0=op0->reg;
		data->last_LEA_op1=op1->adr.adr_disp;
		//printf ("%s() line %d\n", __func__, __LINE__);
		return true;
	}

	/*
	  handle MOVs like:
	  mov     dword ptr [edi+10h], offset sub_528C90
	  mov     dword ptr [edi+14h], offset sub_528BC0
	  mov     dword ptr [edi+18h], offset loc_529440
	*/
	if (Da_is_ins_and_2ops_are (d, I_MOV, DA_OP_TYPE_VALUE_IN_MEMORY, DA_OP_TYPE_VALUE))
	{
		handle_MOV (data, a, d, obj_get_as_REG(&op1->val._v));
		//printf ("%s() line %d\n", __func__, __LINE__);
		return true;
	}

	/*
	  handle second MOV in this pattern:
	  lea     rcx, sub_1401F13D0
	  mov     [rax+3D0h], rcx
	*/
	if (Da_is_ins_and_2ops_are (d, I_MOV, DA_OP_TYPE_VALUE_IN_MEMORY, DA_OP_TYPE_REGISTER) &&
	    op1->reg==data->last_LEA_op0)
	{
		handle_MOV (data, a, d, data->last_LEA_op1);
		//printf ("%s() line %d\n", __func__, __LINE__);
		return true;
	}

	save_and_reset(data); // some other instruction encountered

	//printf ("%s() line %d\n", __func__, __LINE__);
	return true; // always continue
};

static void exec_sections_cb_fn (LOADED_IMAGE *im, IMAGE_SECTION_HEADER *s, void *cb_data_unused)
{
	//printf ("working out section %s\n", s->Name);
	PE_section_disasm (im, s, Fuzzy_Undefined /* x64_code */, &disasm_cb, &cb_data, /* report_error */ false);
};

// FIXME better name please!
obj* find_filling_code_patterns(LOADED_IMAGE *_im, bool _ptrs_to_exec_sections, unsigned _minimal_table_size)
{
	im=_im;
	minimal_table_size=_minimal_table_size;
	ptrs_to_exec_sections=_ptrs_to_exec_sections;
	original_base=PE_get_original_base(im);
	bzero (&cb_data, sizeof(struct my_cb_data));

	// enumerate all executable segmxents:
	PE_enumerate_executable_sections(im, &exec_sections_cb_fn, NULL);
	return tables;
};
