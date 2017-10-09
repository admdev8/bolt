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
//#include <imagehlp.h>

#include "porg_utils.h"
#include "PE.h"
#include "memutils.h"
#include "stuff.h"
#include "dmalloc.h"
#include "bitfields.h"
#include "fmt_utils.h"

// instead of imagehlp.h
#define _IMAGEHLPAPI DECLSPEC_IMPORT WINAPI
//BOOL _IMAGEHLPAPI MapAndLoad(LPSTR,LPSTR,PLOADED_IMAGE,BOOL,BOOL); 
//BOOL _IMAGEHLPAPI UnMapAndLoad(PLOADED_IMAGE); 

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

tetra* PE_get_reloc_directory (LOADED_IMAGE *im, bool PE32_plus, size_t *size)
{
	IMAGE_OPTIONAL_HEADER32* im_opt_header_32=(IMAGE_OPTIONAL_HEADER32*)&((IMAGE_NT_HEADERS32*)im->FileHeader)->OptionalHeader;
	IMAGE_OPTIONAL_HEADER64* im_opt_header_64=(IMAGE_OPTIONAL_HEADER64*)&((IMAGE_NT_HEADERS64*)im->FileHeader)->OptionalHeader;

	if (PE32_plus)
	{
		if (size)
			*size=im_opt_header_64->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
		return (tetra*)ImageRvaToVa (im->FileHeader, im->MappedAddress, im_opt_header_64->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress, NULL);
	}
	else
	{
		if (size)
			*size=im_opt_header_32->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
		return (tetra*)ImageRvaToVa (im->FileHeader, im->MappedAddress, im_opt_header_32->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress, NULL);
	};
};

IMAGE_SECTION_HEADER *PE_find_reloc_section (LOADED_IMAGE *im)
{
	IMAGE_OPTIONAL_HEADER32* im_opt_header_32=(IMAGE_OPTIONAL_HEADER32*)&((IMAGE_NT_HEADERS32*)im->FileHeader)->OptionalHeader;
	IMAGE_OPTIONAL_HEADER64* im_opt_header_64=(IMAGE_OPTIONAL_HEADER64*)&((IMAGE_NT_HEADERS64*)im->FileHeader)->OptionalHeader;

	DWORD reloc_dir_RVA;
	if (PE_is_PE32(im))
		reloc_dir_RVA=im_opt_header_64->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	else
		reloc_dir_RVA=im_opt_header_32->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

	for (unsigned i=0; i<im->NumberOfSections; i++)
		if (im->Sections[i].VirtualAddress==reloc_dir_RVA)
			return &im->Sections[i];
	return NULL;
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
	tetra* fixups=PE_get_reloc_directory (im, PE_is_PE32(im), NULL);
	if (fixups==NULL)
		return; // no fixups

	unsigned n=0;
	for (tetra *p=fixups; *p; )
	{
		tetra page_RVA=*p; p++;
		tetra block_size=*p; p++;
		//printf ("fixup block. page_RVA=0x%x, block_size=0x%x\n", page_RVA, block_size);
		wyde *l; unsigned i;
		for (l=(wyde*)p, i=0; i<(block_size - sizeof(tetra)*2)/2; l++, i++)
		{
			unsigned type=(*l)>>12;
			unsigned offset=(*l)&0xFFF;
			if (type==IMAGE_REL_BASED_ABSOLUTE) // 0
				continue;
			//printf ("type=%d, offset (RVA)=0x%x\n", type, page_RVA+offset);
			callback_fn(n, type, page_RVA+offset, param);
			n++;
		};
		p+=(block_size - sizeof(tetra)*2)/4;
	};
};

static void count_fixups_callback_fn (unsigned i, byte type, address a, void *param)
{
#ifdef _WIN64
	oassert(type==IMAGE_REL_BASED_DIR64); // 10
#else
	oassert(type==IMAGE_REL_BASED_HIGHLOW); // 3
#endif
	(*(tetra*)param)++;
};

unsigned count_fixups (LOADED_IMAGE *im)
{
	tetra cnt=0;

	enum_all_fixups (im, count_fixups_callback_fn, &cnt);

	return cnt;
};

