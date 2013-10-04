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
#include "oassert.h"
#include <dbghelp.h>

#include "porg_utils.h"
#include "PE.h"
#include "memutils.h"
#include "stuff.h"
#include "dmalloc.h"

// instead of imagehlp.h
#define _IMAGEHLPAPI DECLSPEC_IMPORT WINAPI
BOOL _IMAGEHLPAPI MapAndLoad(LPSTR,LPSTR,PLOADED_IMAGE,BOOL,BOOL); 
BOOL _IMAGEHLPAPI UnMapAndLoad(PLOADED_IMAGE); 

void MapAndLoad_or_die(PSTR image_name, PSTR dllpath, 
		PLOADED_IMAGE LoadedImage, bool DotDll, bool ReadOnly)
{
	if (MapAndLoad (image_name, dllpath, LoadedImage, DotDll, ReadOnly)==false)
	{
		printf ("image_name=%s\n", image_name);
		printf ("dllpath=%s\n", dllpath);
		die_GetLastError ("MapAndLoad() failed.");
	};
};

void UnMapAndLoad_or_die(PLOADED_IMAGE LoadedImage)
{
	if (UnMapAndLoad(LoadedImage)==false)
		die_GetLastError ("UnMapAndLoad() failed.");
};

// sections will be ptr to array of sections. allocated via DMALLOC(). deep copy here. 
// should be freed by DFREE().
void PE_get_sections_info (char *fname, IMAGE_SECTION_HEADER **sections, unsigned *sections_total)
{
	LOADED_IMAGE im;
	MapAndLoad_or_die (fname, NULL, &im, false, true);

	*sections_total=im.NumberOfSections;
	*sections=DMEMDUP(im.Sections, im.NumberOfSections*sizeof(IMAGE_SECTION_HEADER), 
			"IMAGE_SECTION_HEADER[]");

	UnMapAndLoad_or_die (&im);
};

bool PE_is_PE32 (LOADED_IMAGE *im)
{
	if (im->FileHeader->OptionalHeader.Magic==0x10B)
		return false;
	else
		if (im->FileHeader->OptionalHeader.Magic==0x20B)
			return true;
		else
			die ("Unknown FileHeader->OptionalHeader.Magic value\n");
};

IMAGE_EXPORT_DIRECTORY* PE_get_export_directory (LOADED_IMAGE *im, bool PE32_plus)
{
	IMAGE_OPTIONAL_HEADER32* im_opt_header_32=(IMAGE_OPTIONAL_HEADER32*)&((IMAGE_NT_HEADERS32*)im->FileHeader)->OptionalHeader;
	IMAGE_OPTIONAL_HEADER64* im_opt_header_64=(IMAGE_OPTIONAL_HEADER64*)&((IMAGE_NT_HEADERS64*)im->FileHeader)->OptionalHeader;

	if (PE32_plus)
		return (IMAGE_EXPORT_DIRECTORY*)ImageRvaToVa (im->FileHeader, im->MappedAddress, im_opt_header_64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, NULL);
	else
		return (IMAGE_EXPORT_DIRECTORY*)ImageRvaToVa (im->FileHeader, im->MappedAddress, im_opt_header_32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, NULL);
};

tetrabyte* PE_get_reloc_directory (LOADED_IMAGE *im, bool PE32_plus)
{
	IMAGE_OPTIONAL_HEADER32* im_opt_header_32=(IMAGE_OPTIONAL_HEADER32*)&((IMAGE_NT_HEADERS32*)im->FileHeader)->OptionalHeader;
	IMAGE_OPTIONAL_HEADER64* im_opt_header_64=(IMAGE_OPTIONAL_HEADER64*)&((IMAGE_NT_HEADERS64*)im->FileHeader)->OptionalHeader;

	if (PE32_plus)
		return (tetrabyte*)ImageRvaToVa (im->FileHeader, im->MappedAddress, im_opt_header_64->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress, NULL);
	else
		return (tetrabyte*)ImageRvaToVa (im->FileHeader, im->MappedAddress, im_opt_header_32->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress, NULL);
};

unsigned PE_count_import_descriptors (LOADED_IMAGE *im)
{
	unsigned rt=0;
	bool PE32_plus=PE_is_PE32(im);
	IMAGE_IMPORT_DESCRIPTOR* import_dir=PE_get_import_descriptor(im, PE32_plus);
	for (IMAGE_IMPORT_DESCRIPTOR *i=import_dir; i->OriginalFirstThunk; i++)
		rt++;
	return rt;
};

