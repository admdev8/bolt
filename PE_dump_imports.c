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
