#include "dmalloc.h"
#include "memutils.h"
#include "X86_emu.h"
#include "x86.h"
#include "x86_intrin.h"
#include "x86_disas.h"
#include "oassert.h"
#include "rand.h"
#include "stuff.h"

// here I test mostly flags-affecting instructions like shifts, arithmetic instrucions, etc

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
	{
		tetrabyte val=genrand();
		for (byte CL=0; CL<0x40; CL++)
		{
			REG result;

			void (*intrin_funcs[])(tetrabyte, uint8_t, tetrabyte*, tetrabyte*)={ &intrin_SHL, &intrin_SHR, &intrin_SAR };
			const char *opcodes[]={ X86_SHL_OP_ESI_CP_CL, X86_SHR_OP_ESI_CP_CL, X86_SAR_OP_ESI_CP_CL };
			for (int f=0; f<3; f++)
			{
				ctx.Esi=0;
				ctx.Ecx=CL;
				ctx.EFlags=0;
				MC_WriteREG(mc, 0, val);
				b=Da_Da(Fuzzy_False, (BYTE*)opcodes[f], ctx.Eip, &da);
				oassert(b);
				r=Da_emulate(&da, &ctx, mc);
				oassert(r==DA_EMULATED_OK);
				b=MC_ReadREG(mc, 0, &result);	
				oassert(b);
				tetrabyte intrin_result, intrin_result_flags=0;
				intrin_funcs[f] (val, CL, &intrin_result, &intrin_result_flags);
				oassert(intrin_result==result);
				oassert((ctx.EFlags & FLAG_PSAZOC)==intrin_result_flags);
			};
		};
	};

	for (unsigned i=0; i<1000; i++)
	{
		tetrabyte op1=genrand(), op2=genrand(), flags=genrand() & FLAG_PSAZOC;

		void (*intrin_funcs[])(tetrabyte, tetrabyte, tetrabyte*, tetrabyte*)={ &intrin_ADD, /* &intrin_ADC, */ &intrin_SUB, &intrin_SBB, &intrin_XOR, &intrin_OR, &intrin_AND };
		const char *opcodes[]={ X86_ADD_EAX_EBX, /* X86_ADC_EAX_EBX, */ X86_SUB_EAX_EBX, X86_SBB_EAX_EBX, X86_XOR_EAX_EBX, X86_OR_EAX_EBX, X86_AND_EAX_EBX };
		for (int f=0; f<6; f++)
		{
			ctx.Eax=op1;
			ctx.Ebx=op2;
			ctx.EFlags=flags;
			b=Da_Da(Fuzzy_False, (BYTE*)opcodes[f], ctx.Eip, &da);
			oassert(b);
			r=Da_emulate(&da, &ctx, mc);
			oassert(r==DA_EMULATED_OK);
			tetrabyte intrin_result, intrin_result_flags=0;
			intrin_result_flags=flags;
			intrin_funcs[f] (op1, op2, &intrin_result, &intrin_result_flags);
			oassert(ctx.Eax==intrin_result);
			oassert((ctx.EFlags & FLAG_PSAZOC)==(intrin_result_flags & FLAG_PSAZOC));
		};
	};
};