address PE_get_import_descriptor_RVA (LOADED_IMAGE *im, bool PE32_plus)
{
	IMAGE_OPTIONAL_HEADER32* im_opt_header_32=(IMAGE_OPTIONAL_HEADER32*)&((IMAGE_NT_HEADERS32*)im->FileHeader)->OptionalHeader;
	IMAGE_OPTIONAL_HEADER64* im_opt_header_64=(IMAGE_OPTIONAL_HEADER64*)&((IMAGE_NT_HEADERS64*)im->FileHeader)->OptionalHeader;

	if (PE32_plus)
		return im_opt_header_64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	else
		return im_opt_header_32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
};

IMAGE_IMPORT_DESCRIPTOR* PE_get_import_descriptor (LOADED_IMAGE *im, bool PE32_plus)
{
	return (IMAGE_IMPORT_DESCRIPTOR*)ImageRvaToVa (im->FileHeader, im->MappedAddress, 
			PE_get_import_descriptor_RVA(im, PE32_plus), NULL);
};

address PE_get_original_base (LOADED_IMAGE *im)
{
#ifdef _WIN64
	IMAGE_OPTIONAL_HEADER64* im_opt_header_64=(IMAGE_OPTIONAL_HEADER64*)&((IMAGE_NT_HEADERS64*)im->FileHeader)->OptionalHeader;

	return im_opt_header_64->ImageBase;
#else
	IMAGE_OPTIONAL_HEADER32* im_opt_header_32=(IMAGE_OPTIONAL_HEADER32*)&((IMAGE_NT_HEADERS32*)im->FileHeader)->OptionalHeader;

	return im_opt_header_32->ImageBase;
#endif
};

void PE_get_info (char *fname, address loaded_base, PE_info *out, callback_add_symbol add_symbol_fn, void *add_symbol_fn_params)
{
	LOADED_IMAGE im;
	char *s;

	bzero (out, sizeof(PE_info));

	MapAndLoad_or_die (fname, NULL, &im, false, true);

	out->PE32_plus=PE_is_PE32(&im);

	IMAGE_EXPORT_DIRECTORY* export_dir=PE_get_export_directory (&im, out->PE32_plus);

	out->original_base=PE_get_original_base (&im);

	// why AddressOfEntryPoint is DWORD for x64 as well?
	out->OEP=loaded_base + im.FileHeader->OptionalHeader.AddressOfEntryPoint;
	out->timestamp=im.FileHeader->FileHeader.TimeDateStamp;
	out->machine=im.FileHeader->FileHeader.Machine;
	out->size=im.FileHeader->OptionalHeader.SizeOfImage;

	if (export_dir==NULL)
		out->internal_name=NULL;
	else
	{
		s=(char*)ImageRvaToVa (im.FileHeader, im.MappedAddress, export_dir->Name, NULL);
		if (s==NULL)
			out->internal_name=NULL;
		else
			out->internal_name=DSTRDUP (s, "s");
	};

	if (export_dir)
	{
		DWORD i;
		// ordinals
		for (i=0; i<export_dir->NumberOfFunctions; i++)
		{
			DWORD addr=*((DWORD*)ImageRvaToVa (im.FileHeader, im.MappedAddress, export_dir->AddressOfFunctions+i*4, NULL));
			char tmp_s[128];
			sprintf (tmp_s, "ordinal_%d", (unsigned)(i + export_dir->Base));

			add_symbol_fn(loaded_base + (address)addr, tmp_s, add_symbol_fn_params);
		};

		for (i=0; i<export_dir->NumberOfNames; i++)
		{
			DWORD *oft, *dw;
			WORD *wo;

			oft=(DWORD*)ImageRvaToVa (im.FileHeader, im.MappedAddress, export_dir->AddressOfNames+i*4, NULL);
			s=(char*)ImageRvaToVa (im.FileHeader, im.MappedAddress, *oft, NULL);

			wo=(WORD*)ImageRvaToVa (im.FileHeader, im.MappedAddress, export_dir->AddressOfNameOrdinals+i*2, NULL);
			dw=(DWORD*)ImageRvaToVa (im.FileHeader, im.MappedAddress, (*wo)*4+export_dir->AddressOfFunctions, NULL);

			add_symbol_fn(loaded_base + (address)(*dw), s, add_symbol_fn_params);
		};
	};

	UnMapAndLoad_or_die (&im);
};

void PE_info_free (PE_info *i)
{
	DFREE (i->internal_name);
	DFREE(i);
};

