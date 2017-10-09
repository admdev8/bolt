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

int main(int argc, char* argv[])
{
	printf ("Simple tool for patching PE executables\n");
	printf ("<dennis@yurichev.com> (%s %s)\n", __DATE__, __TIME__);
	if (argc!=4)
	{
		printf ("usage: %s filename.exe address bytes\n", argv[0]);
		printf ("for example:\n");
		printf ("\t%s filename.exe 0x401000 33C0C3\n", argv[0]);
		return 0;
	};

	LOADED_IMAGE im;

	MapAndLoad_or_die (argv[1], NULL, &im, false, /* ReadOnly */ false);

	address a;
	if (sscanf (argv[2], PRI_REG_HEX, &a)!=1)
		die ("Can't parse address [%s]\n", argv[2]);

	byte *ptr=ImageRvaToVa (im.FileHeader, im.MappedAddress, a - PE_get_original_base(&im), NULL);
	oassert(ptr);

	size_t bytemask_len;
	byte *bytemask=hexstring_to_array_or_die (argv[3], &bytemask_len);

	// patch in memory
	if (memcmp (ptr, bytemask, bytemask_len)==0)
		printf ("already patched\n");
	else
	{
		memcpy (ptr, bytemask, bytemask_len);
		printf ("patching done\n");
	};

	DFREE (bytemask);
	UnMapAndLoad_or_die (&im);

	dump_unfreed_blocks();
};
