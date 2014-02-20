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

int main(int argc, char* argv[])
{
	oassert(argc==2);
	
	LOADED_IMAGE im;

	MapAndLoad_or_die (argv[1], NULL, &im, false, /* ReadOnly */ true);

	for (unsigned i=0; i<im.NumberOfSections; i++)
	{
		IMAGE_SECTION_HEADER *sect=im.Sections + i;

		DWORD crc=PE_section_CRC32(&im, sect);
		printf ("CRC32 of section %s=0x%08lX\n", (char*)sect->Name, crc);
	};

	UnMapAndLoad_or_die (&im);

	dump_unfreed_blocks();
};