void enum_all_fixups (LOADED_IMAGE *im, callback_enum_fixups callback_fn, void* param)
{
	tetrabyte* fixups=PE_get_reloc_directory (im, PE_is_PE32(im));
	if (fixups==NULL)
		return; // no fixups

	for (tetrabyte *p=fixups; *p; )
	{
		tetrabyte page_RVA=*p; p++;
		tetrabyte block_size=*p; p++;
		//printf ("fixup block. page_RVA=0x%x, block_size=0x%x\n", page_RVA, block_size);
		wyde *l; unsigned i;
		for (l=(wyde*)p, i=0; i<(block_size - sizeof(tetrabyte)*2)/2; l++, i++)
		{
			unsigned type=(*l)>>12;
			unsigned offset=(*l)&0xFFF;
			if (type==IMAGE_REL_BASED_ABSOLUTE)
				continue;
			//printf ("type=%d, offset (RVA)=0x%x\n", type, page_RVA+offset);
			callback_fn(i, type, page_RVA+offset, param);
		};
		p+=(block_size - sizeof(tetrabyte)*2)/4;
	};
};

static void count_fixups_callback_fn (unsigned i, byte type, address a, void *param)
{
	oassert(type==IMAGE_REL_BASED_HIGHLOW);
	(*(tetrabyte*)param)++;
};

unsigned count_fixups (LOADED_IMAGE *im)
{
	tetrabyte cnt=0;

	enum_all_fixups (im, count_fixups_callback_fn, &cnt);

	return cnt;
};

IMAGE_SECTION_HEADER* get_last_section (LOADED_IMAGE *im)
{
	return &im->Sections[im->NumberOfSections-1];
};
	
IMAGE_SECTION_HEADER* PE_find_section_by_name (LOADED_IMAGE *im, char *name)
{
	for (unsigned i=0; i<im->NumberOfSections; i++)
		if (strnicmp(name, (char*)im->Sections[i].Name, 8)==0)
			return &im->Sections[i];
	return NULL; // not found
};

void calculate_next_available_RVA_and_phys_ofs(LOADED_IMAGE *im, address *next_available_RVA, 
		address *next_available_phys_ofs)
{
	IMAGE_SECTION_HEADER *last_sec=get_last_section (im);
	*next_available_RVA=
		align_to_boundary (last_sec->VirtualAddress + last_sec->Misc.VirtualSize, 
				im->FileHeader->OptionalHeader.SectionAlignment);
	*next_available_phys_ofs=
		align_to_boundary (last_sec->PointerToRawData + last_sec->SizeOfRawData,
				im->FileHeader->OptionalHeader.FileAlignment);
};

size_t add_PE_section_at_end(LOADED_IMAGE *im, char* name, SIZE_T sz, DWORD characteristics)
{
	// calculate RVA for new section
	address next_available_RVA, next_available_phys_ofs;
	calculate_next_available_RVA_and_phys_ofs(im, &next_available_RVA, &next_available_phys_ofs);
	
	// create new PE section
	IMAGE_SECTION_HEADER new_sect;
	bzero (&new_sect, sizeof(IMAGE_SECTION_HEADER));

	memcpy (&new_sect.Name[0], name, 8);
	new_sect.Misc.VirtualSize=sz;
	new_sect.VirtualAddress=next_available_RVA;
	new_sect.PointerToRawData=next_available_phys_ofs;
	new_sect.SizeOfRawData=align_to_boundary (sz, im->FileHeader->OptionalHeader.FileAlignment);
	new_sect.Characteristics=characteristics;

	im->FileHeader->OptionalHeader.SizeOfInitializedData+=new_sect.SizeOfRawData;
	im->FileHeader->OptionalHeader.SizeOfImage+=new_sect.SizeOfRawData;

	// add new section
	// just in case: check if this place is free?
	if (is_blk_zero(&im->Sections[im->NumberOfSections], sizeof(IMAGE_SECTION_HEADER))==false)
	{
		printf ("%s() WARNING! new section information was written over some other's data\n", __FUNCTION__);
		printf ("Resulting PE image may not be correct!\n");
	};
	memcpy (&im->Sections[im->NumberOfSections], &new_sect, sizeof(IMAGE_SECTION_HEADER));
	im->NumberOfSections++;
	im->FileHeader->FileHeader.NumberOfSections++;

	return new_sect.SizeOfRawData;
};

void set_data_directory_entry (LOADED_IMAGE *im, unsigned no, DWORD adr, DWORD sz)
{
	im->FileHeader->OptionalHeader.DataDirectory[no].VirtualAddress=adr;
	im->FileHeader->OptionalHeader.DataDirectory[no].Size=sz;
};
