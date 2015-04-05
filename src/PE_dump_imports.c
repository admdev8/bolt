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
#include "bolt_mingw_addons.h"
#include "memutils.h"
#include "PE.h"
#include "stuff.h"
#include "logging.h"
#include "PE_imports.h"
#include "dmalloc.h"

void _dump_imports (char *fname)
{
	LOADED_IMAGE im;
    
	MapAndLoad_or_die (fname, NULL, &im, false, true);

	struct PE_get_imports_info* imports=PE_get_imports (&im);

	if (imports==NULL)
		die ("No imports\n");

	dump_imports (imports);

	UnMapAndLoad_or_die (&im);

	PE_get_imports_info_free(imports);

	dump_unfreed_blocks();
};

int main(int argc, char* argv[])
{
	oassert(argc==2);
	_dump_imports (argv[1]);
};
