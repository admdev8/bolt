#include <windows.h>
#include <stdbool.h>
#include "oassert.h"
#include <dbghelp.h>
#include "bolt_mingw_addons.h"
#include "memutils.h"
#include "PE.h"
#include "stuff.h"
#include "logging.h"

void dump_imports (char *fname)
{
	LOADED_IMAGE im;
	IMAGE_IMPORT_DESCRIPTOR* import_dir;
	IMAGE_NT_HEADERS32* im_nt_headers_32;
	IMAGE_NT_HEADERS64* im_nt_headers_64;
	IMAGE_OPTIONAL_HEADER32* im_opt_header_32;
	IMAGE_OPTIONAL_HEADER64* im_opt_header_64;
	bool PE32_plus;

	MapAndLoad_or_die (fname, NULL, &im, false, true);

	if (im.FileHeader->OptionalHeader.Magic==0x10B)
		PE32_plus=false;
	else
		if (im.FileHeader->OptionalHeader.Magic==0x20B)
			PE32_plus=true;
		else
			die ("Unknown FileHeader->OptionalHeader.Magic value\n");

	im_nt_headers_32=(IMAGE_NT_HEADERS32*)im.FileHeader;
	im_nt_headers_64=(IMAGE_NT_HEADERS64*)im.FileHeader;

	im_opt_header_32=(IMAGE_OPTIONAL_HEADER32*)&im_nt_headers_32->OptionalHeader;
	im_opt_header_64=(IMAGE_OPTIONAL_HEADER64*)&im_nt_headers_64->OptionalHeader;

	if (PE32_plus)
		import_dir=(IMAGE_IMPORT_DESCRIPTOR*)ImageRvaToVa (im.FileHeader, im.MappedAddress, im_opt_header_64->DataDirectory[1].VirtualAddress, NULL);
	else
		import_dir=(IMAGE_IMPORT_DESCRIPTOR*)ImageRvaToVa (im.FileHeader, im.MappedAddress, im_opt_header_32->DataDirectory[1].VirtualAddress, NULL);

	if (import_dir==NULL)
		die ("No imports\n");

	cur_fds.fd1=stdout;
	for (IMAGE_IMPORT_DESCRIPTOR *i=import_dir; i->OriginalFirstThunk; i++)
	{
		address* OriginalFirstThunk_a=(char*)ImageRvaToVa (im.FileHeader, im.MappedAddress, i->OriginalFirstThunk, NULL);
		address* FirstThunk_a=(char*)ImageRvaToVa (im.FileHeader, im.MappedAddress, i->FirstThunk, NULL);
		printf ("OriginalFirstThunk=0x%x\n", i->OriginalFirstThunk);
		printf ("TimeDateStamp=0x%x\n", i->TimeDateStamp);
		printf ("ForwarderChain=0x%x\n", i->ForwarderChain);
		char* name=(char*)ImageRvaToVa (im.FileHeader, im.MappedAddress, i->Name, NULL);
		printf ("Name=0x%x (%s)\n", i->Name, name);
		printf ("FirstThunk=0x%x\n", i->FirstThunk);
		printf ("OriginalFirstThunk:\n");
		//L_print_buf (OriginalFirstThunk_a, 0x100);
		
		for (address *i=OriginalFirstThunk_a; *i; i++)
		{
			//printf ("0x%p\n", i);
			byte* tmp=(byte*)ImageRvaToVa(im.FileHeader, im.MappedAddress, *i, NULL);
			printf ("hint=%d, name=%s\n", *(wyde*)tmp, tmp+2);
		};
		
		printf ("FirstThunk:\n");
		L_print_buf (FirstThunk_a, 0x100);
	};

	UnMapAndLoad_or_die (&im);
};

int main(int argc, char* argv[])
{
	oassert(argc==2);
	dump_imports (argv[1]);
};
