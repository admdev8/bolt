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

#include "PE_imports.h"
#include "memutils.h"
#include "logging.h"

int main()
{
	byte buf[0x1000];
	bzero(buf, sizeof(buf));
	char* DLL_names[]={"one.dll", "two.dll", "three.dll" };

	byte* one_symbols[]={ (byte*)"\x11\x22_FirstFunction", (byte*)"\x22\x33SecondFunction", (byte*)"\x33\x44ThirdFunction", NULL };
	byte* two_symbols[]={ (byte*)"\x11\x22TwoDllFirstFunction", (byte*)"\x22\x33TwoDllSecondFunction", NULL };
	byte* three_symbols[]={ (byte*)"\x11\x22ThirdDllFirstFunction", NULL };
	byte** symbols[]={ one_symbols, two_symbols, three_symbols };

	cur_fds.fd1=stdout;

	struct PE_get_imports_info i;
	i.start_RVA=0x4000;
	i.import_descriptors_t=3;
	i.DLL_names=DLL_names;
	i.symbols=symbols;

	size_t sz=PE_generate_import_table (&i, /* place_thunks */ true, buf, sizeof(buf));

	L_print_buf(buf, sz);
};
