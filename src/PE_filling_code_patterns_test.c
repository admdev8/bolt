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
#include <windows.h>
#include "PE_filling_code_patterns.h"
#include "dmalloc.h"
#include "lisp.h"
#include <dbghelp.h>

int main(int argc, char* argv[])
{
	LOADED_IMAGE im;
	if(argc!=2)
	{
		printf ("insufficient arguments\n");
		printf ("Use: %s <filename.exe>\n", argv[0]);
		return 0;
	};

	MapAndLoad_or_die (argv[1], NULL, &im, false, /* ReadOnly */ false);

	obj* tmp=find_filling_code_patterns(&im, false /* ptrs should be pointing to executable sections */, 2);
	
	UnMapAndLoad_or_die (&im);

	obj_dump(tmp);

	obj_free(tmp);
	dump_unfreed_blocks();
};
