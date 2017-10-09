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

#include "lisp.h"
#include "stuff.h"

struct table_info
{
	address begin;
	struct my_range offsets;
	obj* list_of_pairs;
	uint32_t total;
};

// FIXME better name please!
obj* find_filling_code_patterns(LOADED_IMAGE *_im, bool _ptrs_to_exec_sections, unsigned _minimal_table_size);