static void make_array_of_fixups_callback_fn (unsigned i, byte type, address a, void *param)
{
#ifdef _WIN64
	oassert(type==IMAGE_REL_BASED_DIR64); // 10
#else
	oassert(type==IMAGE_REL_BASED_HIGHLOW); // 3
#endif
	((DWORD*)param)[i]=a;
};

DWORD *make_array_of_fixups (LOADED_IMAGE *im, unsigned *cnt)
{
	*cnt=count_fixups (im);
	//printf ("*cnt=%d\n", *cnt);
	DWORD *rt=DMALLOC(DWORD, *cnt, "fixups");
	enum_all_fixups (im, make_array_of_fixups_callback_fn, rt);
	//for (unsigned i=0; i<*cnt; i++)
	//	printf ("%08X\n", rt[i]);
	return rt;
};	

IMAGE_SECTION_HEADER* get_last_section (LOADED_IMAGE *im)
{
	return &im->Sections[im->NumberOfSections-1];
};
	
IMAGE_SECTION_HEADER* PE_find_section_by_name (LOADED_IMAGE *im, char *name)
{
	for (unsigned i=0; i<im->NumberOfSections; i++)
		if (my_strnicmp(name, (char*)im->Sections[i].Name, 8)==0)
			return &im->Sections[i];
	return NULL; // not found
};

void calculate_next_available_RVA_and_phys_ofs(LOADED_IMAGE *im, address *next_available_RVA, 
		address *next_available_phys_ofs)
{
	IMAGE_SECTION_HEADER *last_sec=get_last_section (im);
	if (next_available_RVA)
	{
		DWORD sect_size;

		// some old Watcom compiler may set Virtual Size to 0!
		// that's weird. use SizeOfRawData then...
		if (last_sec->Misc.VirtualSize==0)
			sect_size=last_sec->SizeOfRawData;
		else
			sect_size=last_sec->Misc.VirtualSize;

		*next_available_RVA=
			align_to_boundary (last_sec->VirtualAddress + sect_size, 
					im->FileHeader->OptionalHeader.SectionAlignment);
	};
	if (next_available_phys_ofs)
	{
		*next_available_phys_ofs=
			align_to_boundary (last_sec->PointerToRawData + last_sec->SizeOfRawData,
					im->FileHeader->OptionalHeader.FileAlignment);
	};
};

