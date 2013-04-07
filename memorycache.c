#include "memorycache.h"
//#include "printf-utils.h"
#include "dmalloc.h"
#include "rbtree.h"
#include "stuff.h"

MemoryCache* MC_MemoryCache_ctor(HANDLE PHDL, BOOL dont_read_from_quicksilver_places)
{
    MemoryCache* rt=(MemoryCache*)DCALLOC(sizeof (MemoryCache), "MemoryCache");
    rt->PHDL=PHDL;
    rt->last_ptr_idx=-1;
    rt->_cache=rbtree_create(TRUE, "MemoryCache._cache", compare_size_t);
    rt->dont_read_from_quicksilver_places=dont_read_from_quicksilver_places;
    return rt;
};

void MC_MemoryCache_dtor(MemoryCache *mc, BOOL check_unflushed_elements)
{
    struct rbtree_node_t *i;

    if (check_unflushed_elements)
    {
        for (i=rbtree_minimum(mc->_cache); i!=NULL; i=rbtree_succ(i))
        {
            MemoryCacheElement *v=(MemoryCacheElement*)i->value;
            if (v->to_be_flushed)
            {
                printf (__FUNCTION__"(): there are still elements to be flushed!\n");
                assert(0);
            };
        };
    };

    rbtree_foreach(mc->_cache, NULL, NULL, dfree);
    rbtree_deinit(mc->_cache);
    DFREE (mc);
};

static void* key_copier(void *i)
{
    return i;
};

static void* value_copier(void *v)
{
    return DMEMDUP (v, sizeof(MemoryCacheElement), "MemoryCacheElement");
};

MemoryCache* MC_MemoryCache_copy_ctor (MemoryCache *mc)
{
    MemoryCache* rt;
    
    //L (2, __FUNCTION__"(): begin\n");
        
    rt=(MemoryCache*)DCALLOC(sizeof (MemoryCache), "MemoryCache"); 
    rt->PHDL=mc->PHDL;
    rt->last_ptr_idx=-1;
    rt->_cache=rbtree_create(TRUE, "MemoryCache._cache", compare_size_t);
    rbtree_copy (mc->_cache, rt->_cache, key_copier, value_copier);
    
    return rt;
};

BOOL MC_LoadPageForAddress (MemoryCache *mc, address adr)
{
    address idx, rd_adr;
    SIZE_T bytes_read;
    MemoryCacheElement *t;

#ifndef _WIN64
    // as of win32
#define _ADR_SKIP 0x7FFE0000
    if (mc->dont_read_from_quicksilver_places && (adr>=_ADR_SKIP && adr<(_ADR_SKIP+PAGE_SIZE)))
    {
        // нужно всегда обламывать чтение этих мест - там, например, текущее системное время, 
        // от этого тестирование эмулятора CPU рандомно глючит, долго я искал эту багу :(
        
        // с другой стороны, эмулятор CPU вполне может нормально работать, хоть и с небольшими 
        // отклонениями по системному времени
        //L (2, __FUNCTION__ "(0x" PRI_ADR_HEX "): wouldn't read process memory\n");
        return FALSE;
    };
#endif

    idx=adr>>LOG2_PAGE_SIZE;
    rd_adr=idx<<LOG2_PAGE_SIZE;
    t=(MemoryCacheElement*)DCALLOC(sizeof(MemoryCacheElement), "MemoryCacheElement");

    if (ReadProcessMemory (mc->PHDL, (LPCVOID)rd_adr, t->block, PAGE_SIZE, &bytes_read)==FALSE)
    {
        DFREE (t);
        //L (2, __FUNCTION__ "(0x" PRI_ADR_HEX "): can't read process memory at 0x" PRI_ADR_HEX "\n", adr, rd_adr);
        return FALSE;
    };
    
    assert (bytes_read==PAGE_SIZE);
    rbtree_insert(mc->_cache, (void*)idx, t);
    return TRUE;
};

BOOL MC_ReadBuffer (MemoryCache *mc, address adr, SIZE_T size, BYTE* outbuf)
{
    SIZE_T i;
    // FIXME: это временное решение. и тормозное, конечно
    for (i=0; i<size; i++)
        if (MC_ReadByte (mc, adr+i, &outbuf[i])==FALSE)
            return FALSE;
    return TRUE;
};

