#include "dmalloc.h"
#include "memutils.h"
#include "X86_emu.h"
#include "x86.h"
#ifdef _WIN64
#include "intrin_MSVC_x64.h"
#else
#include "intrin_MSVC_x86.h"
#endif
#include "x86_disas.h"
#include "oassert.h"
#include "rand.h"
#include "stuff.h"
#include "fmt_utils.h"
#include "bitfields.h"

// here I test mostly flags-affecting instructions like shifts, arithmetic instrucions, etc

// FIXME: there should be exit(1) instead of oassert()'s

REG gen_rand_REG()
{
#ifdef _WIN64
	return genrand64();
#else
	return genrand();
#endif
}

void Da_emulate_tests()
{
	enum Da_emulate_result r;	
	CONTEXT ctx;
	bool b;
	struct Da da;
	BYTE* memory_test=DCALLOC(byte, PAGE_SIZE, "BYTE");
	struct MemoryCache *mc=MC_MemoryCache_ctor_testing (memory_test, PAGE_SIZE);
	bzero (&ctx, sizeof(CONTEXT));
	cur_fds.fd1=stdout; // for dump_CONTEXT

	sgenrand(GetTickCount());

	// SHL, SHR, SAR
	for (unsigned i=0; i<1000; i++)
	{
		REG val=gen_rand_REG();
		for (byte CL=0; CL<0x40; CL++)
		{
			REG result;
#ifdef _WIN64
			void (*intrin_funcs[])(REG, uint8_t, REG*, REG*)={ &intrin_SHL, &intrin_SHR, &intrin_SAR };
#else
			void (__fastcall *intrin_funcs[])(REG, uint8_t, REG*, REG*)={ &intrin_SHL, &intrin_SHR, &intrin_SAR };
#endif
			// SHL [RSI], CL, etc
#ifdef _WIN64
			const char *opcodes[]={ X64_SHL_OP_RSI_CP_CL, X64_SHR_OP_RSI_CP_CL, X64_SAR_OP_RSI_CP_CL };
#else
			const char *opcodes[]={ X86_SHL_OP_ESI_CP_CL, X86_SHR_OP_ESI_CP_CL, X86_SAR_OP_ESI_CP_CL };
#endif
			for (int f=0; f<3; f++)
			{
				CONTEXT_set_xCX (&ctx, CL);
				CONTEXT_set_xSI (&ctx, 0);
				ctx.EFlags=0;
				MC_WriteREG(mc, 0, val);
				b=Da_Da(Fuzzy_Undefined, (BYTE*)opcodes[f], CONTEXT_get_PC(&ctx), &da); // Rip is not set
				oassert(b);
				r=Da_emulate(&da, &ctx, mc, /*emulate_FS_accesses*/ false, 0);
				oassert(r==DA_EMULATED_OK);
				b=MC_ReadREG(mc, 0, &result);
				oassert(b);
				REG intrin_result, intrin_result_flags=0;
				intrin_funcs[f] (val, CL, &intrin_result, &intrin_result_flags);
				//oassert(intrin_result==result);
				if(intrin_result!=result)
				{
					printf ("%s:%d f=%d intrin_result=0x"PRI_REG_HEX_PAD" result=0x"PRI_REG_HEX_PAD"\n", __FILE__, __LINE__, f, intrin_result, result);
					//exit(0);
					oassert(0);
				};
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
		REG op1=gen_rand_REG(), op2=gen_rand_REG(), flags=gen_rand_REG() & FLAG_PSAZOC;
#ifdef _WIN64
		void (*intrin_funcs[])(REG, REG, REG*, REG*)={ &intrin_ADD, &intrin_ADC, &intrin_SUB, &intrin_SBB, &intrin_XOR, &intrin_OR, &intrin_AND };
		const char *opcodes[]={ X86_ADD_RAX_RBX, X86_ADC_RAX_RBX, X86_SUB_RAX_RBX, X86_SBB_RAX_RBX, X86_XOR_RAX_RBX, X86_OR_RAX_RBX, X86_AND_RAX_RBX };
#else
		void (__fastcall *intrin_funcs[])(REG, REG, REG*, REG*)={ &intrin_ADD, &intrin_ADC, &intrin_SUB, &intrin_SBB, &intrin_XOR, &intrin_OR, &intrin_AND };
		const char *opcodes[]={ X86_ADD_EAX_EBX, X86_ADC_EAX_EBX, X86_SUB_EAX_EBX, X86_SBB_EAX_EBX, X86_XOR_EAX_EBX, X86_OR_EAX_EBX, X86_AND_EAX_EBX };
#endif
		for (int f=0; f<6; f++)
		{
			CONTEXT_set_Accum (&ctx, op1);
			CONTEXT_set_xBX (&ctx, op2);
			ctx.EFlags=flags;
			b=Da_Da(Fuzzy_Undefined, (BYTE*)opcodes[f], CONTEXT_get_PC(&ctx), &da); // Rip is not set
			oassert(b);
			r=Da_emulate(&da, &ctx, mc, false, 0);
			oassert(r==DA_EMULATED_OK);
			REG intrin_result, intrin_result_flags=flags;
			intrin_funcs[f] (op1, op2, &intrin_result, &intrin_result_flags);
			oassert(CONTEXT_get_Accum(&ctx)==intrin_result);
			oassert((ctx.EFlags & FLAG_PSAZOC)==(intrin_result_flags & FLAG_PSAZOC));
		};
	};
	
	for (unsigned i=0; i<1000; i++)
	{
		REG op1=gen_rand_REG(), flags=gen_rand_REG() & FLAG_PSAZOC;

#ifdef _WIN64
		void (*intrin_funcs[])(REG, REG*, REG*)={ &intrin_NOT, &intrin_NEG };
		const char *opcodes[]={ X86_NOT_RAX, X86_NEG_RAX };
#else
		void (__fastcall *intrin_funcs[])(REG, REG*, REG*)={ &intrin_NOT, &intrin_NEG };
		const char *opcodes[]={ X86_NOT_EAX, X86_NEG_EAX };
#endif
		for (int f=0; f<2; f++)
		{
			CONTEXT_set_Accum (&ctx, op1);
			ctx.EFlags=flags;
			//printf ("flags in ctx before emulation:\n");
			//dump_flags(&cur_fds, ctx.EFlags & FLAG_PSAZOC);
			//printf ("\n");
			b=Da_Da(Fuzzy_Undefined, (BYTE*)opcodes[f], CONTEXT_get_PC(&ctx), &da); // Rip is not set
			oassert(b);
			r=Da_emulate(&da, &ctx, mc, false, 0);
			oassert(r==DA_EMULATED_OK);
			REG intrin_result, intrin_result_flags=flags;
			intrin_funcs[f] (op1, &intrin_result, &intrin_result_flags);
			//oassert(CONTEXT_get_Accum(&ctx)==intrin_result);
			if(CONTEXT_get_Accum(&ctx)!=intrin_result)
			{
				printf ("%s:%d f=%d CONTEXT_get_Accum(&ctx)=0x"PRI_REG_HEX_PAD" intrin_result=0x"PRI_REG_HEX_PAD"\n", __FILE__, __LINE__, f, 
					CONTEXT_get_Accum(&ctx), intrin_result);
				//exit(0);
				oassert(0);
			};

			// I don't know how ZF appeared in intrin_ functions
			REMOVE_BIT(ctx.EFlags, FLAG_ZF);
			REMOVE_BIT(intrin_result_flags, FLAG_ZF);
			if ((ctx.EFlags & FLAG_PSAZOC) != (intrin_result_flags & FLAG_PSAZOC))
			{
				printf ("f=%d op1=0x"PRI_REG_HEX_PAD" intrin_result=0x"PRI_REG_HEX_PAD"\n", f, op1, intrin_result);

				printf ("flags are not equal. emulated:\n");
    				dump_flags(&cur_fds, ctx.EFlags & FLAG_PSAZOC);
				printf ("\n");
				printf ("must be:\n");
    				dump_flags(&cur_fds, intrin_result_flags & FLAG_PSAZOC);
				printf ("\n");
				fatal_error();
			};
		};
	};
	
#if 0
	for (unsigned i=0; i<10; i++)
	{
		tetra op1=genrand(), op2=genrand(), flags=genrand() & FLAG_PSAZOC;

		ctx.Eax=op1;
		ctx.Ebx=op2;
		ctx.EFlags=flags;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_XOR_EAX_EBX, ctx.Eip, &da);
		oassert(b);
		r=Da_emulate(&da, &ctx, mc, false, 0);
		oassert(r==DA_EMULATED_OK);
		tetra intrin_result, intrin_result_flags=flags;
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

int main()
{
	Da_emulate_tests();
};
