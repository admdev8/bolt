#include "dmalloc.h"
#include "memutils.h"
#include "X86_emu.h"
#include "x86.h"
#include "x86_disas.h"
#include "oassert.h"
#include "rand.h"

void SHL (tetrabyte _shifted_value, uint8_t shift_value, tetrabyte* out_result, tetrabyte* out_flags)
{
	tetrabyte flags;
	// ~FLAG_PSAZOC
	__asm__("pushfl;"
		"popl %%ebx;"
		"andl $0xfffff72a, %%ebx;" 		
		"orl %%edx, %%ebx;"
		"pushl %%ebx;"
		"popfl;"
		"shll %%cl, %%eax;"
		"pushfl;"
		"popl %%edx;"
				: "=a" (*out_result), "=d" (flags)
				: "a" (_shifted_value), "c" (shift_value), "d" (*out_flags)
				: "%ebx", "cc"
		);
	*out_flags=(flags & FLAG_PSAZOC);
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

	for (unsigned i=0; i<1000; i++)
		for (byte CL=0; CL<0x20; CL++)
		{
			tetrabyte val=genrand();
			ctx.Esi=0;
			ctx.Ecx=CL;
			ctx.EFlags=0;
			MC_WriteREG(mc, 0, val);
			b=Da_Da(Fuzzy_False, (BYTE*)X86_SHL_OP_ESI_CP_CL, ctx.Eip, &da);
			oassert(b);
			r=Da_emulate(&da, &ctx, mc);
			oassert(r==DA_EMULATED_OK);
			REG result;
			b=MC_ReadREG(mc, 0, &result);	
			oassert(b);
			tetrabyte SHL_result, SHL_flags=0;
			SHL (val, CL, &SHL_result, &SHL_flags);
			oassert(SHL_result==result);
			oassert((ctx.EFlags & FLAG_PSAZOC)==SHL_flags);
		};
};