BOOL MC_WriteBuffer (MemoryCache *mc, address adr, SIZE_T size, BYTE* inbuf)
{
    SIZE_T i;
    // FIXME: это временное решение. и тормозное, конечно
    for (i=0; i<size; i++)
        if (MC_WriteByte (mc, adr+i, inbuf[i])==FALSE)
            return FALSE;
    return TRUE;
};

BYTE* MC_find_page_ptr(MemoryCache *mc, address adr)
{
    address idx=adr>>LOG2_PAGE_SIZE;

    if (idx==mc->last_ptr_idx)
        return mc->last_ptr;
    else
    {
        MemoryCacheElement *tmp=(MemoryCacheElement*)rbtree_lookup(mc->_cache, (void*)idx);
        if (tmp==NULL)
        {
            if (MC_LoadPageForAddress (mc, adr)==FALSE) // подгружаем блок если у нас его нету
                return NULL;
            tmp=(MemoryCacheElement*)rbtree_lookup(mc->_cache, (void*)idx);
            assert (tmp!=NULL);
            mc->last_ptr=tmp->block;
            mc->last_ptr_idx=idx;
            return mc->last_ptr;
        }
        else
            return tmp->block;
    };
};

BOOL MC_ReadByte (MemoryCache *mc, address adr, BYTE * out)
{
    BYTE* p=MC_find_page_ptr (mc, adr);
    if (p==NULL)
        return FALSE;
    *out=p[adr&(PAGE_SIZE-1)];
    return TRUE;
};

void MC_mark_as_to_be_flushed(MemoryCache *mc, address idx)
{
    MemoryCacheElement *m=(MemoryCacheElement*)rbtree_lookup(mc->_cache, (void*)idx);
    assert (m!=NULL);
    m->to_be_flushed=TRUE;
};

BOOL MC_WriteByte (MemoryCache *mc, address adr, BYTE val)
{
    address idx;
    BYTE *p;
    
    p=MC_find_page_ptr (mc, adr);
    if (p==NULL)
        return FALSE;
    idx=adr>>LOG2_PAGE_SIZE;

    p[adr&(PAGE_SIZE-1)]=val;
    MC_mark_as_to_be_flushed(mc, idx);
    return TRUE;
};

BOOL MC_ReadWord (MemoryCache *mc, address adr, WORD * out)
{
    BYTE *p;
    int adr_frac;
    
    p=MC_find_page_ptr (mc, adr);
    if (p==NULL)
        return FALSE;

    adr_frac=adr&(PAGE_SIZE-1);
    // if this WORD is right on joint of two pages...
    if (adr_frac>(PAGE_SIZE-sizeof(WORD)))
    {
        BYTE b1, b2;

        if (MC_ReadByte(mc, adr+1, &b1)==FALSE)
            return FALSE;
        if (MC_ReadByte(mc, adr, &b2)==FALSE)
            return FALSE;

        *out=(b1 << 8) | b2;
    }
    else
        *out=*(WORD*)(p+adr_frac);
    return TRUE;
};

BOOL MC_WriteWord (MemoryCache *mc, address adr, WORD val)
{
    address idx;
    BYTE *p;
    unsigned adr_frac;
    
    p=MC_find_page_ptr (mc, adr);
    idx=adr>>LOG2_PAGE_SIZE;

    if (p==NULL)
        return FALSE;

    adr_frac=adr&(PAGE_SIZE-1);
    // а если этот WORD на границе двух страниц...
    if (adr_frac>(PAGE_SIZE-sizeof(WORD)))
    {
        if (MC_WriteByte (mc, adr+0, val&0xFF)==FALSE)
            return FALSE;
        if (MC_WriteByte (mc, adr+1, val>>8)==FALSE)
            return FALSE;
    }
    else
    {
        *(WORD*)(p+adr_frac)=val;
        MC_mark_as_to_be_flushed(mc, idx);
    };
 
    return TRUE;
};