size_t add_PE_section_at_end(LOADED_IMAGE *im, char* name, SIZE_T sz, DWORD characteristics, DWORD *out_sect_RVA)
{
	// calculate RVA for new section
	address next_available_RVA, next_available_phys_ofs;
	calculate_next_available_RVA_and_phys_ofs(im, &next_available_RVA, &next_available_phys_ofs);

	//printf ("%s(). next_available_RVA=0x%x\n", __FUNCTION__, next_available_RVA);
	
	// create new PE section
	IMAGE_SECTION_HEADER new_sect;
	bzero (&new_sect, sizeof(IMAGE_SECTION_HEADER));

	memcpy (&new_sect.Name[0], name, 8);
	new_sect.Misc.VirtualSize=sz;
	new_sect.VirtualAddress=next_available_RVA;
	if (out_sect_RVA)
		*out_sect_RVA=next_available_RVA;
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

tetra PE_section_CRC32(LOADED_IMAGE *im, IMAGE_SECTION_HEADER* sect)
{
	return CRC32 ((byte*)(im->MappedAddress + sect->PointerToRawData), sect->SizeOfRawData, 0);
};

unsigned PE_section_count_needles(LOADED_IMAGE *im, char *sect_name, byte *needle, size_t needle_size)
{
	IMAGE_SECTION_HEADER* sect=PE_find_section_by_name (im, sect_name);
		
	if (sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, sect_name);

	return omemmem_count ((byte*)(im->MappedAddress + sect->PointerToRawData), sect->SizeOfRawData,
			needle, needle_size);
};

byte* PE_section_find_needle(LOADED_IMAGE *im, char *sect_name, byte *needle, size_t needle_size, DWORD *out_RVA)
{
	IMAGE_SECTION_HEADER* sect=PE_find_section_by_name (im, sect_name);
		
	if (sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, sect_name);

	byte *start=(byte*)(im->MappedAddress + sect->PointerToRawData);
	byte *rt=omemmem (start, sect->SizeOfRawData, needle, needle_size);
	if (rt && out_RVA)
	{
		size_t diff=rt-start;
		//sect->PointerToRawData + diff; physical address in file
		*out_RVA=sect->VirtualAddress + diff;
	};
	return rt;
};

byte* PE_section_get_ptr_in(LOADED_IMAGE *im, IMAGE_SECTION_HEADER *sect, address RVA)
{
	// is it correct?
	return (byte*)(im->MappedAddress + sect->PointerToRawData + RVA - sect->VirtualAddress);
};

void PE_disasm_range (LOADED_IMAGE *im, IMAGE_SECTION_HEADER *sect,
		DWORD begin_RVA, DWORD size, TrueFalseUndefined x64_code,
		PE_section_disasm_cb_fn cb, void* cb_data, bool report_error)
{
	DWORD RVA=begin_RVA;
	DWORD done=0;
	byte *ptr=PE_section_get_ptr_in(im, sect, RVA);
	struct Da d;
	
	do
	{
		//printf ("%s() RVA=0x%x\n", __func__, RVA);
		if (Da_Da(x64_code /*x64*/, (byte*)ptr, (disas_address)RVA, &d))
		{
			if (cb)
				if (cb (RVA, &d, cb_data)==false)
					return;

			ptr+=d.ins_len; RVA+=d.ins_len; done+=d.ins_len;
		}
		else
		{
			if (report_error)
				fprintf (stderr, "%s() Instruction at 0x%lx (RVA) was not disasmed\n", __FUNCTION__, RVA);
			ptr+=1; RVA+=1; done+=1;
		}
	} while (done < size);
};

void PE_section_disasm (LOADED_IMAGE *im, IMAGE_SECTION_HEADER *sect, TrueFalseUndefined x64_code,
		PE_section_disasm_cb_fn cb, void* cb_data, bool report_error)
{
	PE_disasm_range(im, sect, sect->VirtualAddress, sect->Misc.VirtualSize, x64_code, cb, cb_data, report_error);
};

struct RUNTIME_FUNCTION* PE_find_address_among_pdata_RUNTIME_FUNCTIONs (LOADED_IMAGE *im, DWORD a)
{
	IMAGE_SECTION_HEADER* sect=PE_find_section_by_name (im, ".pdata");

	if (sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, ".pdata");

	for (struct RUNTIME_FUNCTION *p=
			(struct RUNTIME_FUNCTION*)(im->MappedAddress + sect->PointerToRawData);
			p->FunctionStart!=0 && p->FunctionEnd!=0; p++)
	{
		if (a >= p->FunctionStart && a< p->FunctionEnd)
			return p;
	};
	return NULL;
};

size_t *PE_section_find_needles (LOADED_IMAGE *im, char *sect_name, byte *needle, size_t needle_size, 
		OUT size_t *needles_total)
{
	IMAGE_SECTION_HEADER* sect=PE_find_section_by_name (im, sect_name);
		
	if (sect==NULL)
		die ("%s() section %s not found\n", __FUNCTION__, sect_name);

	byte *start=(byte*)(im->MappedAddress + sect->PointerToRawData);
	
	size_t* positions=find_all_needles (start, sect->SizeOfRawData, needle, needle_size, needles_total);

	add_value_to_each_element_of_size_t_array (positions, *needles_total, sect->VirtualAddress);
	
	return positions;
};

void PE_fix_checksum(const char *fname)
{
	LOADED_IMAGE im;

	MapAndLoad_or_die (fname, NULL, &im, false, /* ReadOnly */ false);
	DWORD old_checksum, new_checksum;
	if (CheckSumMappedFile (im.MappedAddress, im.SizeOfImage, &old_checksum, &new_checksum)==NULL)
		die ("CheckSumMappedFile() failed\n");

	//printf ("im.FileHeader->OptionalHeader.CheckSum=0x%x, old_checksum=0x%x, new_checksum=0x%x\n", 
	//	im.FileHeader->OptionalHeader.CheckSum,old_checksum, new_checksum);
	im.FileHeader->OptionalHeader.CheckSum=new_checksum;
	UnMapAndLoad_or_die (&im);
};

#define FIXUP_chunk_size 0x1000

// fixups should be sorted on input!
byte* generate_fixups_section (DWORD *fixups, size_t fixups_t, size_t *fixup_section_size)
{
	//printf ("%s(fixups_t=%d)\n", __FUNCTION__, fixups_t);
	byte* out=DCALLOC(byte, fixups_t*sizeof(DWORD)+(sizeof(DWORD)*2), "out"); // с запасом!
	byte* start=out;

	for (size_t i=0; i<fixups_t; )
	{
		// each chunk should be aligned on 4096 byte boundary. don't know why.
		// otherwise, the image will not be loaded.
		DWORD blk_RVA=*((DWORD*)out)=fixups[i]&0xFFFFF000; out+=sizeof(DWORD);
		DWORD* blk_size_to_be_set=(DWORD*)out; out+=sizeof(DWORD);
		
		size_t blk_size=0; // in wydes
			
		for (; i<fixups_t && ((fixups[i]-blk_RVA) < FIXUP_chunk_size); blk_size++, i++)
		{
			WORD to_write=fixups[i]-blk_RVA;
			oassert(to_write < FIXUP_chunk_size);
#ifdef _WIN64
			*((WORD*)out)=(IMAGE_REL_BASED_DIR64 << 12) | (to_write&0xFFF);
#else
			*((WORD*)out)=(IMAGE_REL_BASED_HIGHLOW << 12) | (to_write&0xFFF);
#endif
			out+=sizeof(WORD);
		};
		if (i<fixups_t /* i is not accross array bounds at this moment */ && fixups[i]<blk_RVA)
			die ("%s() fixups array is not monotonic at %d. fixups[%d]=0x%x, blk_RVA=0x%x\n", 
					__FUNCTION__, i, i, fixups[i], blk_RVA);
		if (blk_size&1) // odd?
		{
			// align at 4-bytes boundary
			blk_size++;
			out+=sizeof(WORD); // put empty IMAGE_REL_BASED_ABSOLUTE
		};
		*blk_size_to_be_set=blk_size*sizeof(wyde) + 2*sizeof(tetra);
	};
	*fixup_section_size=out-start;
	return start;
};


bool PE_is_it_code_section (IMAGE_SECTION_HEADER *s)
{
	return IS_SET (s->Characteristics, IMAGE_SCN_CNT_CODE) ||
		IS_SET (s->Characteristics, IMAGE_SCN_MEM_EXECUTE);
};

void PE_enumerate_executable_sections(LOADED_IMAGE *im, PE_enumerate_executable_sections_cb_fn cb, void* cb_data)
{
	for (unsigned i=0; i<im->NumberOfSections; i++)
		if (PE_is_it_code_section (&im->Sections[i]))
			cb (im, &im->Sections[i], cb_data);
};

// return NULL is not found
IMAGE_SECTION_HEADER *PE_get_section_of_address(LOADED_IMAGE *im, address RVA)
{
	for (unsigned i=0; i<im->NumberOfSections; i++)
	{
		IMAGE_SECTION_HEADER *sect=&im->Sections[i];
		if (REG_in_range2 (RVA, sect->VirtualAddress, sect->Misc.VirtualSize))
			return sect;
	};
	return NULL;
};

bool PE_is_address_in_executable_section(LOADED_IMAGE *im, address RVA)
{
	IMAGE_SECTION_HEADER *sect=PE_get_section_of_address (im, RVA);

	if (sect==NULL)
	{
		oassert (!"section for RVA isn't found");
		//printf ("section for RVA (0x" PRI_ADR_HEX ") isn't found\n", RVA);
		return false;
	}

	return PE_is_it_code_section (sect);
};

// move to PE_get_info()?
size_t PE_get_file_size_not_including_non_standard_end (LOADED_IMAGE* im)
{
	IMAGE_SECTION_HEADER* last_sect=get_last_section(im);
	return last_sect->PointerToRawData + last_sect->SizeOfRawData;
}

size_t PE_get_file_size_not_including_non_standard_end_by_name (char* fname)
{
	LOADED_IMAGE im;
	MapAndLoad_or_die (fname, NULL, &im, false, /* ReadOnly */ false);
	size_t rt=PE_get_file_size_not_including_non_standard_end(&im);
	UnMapAndLoad_or_die (&im);
	return rt;
};

