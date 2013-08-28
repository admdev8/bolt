#include "dmalloc.h"
#include "memutils.h"
#include "X86_emu.h"
#include "x86.h"
#include "x86_disas.h"
#include "oassert.h"
#include "rand.h"
#include "stuff.h"

void SHL (IN tetrabyte value, IN uint8_t shift_value, OUT tetrabyte* result, IN OUT tetrabyte* flags)
{
	tetrabyte tmp;
	
	__asm__("pushfl;"
		"popl %%ebx;"
		"andl $0xfffff72a, %%ebx;" // ~FLAG_PSAZOC
		"orl %%edx, %%ebx;"
		"pushl %%ebx;"
		"popfl;"
		"shll %%cl, %%eax;"
		"pushfl;"
		"popl %%edx;"
				: "=a" (*result), "=d" (tmp)
				: "a" (value), "c" (shift_value), "d" (*flags)
				: "%ebx", "cc"
		);
	*flags=(tmp & FLAG_PSAZOC);
};

void SHR (IN tetrabyte value, IN uint8_t shift_value, OUT tetrabyte* result, IN OUT tetrabyte* flags)
{
	tetrabyte tmp;
	
	__asm__("pushfl;"
		"popl %%ebx;"
		"andl $0xfffff72a, %%ebx;" // ~FLAG_PSAZOC
		"orl %%edx, %%ebx;"
		"pushl %%ebx;"
		"popfl;"
		"shrl %%cl, %%eax;"
		"pushfl;"
		"popl %%edx;"
				: "=a" (*result), "=d" (tmp)
				: "a" (value), "c" (shift_value), "d" (*flags)
				: "%ebx", "cc"
		);
	*flags=(tmp & FLAG_PSAZOC);
};

void SAR (IN tetrabyte value, IN uint8_t shift_value, OUT tetrabyte* result, IN OUT tetrabyte* flags)
{
	tetrabyte tmp;
	
	__asm__("pushfl;"
		"popl %%ebx;"
		"andl $0xfffff72a, %%ebx;" // ~FLAG_PSAZOC
		"orl %%edx, %%ebx;"
		"pushl %%ebx;"
		"popfl;"
		"sarl %%cl, %%eax;"
		"pushfl;"
		"popl %%edx;"
				: "=a" (*result), "=d" (tmp)
				: "a" (value), "c" (shift_value), "d" (*flags)
				: "%ebx", "cc"
		);
	*flags=(tmp & FLAG_PSAZOC);
};

void ADD (IN tetrabyte op1, IN tetrabyte op2, OUT tetrabyte* result, IN OUT tetrabyte* flags)
{
	tetrabyte tmp;
	
	__asm__("pushfl;"
		"popl %%ebx;"
		"andl $0xfffff72a, %%ebx;" // ~FLAG_PSAZOC
		"orl %%edx, %%ebx;"
		"pushl %%ebx;"
		"popfl;"
		"addl %%ecx, %%eax;"
		"pushfl;"
		"popl %%edx;"
				: "=a" (*result), "=d" (tmp)
				: "a" (op1), "c" (op2), "d" (*flags)
				: "%ebx", "cc"
		);
	*flags=(tmp & FLAG_PSAZOC);
};

void Da_emulate_tests()
{
	Da_emulate_result r;	
	CONTEXT ctx;
	bool b;
	Da da;
	BYTE* memory_test=DCALLOC(byte, PAGE_SIZE, "BYTE");
	MemoryCache *mc=MC_MemoryCache_ctor_testing (memory_test, PAGE_SIZE);
	bzero (&ctx, sizeof(CONTEXT));
	cur_fds.fd1=stdout; // for dump_CONTEXT

	sgenrand(GetTickCount());

	// SHL, SHR, SAR
	for (unsigned i=0; i<1000; i++)
		for (byte CL=0; CL<0x20; CL++)
		{
			tetrabyte val=genrand();
			REG result;

			// SHL
			ctx.Esi=0;
			ctx.Ecx=CL;
			ctx.EFlags=0;
			MC_WriteREG(mc, 0, val);
			b=Da_Da(Fuzzy_False, (BYTE*)X86_SHL_OP_ESI_CP_CL, ctx.Eip, &da);
			oassert(b);
			r=Da_emulate(&da, &ctx, mc);
			oassert(r==DA_EMULATED_OK);
			b=MC_ReadREG(mc, 0, &result);	
			oassert(b);
			tetrabyte SHL_result, SHL_flags=0;
			SHL (val, CL, &SHL_result, &SHL_flags);
			oassert(SHL_result==result);
			oassert((ctx.EFlags & FLAG_PSAZOC)==SHL_flags);

			// SHR
			ctx.Esi=0;
			ctx.Ecx=CL;
			ctx.EFlags=0;
			MC_WriteREG(mc, 0, val);
			b=Da_Da(Fuzzy_False, (BYTE*)X86_SHR_OP_ESI_CP_CL, ctx.Eip, &da);
			oassert(b);
			r=Da_emulate(&da, &ctx, mc);
			oassert(r==DA_EMULATED_OK);
			b=MC_ReadREG(mc, 0, &result);	
			oassert(b);
			tetrabyte SHR_result, SHR_flags=0;
			SHR (val, CL, &SHR_result, &SHR_flags);
			oassert(SHR_result==result);
			oassert((ctx.EFlags & FLAG_PSAZOC)==SHR_flags);
			
			// SAR
			ctx.Esi=0;
			ctx.Ecx=CL;
			ctx.EFlags=0;
			MC_WriteREG(mc, 0, val);
			b=Da_Da(Fuzzy_False, (BYTE*)X86_SAR_OP_ESI_CP_CL, ctx.Eip, &da);
			oassert(b);
			r=Da_emulate(&da, &ctx, mc);
			oassert(r==DA_EMULATED_OK);
			b=MC_ReadREG(mc, 0, &result);	
			oassert(b);
			tetrabyte SAR_result, SAR_flags=0;
			SAR (val, CL, &SAR_result, &SAR_flags);
			oassert(SAR_result==result);
			oassert((ctx.EFlags & FLAG_PSAZOC)==SAR_flags);
		};
};