BOOL MC_ReadDword (MemoryCache *mc, address adr, DWORD * out)
{
    //L (2, __FUNCTION__ "(): adr=0x" PRI_ADR_HEX "\n", adr);
 
    BYTE *p;
    unsigned adr_frac;
    
    p=MC_find_page_ptr (mc, adr);
    if (p==NULL)
        return FALSE;

    adr_frac=adr&(PAGE_SIZE-1);
    // а если этот DWORD на границе двух страниц...
    if (adr_frac>(PAGE_SIZE-sizeof(DWORD)))
    {
        BYTE b1, b2, b3, b4;
        BOOL read_OK=TRUE;
        
        if (MC_ReadByte(mc, adr+3, &b1)==FALSE)
            read_OK=FALSE;
        if (MC_ReadByte(mc, adr+2, &b2)==FALSE)
            read_OK=FALSE;
        if (MC_ReadByte(mc, adr+1, &b3)==FALSE)
            read_OK=FALSE;
        if (MC_ReadByte(mc, adr+0, &b4)==FALSE)
            read_OK=FALSE;
        
        if (read_OK==FALSE)
        {
            //L (2, __FUNCTION__ "(0x" PRI_ADR_HEX "): one of ReadByte() funcs failed\n", adr);
            return FALSE;
        };

        *out=(b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
    }
    else
        *out=*(DWORD*)(p+adr_frac);
    return TRUE;
};

BOOL MC_WriteDword (MemoryCache *mc, address adr, DWORD val)
{
    address idx;
    BYTE *p;
    unsigned adr_frac;
    
    p=MC_find_page_ptr (mc, adr);

    if (p==NULL)
        return FALSE;
    
    idx=adr>>LOG2_PAGE_SIZE;

    adr_frac=adr&(PAGE_SIZE-1);
    // а если этот DWORD на границе двух страниц...
    if (adr_frac>(PAGE_SIZE-sizeof(DWORD)))
    {
        if (MC_WriteByte (mc, adr+0, val&0xFF)==FALSE)
            return FALSE;
        if (MC_WriteByte (mc, adr+1, (val>>8)&0xFF)==FALSE)
            return FALSE;
        if (MC_WriteByte (mc, adr+2, (val>>16)&0xFF)==FALSE)
            return FALSE;
        if (MC_WriteByte (mc, adr+3, (val>>24)&0xFF)==FALSE)
            return FALSE;
    }
    else
    {
        *(DWORD*)(p+adr_frac)=val;
        MC_mark_as_to_be_flushed(mc, idx);
    };

    return TRUE;
};

BOOL MC_ReadDword64 (MemoryCache *mc, address adr, DWORD64 * out)
{
    unsigned adr_frac;
    BYTE *p=MC_find_page_ptr (mc, adr);
    if (p==NULL)
        return FALSE;

    adr_frac=adr&(PAGE_SIZE-1);
    // а если этот DWORD64 на границе двух страниц...
    if (adr_frac>(PAGE_SIZE-sizeof(DWORD64)))
    {
        DWORD d1, d2;

        if (MC_ReadDword(mc, adr+4, &d1)==FALSE)
            return FALSE;
        if (MC_ReadDword(mc, adr+0, &d2)==FALSE)
            return FALSE;

        *out=((uint64_t)d1 << 32) | d2;
    }
    else
        *out=*(DWORD64*)(p+adr_frac);
    return TRUE;
};

BOOL MC_WriteDword64 (MemoryCache *mc, address adr, DWORD64 val)
{
    int adr_frac;
    address idx=adr>>LOG2_PAGE_SIZE;
    BYTE *p=MC_find_page_ptr (mc, adr);
    if (p==NULL)
        return FALSE;

    adr_frac=adr&(PAGE_SIZE-1);
    // а если этот DWORD64 на границе двух страниц...
    if (adr_frac>(PAGE_SIZE-sizeof(DWORD64)))
    {
        if (MC_WriteDword (mc, adr+0, val&0xFFFFFFFF)==FALSE)
            return FALSE;
        if (MC_WriteDword (mc, adr+4, (val>>32)&0xFFFFFFFF)==FALSE)
            return FALSE;
    }
    else
    {
        *(DWORD64*)(p+adr_frac)=val;
        MC_mark_as_to_be_flushed(mc, idx);
    };

    return TRUE;
};

BOOL MC_ReadREG (MemoryCache *mc, address a, REG * out)
{
#ifdef _WIN64
    return MC_ReadDword64 (mc, a, out);
#else
    return MC_ReadDword (mc, a, out);
#endif
};

BOOL MC_WriteREG (MemoryCache *mc, address a, REG val)
{
#ifdef _WIN64
    return MC_WriteDword64 (mc, a, val);
#else
    return MC_WriteDword (mc, a, val);
#endif
};

void MC_Flush(MemoryCache *mc)
{
    struct rbtree_node_t *i;

    for (i=rbtree_minimum(mc->_cache); i!=NULL; i=rbtree_succ(i))
    {
        MemoryCacheElement *v=(MemoryCacheElement*)i->value;
        if (v->to_be_flushed)
        {
            address adr=((size_t)i->key) << LOG2_PAGE_SIZE;
            if (WriteProcessMemory (mc->PHDL, (LPVOID)adr, v->block, PAGE_SIZE, NULL)==FALSE)
                die (__FUNCTION__ "(): can't flush memory cache. fatal error. exiting\n");
            v->to_be_flushed=FALSE;
        };
    };
};

void MC_dump_state(fds *s, MemoryCache *mc)
{
    rbtree_node *i;
    L_fds (s, __FUNCTION__ "()\n");
    
    for (i=rbtree_minimum(mc->_cache); i!=NULL; i=rbtree_succ(i))
    {
        MemoryCacheElement *v=(MemoryCacheElement*)i->value;
        L_fds (s, "adr=0x" PRI_ADR_HEX ", to_be_flushed=", (size_t)i->key, v->to_be_flushed);
    };
};

BOOL MC_DryRunFlush(MemoryCache *mc)
{
    BYTE* tmp=(BYTE*)DMALLOC(PAGE_SIZE, "tmp");
    BOOL rt=TRUE;
    struct rbtree_node_t *i;
    int j;

    for (i=rbtree_minimum(mc->_cache); i!=NULL; i=rbtree_succ(i))
    {
        MemoryCacheElement *v=(MemoryCacheElement*)i->value;
        if (v->to_be_flushed)
        {
            address adr=((size_t)i->key) << LOG2_PAGE_SIZE;
            SIZE_T bytes_read;

            if (ReadProcessMemory (mc->PHDL, (LPCVOID)adr, tmp, PAGE_SIZE, &bytes_read)==FALSE)
                die (__FUNCTION__"(): can't read memory. fatal error. exiting\n");

            assert (bytes_read==PAGE_SIZE);

            if (memcmp (v->block, tmp, PAGE_SIZE)!=0)
            {
                rt=FALSE;
                for (j=0; j<PAGE_SIZE; j++)
                {
                    if (v->block[j]!=tmp[j])
                    {
                        L (__FUNCTION__" () bytes are different at adr 0x" PRI_ADR_HEX ": in cache: 0x%02X, in memory: 0x%02X\n",
                                adr+j, v->block[j], tmp[j]);
                    };
                };
            };
        };
    };

    DFREE (tmp);
    return rt;
};

// FIXME: slow
// can output something to out, but eventually return FALSE
BOOL MC_GetString (MemoryCache *mc, address adr, BOOL unicode, strbuf * out)
{
    int step, i;
    char by, _out;
    int chars_read=0;

    if (unicode)
        step=2;
    else
        step=1;

    // per-byte reading
    if (MC_ReadByte (mc, adr, &_out)==FALSE)
        return FALSE; // memory read error

    if (_out==0)
        return FALSE; // read OK, but no string

    for (i=0; ; i=i+step)
    {
        if (MC_ReadByte (mc, adr+i, &by)==FALSE)
            return FALSE; // memory read error

        strbuf_addc_C_escaped (out, by, FALSE);
        chars_read++;
        if (by==0)
            break;

        if (my_isprint (by)==FALSE)
        {
            //L (2, __FUNCTION__"() (per-byte read) not a printable string (adr=0x" PRI_ADR_HEX " i=%d, b=0x%x), returning empty string\n", adr, i, b);
            return FALSE;
        };
    };

    if (chars_read>3)
    {
        //L (2, __FUNCTION__"() (per-byte read) out=[%s]\n", out.c_str());
        return TRUE;
    }
    else
    {
        //L (2, __FUNCTION__"() (per-byte read) too short string rt=[%s], suppress it\n", rt.c_str());
        return FALSE;
    };
};

BOOL MC_L_print_buf_in_mem_ofs (MemoryCache *mc, address adr, REG size, REG ofs)
{
	BYTE* buf=(BYTE*)DMALLOC (size, "buf");

    if (MC_ReadBuffer (mc, adr, size, buf)==FALSE)
        return FALSE;

	L_print_buf_ofs (buf, size, ofs); // print starting from zero

	DFREE(buf);
    return TRUE;
};

BOOL MC_L_print_buf_in_mem (MemoryCache *mc, address adr, SIZE_T size)
{
	return MC_L_print_buf_in_mem_ofs (mc, adr, size, 0);
};

