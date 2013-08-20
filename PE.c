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
#include <assert.h>
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

static void MapAndLoad_or_die(PSTR image_name, PSTR dllpath, 
        PLOADED_IMAGE LoadedImage, bool DotDll, bool ReadOnly)
{
    if (MapAndLoad (image_name, dllpath, LoadedImage, DotDll, ReadOnly)==false)
    {
        printf ("image_name=%s\n", image_name);
        printf ("dllpath=%s\n", dllpath);
        die_GetLastError ("MapAndLoad() failed.");
    };
};

static void UnMapAndLoad_or_die(PLOADED_IMAGE LoadedImage)
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

void PE_get_info (char *fname, address loaded_base, PE_info *out, callback_add_symbol add_symbol_fn, void *add_symbol_fn_params)
{
    LOADED_IMAGE im;
    IMAGE_EXPORT_DIRECTORY* export_dir;
    IMAGE_NT_HEADERS32* im_nt_headers_32;
    IMAGE_NT_HEADERS64* im_nt_headers_64;
    IMAGE_OPTIONAL_HEADER32* im_opt_header_32;
    IMAGE_OPTIONAL_HEADER64* im_opt_header_64;
    char *s;

    bzero (out, sizeof(PE_info));

    MapAndLoad_or_die (fname, NULL, &im, false, true);

    if (im.FileHeader->OptionalHeader.Magic==0x10B)
        out->PE32_plus=false;
    else
        if (im.FileHeader->OptionalHeader.Magic==0x20B)
            out->PE32_plus=true;
        else
            die ("Unknown FileHeader->OptionalHeader.Magic value\n");

    im_nt_headers_32=(IMAGE_NT_HEADERS32*)im.FileHeader;
    im_nt_headers_64=(IMAGE_NT_HEADERS64*)im.FileHeader;

    im_opt_header_32=(IMAGE_OPTIONAL_HEADER32*)&im_nt_headers_32->OptionalHeader;
    im_opt_header_64=(IMAGE_OPTIONAL_HEADER64*)&im_nt_headers_64->OptionalHeader;

    if (out->PE32_plus)
        export_dir=(IMAGE_EXPORT_DIRECTORY*)ImageRvaToVa (im.FileHeader, im.MappedAddress, im_opt_header_64->DataDirectory[0].VirtualAddress, NULL);
    else
        export_dir=(IMAGE_EXPORT_DIRECTORY*)ImageRvaToVa (im.FileHeader, im.MappedAddress, im_opt_header_32->DataDirectory[0].VirtualAddress, NULL);

#ifdef _WIN64
    out->original_base=im_opt_header_64->ImageBase;
#else
    out->original_base=im_opt_header_32->ImageBase;
#endif

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

/* vim: set expandtab ts=4 sw=4 : */
