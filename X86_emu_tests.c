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
#ifndef _WIN64	
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
				b=Da_Da(Fuzzy_False, (BYTE*)opcodes[f], ctx.Eip, &da); // Eip is not set
				oassert(b);
				r=Da_emulate(&da, &ctx, mc, /*emulate_FS_accesses*/ false, 0);
				oassert(r==DA_EMULATED_OK);
				b=MC_ReadREG(mc, 0, &result);	
				oassert(b);
				tetrabyte intrin_result, intrin_result_flags=0;
				intrin_funcs[f] (val, CL, &intrin_result, &intrin_result_flags);
				oassert(intrin_result==result);
/*
	OF flag behaviour is weird on different CPUs.
	I'm not sure anymore.

				if((ctx.EFlags & FLAG_PSAZOC)!=intrin_result_flags)
				{
					printf ("f=%d CL=0x%02X val=0x%08X ctx.EFlags=\n", f, CL, val);
					dump_flags(&cur_fds, ctx.EFlags);
					printf ("\n");
					printf ("intrin_result_flags=\n");
					dump_flags(&cur_fds, intrin_result_flags);
					printf ("\n");
					oassert(0);
				};
*/
			};
		};
	};

	for (unsigned i=0; i<1000; i++)
	{
		tetrabyte op1=genrand(), op2=genrand(), flags=genrand() & FLAG_PSAZOC;

		void (*intrin_funcs[])(tetrabyte, tetrabyte, tetrabyte*, tetrabyte*)={ &intrin_ADD, &intrin_ADC, &intrin_SUB, &intrin_SBB, &intrin_XOR, &intrin_OR, &intrin_AND };
		const char *opcodes[]={ X86_ADD_EAX_EBX, X86_ADC_EAX_EBX, X86_SUB_EAX_EBX, X86_SBB_EAX_EBX, X86_XOR_EAX_EBX, X86_OR_EAX_EBX, X86_AND_EAX_EBX };
		for (int f=0; f<6; f++)
		{
			ctx.Eax=op1;
			ctx.Ebx=op2;
			ctx.EFlags=flags;
			b=Da_Da(Fuzzy_False, (BYTE*)opcodes[f], ctx.Eip, &da);
			oassert(b);
			r=Da_emulate(&da, &ctx, mc, false, 0);
			oassert(r==DA_EMULATED_OK);
			tetrabyte intrin_result, intrin_result_flags=flags;
			intrin_funcs[f] (op1, op2, &intrin_result, &intrin_result_flags);
			oassert(ctx.Eax==intrin_result);
			oassert((ctx.EFlags & FLAG_PSAZOC)==(intrin_result_flags & FLAG_PSAZOC));
		};
	};
	
	for (unsigned i=0; i<1000; i++)
	{
		tetrabyte op1=genrand(), flags=genrand() & FLAG_PSAZOC;

		void (*intrin_funcs[])(tetrabyte, tetrabyte*, tetrabyte*)={ &intrin_NOT, &intrin_NEG };
		const char *opcodes[]={ X86_NOT_EAX, X86_NEG_EAX };
		for (int f=0; f<2; f++)
		{
			ctx.Eax=op1;
			ctx.EFlags=flags;
			b=Da_Da(Fuzzy_False, (BYTE*)opcodes[f], ctx.Eip, &da);
			oassert(b);
			r=Da_emulate(&da, &ctx, mc, false, 0);
			oassert(r==DA_EMULATED_OK);
			tetrabyte intrin_result, intrin_result_flags=flags;
			intrin_funcs[f] (op1, &intrin_result, &intrin_result_flags);
			oassert(ctx.Eax==intrin_result);
			if ((ctx.EFlags & FLAG_PSAZOC) != (intrin_result_flags & FLAG_PSAZOC))
			{
				printf ("flags are not equal. emulated:\n");
    				dump_flags(&cur_fds, ctx.EFlags & FLAG_PSAZOC);
				printf ("\n");
				printf ("should be:\n");
    				dump_flags(&cur_fds, intrin_result_flags & FLAG_PSAZOC);
				printf ("\n");
				fatal_error();
			};
		};
	};
	
	for (unsigned i=0; i<10; i++)
	{
		tetrabyte op1=genrand(), op2=genrand(), flags=genrand() & FLAG_PSAZOC;

		ctx.Eax=op1;
		ctx.Ebx=op2;
		ctx.EFlags=flags;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_XOR_EAX_EBX, ctx.Eip, &da);
		oassert(b);
		r=Da_emulate(&da, &ctx, mc, false, 0);
		oassert(r==DA_EMULATED_OK);
		tetrabyte intrin_result, intrin_result_flags=flags;
		intrin_XOR_addr (&op1, op2, &intrin_result, &intrin_result_flags);
		oassert(ctx.Eax==op1);
		oassert((ctx.EFlags & FLAG_PSAZOC)==(intrin_result_flags & FLAG_PSAZOC));
		//dump_CONTEXT (&cur_fds, &ctx, false, false, false);
		//printf ("intrin_result_flags="); dump_flags (&cur_fds, intrin_result_flags); printf ("\n");
	};

	// MOVSX
	{
		ctx.Eax=1;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_MOVSX_EAX_AL, ctx.Eip, &da);
		oassert(b);
		r=Da_emulate(&da, &ctx, mc, false, 0);
		oassert(r==DA_EMULATED_OK);
		oassert(ctx.Eax==1);
		
		ctx.Eax=0x80;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_MOVSX_AX_AL, ctx.Eip, &da);
		oassert(b);
		r=Da_emulate(&da, &ctx, mc, false, 0);
		oassert(r==DA_EMULATED_OK);
		oassert(ctx.Eax==0xFF80);
		
		ctx.Eax=0x80;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_MOVSX_EAX_AL, ctx.Eip, &da);
		oassert(b);
		r=Da_emulate(&da, &ctx, mc, false, 0);
		oassert(r==DA_EMULATED_OK);
		oassert(ctx.Eax==0xFFFFFF80);
		
		ctx.Eax=0x8123;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_MOVSX_EAX_AX, ctx.Eip, &da);
		oassert(b);
		r=Da_emulate(&da, &ctx, mc, false, 0);
		oassert(r==DA_EMULATED_OK);
		oassert(ctx.Eax==0xFFFF8123);
	};
#endif	
};
